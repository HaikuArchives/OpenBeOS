/* ipv4.c
 * simple ipv4 implementation
 */

#include <stdio.h>
#include <unistd.h>

#include "ipv4/ipv4.h"
#include "protocols.h"

static void dump_ipv4_addr(char *msg, ipv4_addr *ad)
{
	uint8 *b = (uint8*)ad;
	printf("%s %d.%d.%d.%d\n", msg, b[0], b[1], b[2], b[3]);
}

int ipv4_input(struct mbuf *buf)
{
	ipv4_header *ip = mtod(buf, ipv4_header *);
	
	printf("IPv4 Header :\n");
	printf("            : version       : %d\n", IPV4_VERSION(ip));
	printf("            : header length : %d\n", IPV4_HDR_LENGTH(ip));
	printf("            : tos           : %d\n", ip->tos);
	printf("            : total length  : %d\n", ntohs(ip->length));
	printf("            : id            : %d\n", ntohs(ip->id));
	printf("            : flags         : 0x%02x\n", IPV4_FLAGS(ip));
	printf("            : frag offset   : %d\n", ntohs(IPV4_FRAG(ip)));
	printf("            : ttl           : %d\n", ip->ttl);
	printf("            : protocol      : ");
	switch(ip->prot) {
		case IP_ICMP:
			printf("ICMP\n");
			break;
		case IP_UDP:
			printf("UDP\n");
			break;
		case IP_TCP:
			printf("TCP\n");
			break;
		default:
			printf("unknown (0x%02x)\n", ip->prot);
	}
	dump_ipv4_addr("            : src address   :", &ip->src);
	dump_ipv4_addr("            : dst address   :", &ip->dst);

	return 0; 
}