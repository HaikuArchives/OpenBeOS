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

static uint32 udp_sendspace;	/* size of send buffer */
static uint32 udp_recvspace;	/* size of recieve buffer */


/* temporary hack */
#undef SHOW_DEBUG
#define SHOW_DEBUG 1

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

int udp_input(struct mbuf *buf)
{
	ipv4_header *ip = mtod(buf, ipv4_header*);
	udp_header *udp = (udp_header*)((caddr_t)ip + (ip->hl * 4));
	pudp_header *p = (pudp_header *)ip;
	uint16 ck = 0;
	int len = ntohs(udp->length) + sizeof(ipv4_header);
	/* save a copy in case we need it... */
/* XXX - currently unused...
	ipv4_header saved = *ip;
*/

#if SHOW_DEBUG
        dump_udp(buf);
#endif

	p->length = udp->length;
	p->zero = 0; /* just to make sure */
	p->lead = 0;
	p->lead2 = 0;

	if ((ck = in_cksum(buf, len, 0)) != 0) {
		printf("UDP Checksum check failed. (%d over %d bytes)\n", ck, len);
		m_freem(buf);
		return 0;
	}

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

	&udp_init,
	NULL,
	&udp_input,
	NULL,
	NULL,
	&udp_userreq
};
 
