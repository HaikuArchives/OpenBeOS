/* raw.c */

#include <stdio.h>

#include "mbuf.h"
#include "sys/protosw.h"
#include "sys/domain.h"
#include "sys/socket.h"
#include "netinet/in_pcb.h"
#include "netinet/in.h"
#include "net_module.h"
#include "netinet/in_var.h"
#include "netinet/ip_var.h"

#ifdef _KERNEL_MODE
#include "net_server/core_module.h"
#include <KernelExport.h>
#include "ipv4/ipv4_module.h"
#include "raw/raw_module.h"

#define m_copym             core->m_copym
#define m_free              core->m_free
#define m_freem             core->m_freem
#define m_prepend           core->m_prepend
#define soreserve			core->soreserve
#define in_pcballoc         core->in_pcballoc
#define in_pcbconnect       core->in_pcbconnect
#define in_pcbdisconnect	core->in_pcbdisconnect
#define in_pcbbind			core->in_pcbbind
#define in_pcbdetach		core->in_pcbdetach
#define sbappendaddr        core->sbappendaddr
#define soisconnected       core->soisconnected
#define soisdisconnected    core->soisdisconnected
#define socantsendmore      core->socantsendmore
#define sowakeup            core->sowakeup
#define ifa_ifwithaddr      core->ifa_ifwithaddr
#define get_interfaces      core->get_interfaces

static struct core_module_info *core = NULL;
static struct ipv4_module_info *ipm = NULL;
#else
#define RAW_MODULE_PATH              "modules/protocols/raw"
#endif

static struct inpcb rawinpcb;
static struct sockaddr_in ripsrc;
static int rip_sendspace = 8192;
static int rip_recvspace = 8192;

void rip_init(void)
{
	rawinpcb.inp_next = rawinpcb.inp_prev = &rawinpcb;
	memset(&ripsrc, 0, sizeof(ripsrc));
	ripsrc.sin_family = AF_INET;
	ripsrc.sin_len = sizeof(ripsrc);
}

int rip_input(struct mbuf *m, int hdrlen)
{
	struct ip *ip = mtod(m, struct ip*);
	struct inpcb *inp;
	struct socket *last = NULL;
	
	ripsrc.sin_addr = ip->ip_src;
	for (inp = rawinpcb.inp_next; inp != &rawinpcb; inp=inp->inp_next) {
		if (inp->inp_ip.ip_p && inp->inp_ip.ip_p != ip->ip_p)
			continue;
		if (inp->laddr.s_addr && inp->laddr.s_addr == ip->ip_dst.s_addr)
			continue;
		if (inp->faddr.s_addr && inp->faddr.s_addr == ip->ip_src.s_addr)
			continue;
		if (last) {
			struct mbuf *n;
			if ((n = m_copym(m, 0, (int)M_COPYALL))) {
				if (sbappendaddr(&last->so_rcv, (struct sockaddr*)&ripsrc, 
				                 n, NULL) == 0)
					m_freem(n);
				else
					sorwakeup(last);
			}
		}
		last = inp->inp_socket;
	}
	if (last) {
		if (sbappendaddr(&last->so_rcv, (struct sockaddr*)&ripsrc, 
		                 m, NULL) == 0)
			m_freem(m);
		else
			sorwakeup(last);
	} else {
		m_freem(m);
		ipstat.ips_noproto++;
		ipstat.ips_delivered--;
	}
	return 0;
}

int rip_output(struct mbuf *m, struct socket *so, uint32 dst)
{
	struct ip *ip;
	struct inpcb *inp = sotoinpcb(so);
	struct mbuf *opts;
	int flags = (so->so_options & SO_DONTROUTE) | IP_ALLOWBROADCAST;

	if ((inp->inp_flags & INP_HDRINCL) == 0) {
		M_PREPEND(m, sizeof(struct ip));
		ip = mtod(m, struct ip *);
		ip->ip_p = inp->inp_ip.ip_p;
		ip->ip_len = m->m_pkthdr.len;
		ip->ip_src = inp->laddr;
		ip->ip_dst.s_addr = dst;
		ip->ip_ttl = MAXTTL;
		opts = inp->inp_options;
		ip->ip_off = 0;
		ip->ip_tos = 0;
	} else {
		ip = mtod(m, struct ip *);
		if (ip->ip_id == 0)
#ifdef _KERNEL_MODE
			if (ipm)
				ip->ip_id = htons(ipm->ip_id());
#else
			ip->ip_id = 1;// XXX - fix me!!! htons(ip_id++);
#endif
		opts = NULL;
		flags |= IP_RAWOUTPUT;
		ipstat.ips_rawout++;
	}

#ifdef _KERNEL_MODE
	if (ipm)
		return ipm->ip_output(m, opts, &inp->inp_route, flags, NULL);
	/* XXX - last arg should be inp->inp_moptions when we have multicast */
#endif

	return 0;
}

int rip_userreq(struct socket *so, int req, struct mbuf *m, struct mbuf *nam,
                struct mbuf *control)
{
	int error = 0;
	struct inpcb *inp = sotoinpcb(so);
	struct ifnet *interfaces = get_interfaces();
	
