/* udp.c
 */

#include <stdio.h>

#include "mbuf.h"
#include "udp.h"
#include "net_misc.h"
#include "net_module.h"
#include "protocols.h"
#include "ipv4/ipv4.h"

int *prot_table;
loaded_net_module *net_modules;

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

int udp_input(struct mbuf *buf)
{
	ipv4_header *ip = mtod(buf, ipv4_header*);
	udp_header *udp = (udp_header*)((caddr_t)ip + (ip->hl * 4));

#if SHOW_DEBUG
	dump_udp(buf);
#endif

	m_freem(buf);

	return 0;
}

int udp_init(loaded_net_module *ln, int *pt)
{
	net_modules = ln;
	prot_table = pt;

	return 0;
}

net_module net_module_data = {
	"UDP module",
	NS_UDP,
	NET_LAYER3,

	&udp_init,
	NULL,
	&udp_input,
	NULL,
	NULL
};
 
