/* ipv4.c
 * simple ipv4 implementation
 */

#include <stdio.h>
#include <unistd.h>
#include <kernel/OS.h>
#include <malloc.h>

#include "ipv4/ipv4.h"
#include "ipv4/ipv4_var.h"	/* for stats */
#include "protocols.h"
#include "net_module.h"
#include "mbuf.h"

loaded_net_module *net_modules;
int *prot_table;
static struct ipstat	ipstat;

#if SHOW_DEBUG
static void dump_ipv4_header(struct mbuf *buf)
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
}
#endif

int ipv4_input(struct mbuf *buf)
{
	ipv4_header *ip = mtod(buf, ipv4_header *);

#if SHOW_DEBUG
	dump_ipv4_header(buf);
#endif

	atomic_add(&ipstat.ips_total, 1);

	if (ip->ver != 4) {
		printf("Wrong IP version!\n");
		atomic_add(&ipstat.ips_badvers, 1);
		m_freem(buf);
		return 0;
	}
	if (in_cksum(buf, ip->hl * 4, 0) != 0) {
		printf("Bogus checksum! Discarding packet.\n");
		atomic_add(&ipstat.ips_badsum, 1);
		m_freem(buf);
		return 0;
	}

	if (net_modules[prot_table[ip->prot]].mod->input)
		net_modules[prot_table[ip->prot]].mod->input(buf);

	return 0; 
}

int ipv4_output(struct mbuf *buf, int proto, struct sockaddr *tgt)
{
	ipv4_header *ip = mtod(buf, ipv4_header*);

	switch (proto) {
		case NS_ICMP:
			/* assume all filled in correctly...just need to set the 
			 * tgt address */
		default:
	}

	memcpy(&ip->dst, &tgt->sa_data, 4);
	ip->hdr_cksum = 0;
	ip->hdr_cksum = in_cksum(buf, (ip->hl * 4), 0);
	buf->m_pkthdr.rcvif->output(buf, NS_IPV4, tgt);
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
	&ipv4_output,
	NULL
};

