/* ipv4.c
 * simple ipv4 implementation
 */

#include <stdio.h>
#include <unistd.h>
#include <kernel/OS.h>
#include <malloc.h>

#include "ipv4/ipv4.h"
#include "protocols.h"
#include "net_module.h"
#include "mbuf.h"

loaded_net_module *net_modules;
int *prot_table;

int ipv4_input(struct mbuf *buf)
{
	ipv4_header *ip = mtod(buf, ipv4_header *);


	if (in_cksum(buf, ip->hl * 4) != 0) {
		printf("Bogus checksum! Discarding packet.\n");
		m_freem(buf);
		return 0;
	}

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
	/* move the pointer... not sure if all protocols will want this! */
	m_adj(buf, (ip->hl * 4));

	switch(ip->prot) {
		case IP_ICMP:
			printf("ICMP\n");
			break;
		case IP_UDP: {
			printf("UDP\n");
			break;
		}
		case IP_TCP:
			printf("TCP\n");
			break;
		default:
			printf("unknown (0x%02x)\n", ip->prot);
	}

	if (net_modules[prot_table[ip->prot]].mod->input)
		net_modules[prot_table[ip->prot]].mod->input(buf);

	return 0; 
}

int ipv4_init(loaded_net_module *ln, int *pt)
{
	net_modules = ln;
	prot_table = pt;

	return 0;
}

int ipv4_dev_init(ifnet *dev)
{
	ifaddr *ifa = malloc(sizeof(ifaddr));

	ifa->if_addr.sa_family = AF_INET;
	ifa->if_addr.sa_len = 4;
	/* Yuck - hard coded address! */
	ifa->if_addr.sa_data[0] = 192; 
	ifa->if_addr.sa_data[1] = 168;
	ifa->if_addr.sa_data[2] = 0;
	ifa->if_addr.sa_data[3] = 133;

	ifa->ifn = dev;
	ifa->next = NULL;
	if (dev->if_addrlist)
		dev->if_addrlist->next = ifa;
	else
		dev->if_addrlist = ifa;

	insert_local_address(&ifa->if_addr, dev);
	/* so far all devices will use this!! */
	return 1;
}

net_module net_module_data = {
	"IPv4 module",
	NS_IPV4,
	NET_LAYER2,

	&ipv4_init,
	&ipv4_dev_init,
	&ipv4_input, 
	NULL,
	NULL
};
