/* ipv4.c
 * simple ipv4 implementation
 */

#include <stdio.h>
#include <unistd.h>

#include "ipv4/ipv4.h"
#include "protocols.h"

/* hack to get the thing working... */
#include "udp/udp.h"

static void dump_ipv4_addr(char *msg, ipv4_addr *ad)
{
	uint8 *b = (uint8*)ad;
	printf("%s %d.%d.%d.%d\n", msg, b[0], b[1], b[2], b[3]);
}

int ipv4_input(struct mbuf *buf)
{
	ipv4_header *ip = mtod(buf, ipv4_header *);
	
	printf("IPv4 Header :\n");
	printf("            : version       : %d\n", ip->ver);
	printf("            : header length : %d\n", ip->hl);
	printf("            : tos           : %d\n", ip->tos);
	printf("            : total length  : %d\n", ntohs(ip->length));
	printf("            : id            : %d\n", ntohs(ip->id));
	printf("            : flags         : 0x%02x\n", IPV4_FLAGS(ip));
	printf("            : frag offset   : %d\n", ntohs(IPV4_FRAG(ip)));
	printf("            : ttl           : %d\n", ip->ttl);
        dump_ipv4_addr("            : src address   :", &ip->src);
        dump_ipv4_addr("            : dst address   :", &ip->dst);

	printf("            : protocol      : ");
	/* move the pointer... */
	/* Yuck - big hack - adding 14 isn't correct! */
	m_adj(buf, (ip->hl * 4) + 14);

	switch(ip->prot) {
		case IP_ICMP:
			printf("ICMP\n");
			break;
		case IP_UDP:
			printf("UDP\n");
			udp_input(buf);
			break;
		case IP_TCP:
			printf("TCP\n");
			break;
		default:
			printf("unknown (0x%02x)\n", ip->prot);
	}

	return 0; 
}
