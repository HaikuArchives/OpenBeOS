/* udp.c
 */

#include <stdio.h>

#include "mbuf.h"
#include "udp.h"
#include "net_misc.h"
#include "net_module.h"
#include "protocols.h"
#include "ipv4/ipv4.h"
#include "netinet/in_pcb.h"

int *prot_table;
loaded_net_module *net_modules;
static struct inpcb udb;	/* head of the UDP PCB list! */
static struct inpcb *udp_last_inpcb = NULL;
static struct sockaddr_in udp_in;
static int udpcksum = 1;	/* do we calculate the UDP checksum? */

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
	error = net_modules[prot_table[IPPROTO_IP]].mod->output(m,
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

int udp_userreq(struct socket *so, int req, struct mbuf *m, struct mbuf *addr, struct mbuf *ctrl)
{
	struct inpcb *inp = sotoinpcb(so);
	int error = 0;

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
		case PRU_BIND:
			/* XXX - locking */
			error = in_pcbbind(inp, addr);
			break;
		case PRU_SEND:
			/* we can use this as we're in the same module... */
			return udp_output(inp, m, addr, ctrl);
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
	struct inpcb *inp;

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

	p->length = udp->length;
	p->zero = 0; /* just to make sure */
	p->lead = 0;
	p->lead2 = 0;

	/* XXX - if we have options we need to be careful when calculating the
	 * checksum here...
	 */
	if ((ck = in_cksum(buf, len + sizeof(*ip), 0)) != 0) {
		printf("udp_input: UDP Checksum check failed. (%d over %ld bytes)\n", ck, len + sizeof(*ip));
		dump_buffer(mtod(buf, void *), len + hdrlen);
		goto bad;
	}

	inp = udp_last_inpcb;
	if (!inp || inp->lport != udp->dst_port ||
		inp->fport != udp->src_port ||
		inp->faddr.s_addr != ip->src.s_addr ||
		inp->laddr.s_addr != ip->dst.s_addr) {

		inp = in_pcblookup(&udb, ip->src, udp->src_port,
				   ip->dst, udp->dst_port, INPLOOKUP_WILDCARD);
		if (inp)
			udp_last_inpcb = inp;
	}
	if (!inp) {
		if (buf->m_flags & (M_BCAST | M_MCAST)) {
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

int udp_init(loaded_net_module *ln, int *pt)
{
	net_modules = ln;
	prot_table = pt;
	
	udb.inp_prev = udb.inp_next = &udb;
	udp_sendspace = 9216; /* default size */
	udp_recvspace = 41600; /* default size */

	return 0;
}

net_module net_module_data = {
	"UDP module",
	NS_UDP,
	NET_LAYER3,
	AF_INET,
	SOCK_DGRAM,
	PR_ATOMIC | PR_ADDR,

	&udp_init,
	NULL,
	&udp_input,
	NULL,
	NULL,
	&udp_userreq
};
 
