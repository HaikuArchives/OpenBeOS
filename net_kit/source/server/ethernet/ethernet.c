/* ethernet.c
 * ethernet encapsulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <kernel/OS.h>
#include <unistd.h>

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
	
	printf("Ethernet packet from ");
	print_ether_addr(&eth->src);
	printf(" to ");
	print_ether_addr(&eth->dest);

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

	if (fproto >= 0 && net_modules[prot_table[fproto]].mod->input) {
		return net_modules[prot_table[fproto]].mod->input(buf);
	} else {
		printf("Failed to determine a valid protocol fproto = %d\n", fproto);
	}

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
	status_t status;
	int on = 1;

	if (dev->type != IFD_ETHERNET)
		return 0;

        status = ioctl(dev->dev, IF_INIT, NULL, 0);
        if (status < B_OK) {
                printf("Failed to init the card at %s!\n", dev->name);
                return 0;
        }

        /* try to get the MAC address */
        status = ioctl(dev->dev, IF_GETADDR, &dev->mac, 6);
        if (status < B_OK) {
                printf("Failed to get a MAC address, ignoring %s\n", dev->name);
                return 0;
        }
        status = ioctl(dev->dev, IF_SETPROMISC, &on, 1);
        if (status < B_OK) {
		/* not a hanging offence */
                printf("failed to set %s to promiscuous\n", dev->name);
        }
		
	return 1;
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

