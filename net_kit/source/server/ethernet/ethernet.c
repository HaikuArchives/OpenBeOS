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


/* what should the return value be? */
/* should probably also pass a structure that identifies the interface */
int ethernet_input(struct mbuf *buf)
{
	ethernet_header *eth = mtod(buf, ethernet_header *);
	char srca[17], dsta[17];
	
	if (eth->type < 1500) {
		printf("It's an 802.x encapsulated packet\n");
		return 0;
	}
	sprintf(dsta, "%02x:%02x:%02x:%02x:%02x:%02x", 
			eth->dest.addr[0], eth->dest.addr[1],
			eth->dest.addr[2], eth->dest.addr[3],
			eth->dest.addr[3], eth->dest.addr[5]);
	sprintf(srca, "%02x:%02x:%02x:%02x:%02x:%02x", 
			eth->src.addr[0], eth->src.addr[1],
			eth->src.addr[2], eth->src.addr[3],
			eth->src.addr[3], eth->src.addr[5]);						
	printf("Ethernet packet from %s to %s: proto ", srca, dsta);
	
	switch (ntohs(eth->type)) {
		case PROT_ARP:
			printf("ARP\n");
			break;
		case PROT_RARP:
			printf("RARP\n");
			break;
		case PROT_IPV4:
			printf("IPv4\n");
			break;
		case PROT_IPV6:
			printf("IPv6\n");
		default:
			printf("unknown (%04x)\n", ntohs(eth->type));
	}
	
	printf("\n");
	
	m_free(buf);
	return 0;	
}