	switch(req) {
		case PRU_ATTACH:
			if (inp) {
				printf("Trying to attach to a socket already attached!\n");
				return EINVAL;
			}
			if ((error = soreserve(so, rip_sendspace, rip_recvspace)) ||
			    (error = in_pcballoc(so, &rawinpcb)))
				break;
			inp = (struct inpcb*)so->so_pcb;
			inp->inp_ip.ip_p = (int)nam;
			break;
		case PRU_DISCONNECT:
			if ((so->so_state & SS_ISCONNECTED) == 0) {
				error = ENOTCONN;
				break;
			}
		case PRU_ABORT:
			soisdisconnected(so);
		case PRU_DETACH:
			if (inp == NULL) {
				printf("Can't detach from NULL protocol block!\n");
				error = EINVAL;
				break;
			}
			in_pcbdetach(inp);
			break;
		case PRU_SEND: {
			uint32 dst;
			if ((so->so_state & SS_ISCONNECTED)) {
				if (nam) {
					error = EISCONN;
					break;
				}
				dst = inp->faddr.s_addr;
			} else {
				if (!nam) {
					error = ENOTCONN;
					break;
				}
				dst = mtod(nam, struct sockaddr_in *)->sin_addr.s_addr;
			}
			error = rip_output(m, so, dst);
			m = NULL;
			break;
		}
		case PRU_BIND: {
			struct sockaddr_in *addr = mtod(nam, struct sockaddr_in *);
			if (nam->m_len != sizeof(*addr)) {
				error = EINVAL;
				break;
			}
			if ((interfaces) ||
			    ((addr->sin_family != AF_INET) &&
			    (addr->sin_family != AF_IMPLINK)) ||
			    (addr->sin_addr.s_addr && 
			    ifa_ifwithaddr((struct sockaddr*)addr) == 0)) {
				error = EADDRNOTAVAIL;
				break;
			}
			inp->laddr = addr->sin_addr;
			break;
		}
		case PRU_CONNECT: {
			struct sockaddr_in *addr = mtod(nam, struct sockaddr_in *);
			
			if (nam->m_len != sizeof(*addr)) {
				error = EINVAL;
				break;
			}
			if ((interfaces == NULL)) {
				error = EADDRNOTAVAIL;
				break;
			}
			if ((addr->sin_family != AF_INET) &&
			    (addr->sin_family != AF_IMPLINK)) {
				error = EAFNOSUPPORT;
				break;
			}
			inp->faddr = addr->sin_addr;
			soisconnected(so);
			break;
		}
		case PRU_CONNECT2:
			error = EOPNOTSUPP;
			break;
		case PRU_SHUTDOWN:
			socantsendmore(so);
			break;
		case PRU_RCVOOB:
		case PRU_RCVD:
		case PRU_LISTEN:
		case PRU_ACCEPT:
		case PRU_SENDOOB:
			error = EINVAL;//EOPNOTSUPP;
			break;			
		/* add remaining cases */
	}
	
	return error;
}

int rip_ctloutput(int op, struct socket *so, int level,
                  int optnum, struct mbuf **m)
{
	struct inpcb *inp = sotoinpcb(so);
	
	if (level != IPPROTO_IP)
		return EINVAL;
	
	switch (optnum) {
		case IP_HDRINCL:
			if (op == PRCO_SETOPT || op == PRCO_GETOPT) {
				if (m == NULL || *m == NULL || (*m)->m_len < sizeof(int))
					return EINVAL;
				if (op == PRCO_SETOPT) {
					if (*mtod(*m, int*))
						inp->inp_flags |= INP_HDRINCL;
					else
						inp->inp_flags &= ~INP_HDRINCL;
					m_free(*m);
				} else {
					(*m)->m_len = sizeof(int);
					*mtod(*m, int*) = inp->inp_flags & INP_HDRINCL;
				}
				return 0;
			}
			break;
		/* XXX - Add other options here */
	}
#ifdef _KERNEL_MODE
	return ipm->ctloutput(op, so, level, optnum, m);
#else
/* XXX - get this working for app...? */
	return 0;
#endif
}
	
static struct protosw my_protocol = {
	"Raw IP module",
	RAW_MODULE_PATH,
	SOCK_RAW,
	NULL,
	0,
	PR_ATOMIC | PR_ADDR,
	NET_LAYER4,
	
	&rip_init,
	&rip_input,
	NULL,
	&rip_userreq,
	NULL,                    /* pr_sysctl */
	&rip_ctloutput,
	
	NULL,
	NULL
};

#ifndef _KERNEL_MODE
static void rip_protocol_init(void)
{
	add_domain(NULL, AF_INET);
	add_protocol(&my_protocol, AF_INET);
}

struct protocol_info protocol_info = {
	"Raw IP Module",
	&rip_protocol_init
};

#else /* kernel setup */

static status_t k_init(void)
{
	if (!core)
		get_module(CORE_MODULE_PATH, (module_info**)&core);
	
	core->add_domain(NULL, AF_INET);
	core->add_protocol(&my_protocol, AF_INET);
	
	if (!ipm)
		get_module(IPV4_MODULE_PATH, (module_info**)&ipm);
		
	return 0;
}

static status_t rip_ops(int32 op, ...)
{
	switch(op) {
		case B_MODULE_INIT:
			return k_init();
		case B_MODULE_UNINIT:
			break;
		default:
			return B_ERROR;
	}
	return B_OK;
}

static struct raw_module_info my_module = {
	{
		RAW_MODULE_PATH,
		B_KEEP_LOADED,
		rip_ops
	},
	&rip_input
};

_EXPORT module_info *modules[] = {
	(module_info*)&my_module,
	NULL
};

#endif
