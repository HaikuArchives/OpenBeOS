/* udp.c
 */

#include <stdio.h>

#include "mbuf.h"
#include "udp.h"
#include "net_misc.h"
#include "net_module.h"
#include "protocols.h"
#include "ipv4/ipv4.h"
#include "netinet/in_var.h"
#include "netinet/in_pcb.h"
#include "sys/domain.h"
#include "sys/protosw.h"
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
	ipv4_header *ip = mtod(buf, ipv4_header*);
	udp_header *udp = (udp_header*)((caddr_t)ip + (ip->hl * 4));

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
	ui->ip.lead = ui->ip.lead2 = 0;
	ui->ip.zero = 0;
	ui->ip.prot = IPPROTO_UDP;
	ui->ip.length = htons(len + sizeof(struct udp_header));	
	ui->ip.src = inp->laddr;
	ui->ip.dest = inp->faddr;
	ui->udp.src_port = inp->lport;
	ui->udp.dst_port = inp->fport;
	ui->udp.length = ui->ip.length;
	ui->udp.cksum = 0;

	if (udpcksum)
		ui->udp.cksum = in_cksum(m, hdrlen, 0);

	if (ui->udp.cksum == 0)
		ui->udp.cksum = 0xffff;

	((ipv4_header*)ui)->length = hdrlen;
	((ipv4_header*)ui)->ttl = 64;	/* XXX - Fix this! */
	((ipv4_header*)ui)->tos = 0;	/* XXX - Fix this! */


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
			((struct inpcb*) so->so_pcb)->inp_ip.ttl = 64;
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
	ipv4_header *ip = mtod(buf, ipv4_header*);
	udp_header *udp = (udp_header*)((caddr_t)ip + hdrlen);
	pudp_header *p = (pudp_header *)ip;
	uint16 ck = 0;
	int len;
	ipv4_header saved_ip;
	struct mbuf *opts = NULL;
	struct inpcb *inp = NULL;

printf("udp_input: hdrlen = %d\n", hdrlen);

#if SHOW_DEBUG
        dump_udp(buf);
#endif
	/* check and adjust sizes as required... */
	len = ntohs(udp->length);//ip->length - hdrlen;
	if (len + hdrlen != ip->length) {
		if (len + hdrlen > ip->length)
			/* we got a corrupt packet as we are missing data... */
			goto bad;
		m_adj(buf, len + hdrlen - ip->length);
	}
	saved_ip = *ip;
printf("udp->length = %d\n", len);

	p->length = udp->length;
	p->zero = 0; /* just to make sure */
	p->lead = 0;
	p->lead2 = 0;

	/* XXX - if we have options we need to be careful when calculating the
	 * checksum here...
	 */
printf("udp: checking cksum\n");
	if ((ck = in_cksum(buf, len + sizeof(*ip), 0)) != 0) {
		printf("udp_input: UDP Checksum check failed. (%d over %ld bytes)\n", ck, len + sizeof(*ip));
		goto bad;
	}
	inp = udp_last_inpcb;

	if (inp == NULL ||
	    inp->lport != udp->dst_port ||
		inp->fport != udp->src_port ||
		inp->faddr.s_addr != ip->src.s_addr ||
		inp->laddr.s_addr != ip->dst.s_addr) {

printf("doing in_pcblookup for %08lx:%d %08lx:%d\n",
		ntohl(ip->src.s_addr), ntohs(udp->src_port),
		ntohl(ip->dst.s_addr), ntohs(udp->dst_port));
		
		inp = in_pcblookup(&udb, ip->src, udp->src_port,
				   ip->dst, udp->dst_port, INPLOOKUP_WILDCARD);
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
		ip->length += hdrlen;
		printf("UDP: we'd send an ICMP reply...\n");
		/* XXX - send ICMP reply... */
		return 0;
	}

	udp_in.sin_port = udp->src_port;
	udp_in.sin_addr = ip->src;

	if (inp->inp_flags & INP_CONTROLOPT) {
		printf("INP Control Options to process!\n");
		/* XXX - add code to do this... */
	}

	hdrlen += sizeof(udp_header);
	buf->m_len -= hdrlen;
	buf->m_pkthdr.len -= hdrlen;
	buf->m_data += hdrlen;

printf("calling sbappendaddr (%p)\n", sbappendaddr);
	if (sbappendaddr(&inp->inp_socket->so_rcv, (struct sockaddr*)&udp_in,
			buf, opts) == 0) {
		goto bad;
	}
printf("waking up socket!\n");
	sorwakeup(inp->inp_socket);
	return 0;

bad:
printf("udp_input: bad: freeing %p\n", buf);
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
	NULL,
	&udp_userreq,
	
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
