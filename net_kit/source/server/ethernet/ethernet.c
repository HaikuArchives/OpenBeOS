/* ethernet.c
 * ethernet encapsulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <kernel/OS.h>

#include "net_misc.h"
#include "protocols.h"
#include "ethernet.h"
#include "mbuf.h"
#include "net_module.h"

static loaded_net_module *net_modules;
static int *prot_table;

uint8 ether_bcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };


static int convert_proto(uint16 p)
{
	switch (p) {
		case ETHER_ARP:
			return NS_ARP;
		case ETHER_IPV4:
			return NS_IPV4;
		default:
			return -1;
	}
}

/* what should the return value be? */
/* should probably also pass a structure that identifies the interface */
int ethernet_input(struct mbuf *buf)
{
	ethernet_header *eth = mtod(buf, ethernet_header *);
	char srca[17], dsta[17];
	uint16 proto = ntohs(eth->type);
	int len = sizeof(ethernet_header);
	int fproto = convert_proto(proto);

	if (memcmp((void*)&eth->dest, (void*)&ether_bcast, 6) == 0)
		buf->m_flags |= M_BCAST;
		
	if (proto < 1500) {
		eth802_header *e8 = mtod(buf, eth802_header*);
		proto = htons(e8->type);
		printf("It's an 802.x encapsulated packet - type %04x\n", ntohs(e8->type));
		len = sizeof(eth802_header);
	}
	
	sprintf(dsta, "%02x:%02x:%02x:%02x:%02x:%02x", 
			eth->dest.addr[0], eth->dest.addr[1],
			eth->dest.addr[2], eth->dest.addr[3],
			eth->dest.addr[3], eth->dest.addr[5]);
	sprintf(srca, "%02x:%02x:%02x:%02x:%02x:%02x", 
			eth->src.addr[0], eth->src.addr[1],
			eth->src.addr[2], eth->src.addr[3],
			eth->src.addr[3], eth->src.addr[5]);						
	printf("Ethernet packet from %s to %s:", srca, dsta);

	if (buf->m_flags & M_BCAST)	
		printf(" BCAST");
	
	printf(" proto ");
	m_adj(buf, len);
	
	switch (proto) {
		case ETHER_ARP:
			printf("ARP\n");
			break;
		case ETHER_RARP:
			printf("RARP\n");
			break;
		case ETHER_IPV4:
			printf("IPv4\n");
			break;
		case ETHER_IPV6:
			printf("IPv6\n");
		default:
			printf("unknown (%04x)\n", proto);
	}

	if (fproto >= 0)
		net_modules[prot_table[fproto]].mod->input(buf);
	else 
		printf("Failed to determine a valid Ethernet protocol\n");
	
	printf("\n");
	
	m_freem(buf);
	return 0;	
}

int ether_init(loaded_net_module *ln, int *pt)
{
	net_modules = ln;
	prot_table = pt;

	return 0;
}

int ether_dev_init(ifnet *dev)
{
	if (dev->type == IFD_ETHERNET)
		/* we're interested and will be used */
		return 0;

	return -1;
}

net_module net_module_data = {
	"Ethernet/802.x module",
	NS_ETHER,
	NET_LAYER1,

	&ether_init,
	&ether_dev_init,
	&ethernet_input,
	NULL
};

