/* udp.c
 */

#include <stdio.h>

#include "mbuf.h"
#include "udp.h"
#include "net_misc.h"
#include "net_module.h"
#include "protocols.h"

int *prot_table;
loaded_net_module *net_modules;

int udp_input(struct mbuf*buf)
{
	udp_header *udp = mtod(buf, udp_header *);

	printf("udp_header  :\n");
	printf("            : src_port      : %d\n", ntohs(udp->src_port));
        printf("            : dst_port      : %d\n", ntohs(udp->dst_port));
	printf("            : udp length    : %d bytes\n", ntohs(udp->length));

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
	NULL
};
 
