/* udp.c
 */

#include <stdio.h>

#include "mbuf.h"
#include "net_misc.h"
#include "net_module.h"
#include "protocols.h"
#include "netinet/in_systm.h"
#include "netinet/in_var.h"
#include "netinet/in_pcb.h"
#include "netinet/ip.h"
#include "sys/domain.h"
#include "sys/protosw.h"
#include "netinet/ip_var.h"
#include "netinet/udp.h"
#include "netinet/udp_var.h"

#ifdef _KERNEL_MODE
#include <KernelExport.h>
#include "net_server/core_module.h"

static struct core_module_info *core = NULL;
#define m_freem             core->m_freem
#define m_adj               core->m_adj
#define m_prepend           core->m_prepend
#define in_pcballoc         core->in_pcballoc
#define in_pcbconnect       core->in_pcbconnect
#define in_pcbdisconnect	core->in_pcbdisconnect
#define in_pcbbind			core->in_pcbbind
#define soreserve			core->soreserve
#define sbappendaddr		core->sbappendaddr
#define in_pcblookup		core->in_pcblookup
#define sowakeup			core->sowakeup
#define in_pcbdetach		core->in_pcbdetach
#define in_control          core->in_control
#define soisconnected       core->soisconnected

#define UDP_MODULE_PATH		"network/protocol/udp"

#else
#define UDP_MODULE_PATH	    "modules/protocol/udp"
#endif

struct protosw *proto[IPPROTO_MAX];
static struct inpcb udb;	/* head of the UDP PCB list! */
static struct inpcb *udp_last_inpcb = NULL;
static struct sockaddr_in udp_in;
static int udpcksum = 1;	/* do we calculate the UDP checksum? */
static struct udpstat udpstat;

static uint32 udp_sendspace;	/* size of send buffer */
static uint32 udp_recvspace;	/* size of recieve buffer */

#if SHOW_DEBUG
static void dump_udp(struct mbuf *buf)
{
	struct ip *ip = mtod(buf, struct ip*);
	struct udphdr *udp = (struct udphdr*)((caddr_t)ip + (ip->ip_hl * 4));

	printf("udp_header  :\n");
	printf("            : src_port      : %d\n", ntohs(udp->src_port));
	printf("            : dst_port      : %d\n", ntohs(udp->dst_port));
	printf("            : udp length    : %d bytes\n", ntohs(udp->length));
}
#endif /* SHOW_DEBUG */

int udp_output(struct inpcb *inp, struct mbuf *m, struct mbuf *addr,struct mbuf *control)
{
	struct udpiphdr *ui;
	uint16 len = m->m_pkthdr.len;
	uint16 hdrlen = len + (uint16)sizeof(struct udpiphdr);
	struct in_addr laddr;
	int error = 0;

	if (control)
		m_freem(control);

	if (addr) {
		laddr = inp->laddr;
		if (inp->faddr.s_addr != INADDR_ANY) {
			error = EISCONN;
			goto release;
		}
		error = in_pcbconnect(inp, addr);
		if (error)
			goto release;

	} else {
		if (inp->faddr.s_addr == INADDR_ANY) {
			error = ENOTCONN;
			goto release;
		}
	}

	M_PREPEND(m, sizeof(*ui));
	if (!m) {
		error = ENOMEM;
		goto release;
	}

	ui = mtod(m, struct udpiphdr *);
	memset(&ui->ui_x1,0, sizeof(ui->ui_x1));
	ui->ui_pr = IPPROTO_UDP;
	ui->ui_len = htons(len + sizeof(struct udphdr));	
	ui->ui_src = inp->laddr;
	ui->ui_dst = inp->faddr;
	ui->ui_sport = inp->lport;
	ui->ui_dport = inp->fport;
	ui->ui_ulen = ui->ui_len;
	ui->ui_sum = 0;

	if (udpcksum)
		ui->ui_sum = in_cksum(m, hdrlen, 0);

	if (ui->ui_sum == 0)
		ui->ui_sum = 0xffff;

	((struct ip*)ui)->ip_len = hdrlen;
	((struct ip*)ui)->ip_ttl = 64;	/* XXX - Fix this! */
	((struct ip*)ui)->ip_tos = 0;	/* XXX - Fix this! */


	/* XXX - add multicast options when available! */
	error = proto[IPPROTO_IP]->pr_output(m,
					inp->inp_options, &inp->inp_route,
					inp->inp_socket->so_options & (SO_DONTROUTE | SO_BROADCAST),
					NULL /* inp->inp_moptions */ );

	if (addr) {
		in_pcbdisconnect(inp); /* remove temporary route */
		inp->laddr = laddr;
	}

	return error;

release:
	m_freem(m);
	return error;
}

