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
#include "ipv4/ipv4.h"

uint8 ether_bcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

/* what should the return value be? */
/* should probably also pass a structure that identifies the interface */
int ethernet_input(struct mbuf *buf)
{
	ethernet_header *eth = mtod(buf, ethernet_header *);
	char srca[17], dsta[17];
	uint16 proto = ntohs(eth->type);
	int len = sizeof(ethernet_header);

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
		case PROT_ARP:
			printf("ARP\n");
			break;
		case PROT_RARP:
			printf("RARP\n");
			break;
		case PROT_IPV4:
			printf("IPv4\n");
			ipv4_input(buf);
			break;
		case PROT_IPV6:
			printf("IPv6\n");
		default:
			printf("unknown (%04x)\n", ntohs(eth->type));
	}
		
	printf("\n");
	
	m_freem(buf);
	return 0;	
}

