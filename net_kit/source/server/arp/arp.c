/* arp.c
 * simple arp implentation
 */
 
#include <stdio.h>

#include "mbuf.h"
#include "net_misc.h"
#include "arp/arp.h"
#include "protocols.h"
#include "net_module.h"

loaded_net_module *net_modules;
int *protocol_table;

/* unused presently, but useful to have around...*/
/*
static void dump_buffer(char *buffer, int len) {
	uint8 *b = (uint8 *)buffer;
	int i;
	
	printf ("  ");
	for (i=0;i<len;i+=2) {
		if (i%16 == 0)
			printf("\n  ");
		printf(" %02x", b[i]);
		printf("%02x ", b[i+1]);
	}
	printf("\n\n");
}
*/

static void print_ipv4_addr(ipv4_addr *ip)
{
	uint8 *b = (uint8*)ip;
	printf("%d.%d.%d.%d", b[0], b[1], b[2], b[3]);
}

int arp_input(struct mbuf *buf)
{
	arp_header *arp = mtod(buf, arp_header*);

	printf("arp request :\n");
	printf("            : hardware type : %s\n",
			ntohs(arp->hard_type) == 1 ? "ethernet" : "unknown");
	printf("            : protocol type : %s\n",
			ntohs(arp->prot) == ETHER_IPV4 ? "IPv4" : "unknown");
	printf("            : hardware size : %d\n", arp->hard_size);
	printf("            : protocol size : %d\n", arp->prot_size);
	printf("            : op code       : ");
	
	switch (ntohs(arp->op)) {
		case ARP_RQST:
			printf("ARP request\n");
			printf("            : %02x:%02x:%02x:%02x:%02x:%02x [", 
				arp->sender.addr[0], arp->sender.addr[1],
				arp->sender.addr[2], arp->sender.addr[3],
				arp->sender.addr[3], arp->sender.addr[5]);
			print_ipv4_addr(&arp->sender_ip);
			printf("] wants MAC address for ");
			print_ipv4_addr(&arp->target_ip);
			printf("\n");
			break;
		case ARP_RPLY:
			printf("ARP reply\n");
			break;
		default:
			printf("unknown (%d)\n", ntohs(arp->op));
	}

	return 0;
}

int arp_init(loaded_net_module *lm, int *pt)
{
	net_modules = lm;
	protocol_table = pt;

	return 0;
}

net_module net_module_data = {
	"ARP module",
	NS_ARP,
	NET_LAYER1,

	&arp_init,
	NULL,
	&arp_input,
	NULL
};