int udp_userreq(struct socket *so, int req,
			    struct mbuf *m, struct mbuf *addr, struct mbuf *ctrl)
{
	struct inpcb *inp = sotoinpcb(so);
	int error = 0;

	if (req == PRU_CONTROL)
		return in_control(so, (int)m, (caddr_t)addr, (struct ifnet *)ctrl);

	if (inp == NULL && req != PRU_ATTACH) {
		error = EINVAL;
		goto release;
	}

	switch (req) {
		case PRU_ATTACH:
			/* we don't replace an existing inpcb! */
			if (inp != NULL) {
				error = EINVAL;
				break;
			}
			error = in_pcballoc(so, &udb); /* udp head */
			if (error)
				break;
			error = soreserve(so, udp_sendspace, udp_recvspace);
			if (error)
				break;
			/* XXX - this is a hack! This should be the default ip TTL */
			((struct inpcb*) so->so_pcb)->inp_ip.ip_ttl = 64;
			break;
		case PRU_DETACH:
			/* This should really be protected when in kernel... */
			if (inp == udp_last_inpcb)
				udp_last_inpcb = &udb;
			in_pcbdetach(inp);
			break;
		case PRU_BIND:
			/* XXX - locking */
			error = in_pcbbind(inp, addr);
			break;
		case PRU_SEND:
			/* we can use this as we're in the same module... */
			return udp_output(inp, m, addr, ctrl);
		case PRU_LISTEN:
			error = EINVAL;//EOPNOTSUPP;
			break;
		case PRU_CONNECT:
			if (inp->faddr.s_addr != INADDR_ANY) {
				error = EISCONN;
				break;
			}
			error = in_pcbconnect(inp, addr);
			if (error == 0)
				soisconnected(so);
			break;
		case PRU_CONNECT2:
			error = EINVAL;//EOPNOTSUPP;
			break;
		case PRU_ACCEPT:
			error = EINVAL;//EOPNOTSUPP;
			break;
		case PRU_DISCONNECT:
			if (inp->faddr.s_addr == INADDR_ANY) {
				error = ENOTCONN;
				break;
			}
			in_pcbdisconnect(inp);
			inp->laddr.s_addr = INADDR_ANY;
			so->so_state &= ~SS_ISCONNECTED;
			break;
		default:
			printf("Unknown options passed to udp_userreq (%d)\n", req);
	}

release:
	if (ctrl) {
		printf("UDP control retained!\n");
		m_freem(ctrl);
	}
	if (m)
		m_freem(m);

	return error;
}

int udp_input(struct mbuf *buf, int hdrlen)
{
	struct ip *ip = mtod(buf, struct ip*);
	struct udphdr *udp = (struct udphdr*)((caddr_t)ip + hdrlen);
	uint16 ck = 0;
	int len;
	struct ip saved_ip;
	struct mbuf *opts = NULL;
	struct inpcb *inp = NULL;

#if SHOW_DEBUG
        dump_udp(buf);
#endif
	/* check and adjust sizes as required... */
	len = ntohs(udp->uh_ulen);//ip->length - hdrlen;
	if (len + hdrlen != ip->ip_len) {
		if (len + hdrlen > ip->ip_len)
			/* we got a corrupt packet as we are missing data... */
			goto bad;
		m_adj(buf, len + hdrlen - ip->ip_len);
	}
	saved_ip = *ip;

	if (udpcksum && udp->uh_sum) {
		memset(&((struct ipovly*)ip)->ih_x1[0], 0, sizeof(((struct ipovly*)ip)->ih_x1));
		((struct ipovly*)ip)->ih_len = udp->uh_ulen;
		/* XXX - if we have options we need to be careful when calculating the
		 * checksum here...
		 */
		if ((ck = in_cksum(buf, len + sizeof(*ip), 0)) != 0) {
			udpstat.udps_badsum++;
			m_freem(buf);
			printf("udp_input: UDP Checksum check failed. (%d over %ld bytes)\n", ck, len + sizeof(*ip));
			return 0;
		}
	}
	inp = udp_last_inpcb;

	if (inp == NULL ||
	    inp->lport != udp->uh_dport ||
		inp->fport != udp->uh_sport ||
		inp->faddr.s_addr != ip->ip_src.s_addr ||
		inp->laddr.s_addr != ip->ip_dst.s_addr) {

		inp = in_pcblookup(&udb, ip->ip_src, udp->uh_sport,
				   ip->ip_dst, udp->uh_dport, INPLOOKUP_WILDCARD);
		if (inp)
			udp_last_inpcb = inp;
	}
	if (!inp) {
		atomic_add(&udpstat.udps_noport, 1);
		if (buf->m_flags & (M_BCAST | M_MCAST)) {
			atomic_add(&udpstat.udps_noportbcast, 1);
			goto bad;
		}
		*ip = saved_ip;
		ip->ip_len += hdrlen;
		printf("UDP: we'd send an ICMP reply...\n");
		/* XXX - send ICMP reply... */
		return 0;
	}

	udp_in.sin_port = udp->uh_sport;
	udp_in.sin_addr = ip->ip_src;

	if (inp->inp_flags & INP_CONTROLOPT) {
		printf("INP Control Options to process!\n");
		/* XXX - add code to do this... */
	}

	hdrlen += sizeof(struct udphdr);
	buf->m_len -= hdrlen;
	buf->m_pkthdr.len -= hdrlen;
	buf->m_data += hdrlen;

	if (sbappendaddr(&inp->inp_socket->so_rcv, (struct sockaddr*)&udp_in,
			buf, opts) == 0) {
		goto bad;
	}

	sorwakeup(inp->inp_socket);
	return 0;

bad:
	if (opts)
		m_freem(opts);
	m_freem(buf);

	return 0;
}

void udp_init(void)
{
	udb.inp_prev = udb.inp_next = &udb;
	udp_sendspace = 9216; /* default size */
	udp_recvspace = 41600; /* default size */
	udp_in.sin_len = sizeof(udp_in);
	memset(&udpstat, 0, sizeof(udpstat));
		
	memset(proto, 0, sizeof(struct protosw *) * IPPROTO_MAX);
#ifndef _KERNEL_MODE
	add_protosw(proto, NET_LAYER2);
#else
	core->add_protosw(proto, NET_LAYER2);
#endif
}

static struct protosw my_proto = {
	"UDP Module",
	UDP_MODULE_PATH,
	SOCK_DGRAM,
	NULL,
	IPPROTO_UDP,
	PR_ATOMIC | PR_ADDR,
	NET_LAYER3,
	
	&udp_init,
	&udp_input,
	NULL,                  /* pr_output */
	&udp_userreq,
	NULL,                  /* pr_sysctl */
	NULL,                  /* pr_ctloutput */
		
	NULL,
	NULL
};

#ifndef _KERNEL_MODE
static void udp_protocol_init(void)
{
	add_domain(NULL, AF_INET);
	add_protocol(&my_proto, AF_INET);
}

struct protocol_info protocol_info = {
	"UDP Module",
	&udp_protocol_init
};

#else /* kernel setup */

static status_t k_init(void)
{
	if (!core)
		get_module(CORE_MODULE_PATH, (module_info**)&core);
	
	core->add_domain(NULL, AF_INET);
	core->add_protocol(&my_proto, AF_INET);
	
	return 0;
}

static status_t udp_ops(int32 op, ...)
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

static module_info my_module = {
	UDP_MODULE_PATH,
	B_KEEP_LOADED,
	udp_ops
};

_EXPORT module_info *modules[] = {
	&my_module,
	NULL
};

#endif
