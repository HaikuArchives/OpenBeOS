/* icmp.c
 */
 
#include <stdio.h>

#include "mbuf.h"
#include "icmp.h"
#include "ipv4/ipv4.h"
#include "net_module.h"
#include "protocols.h"
#include "sys/socket.h"

static loaded_net_module *global_modules;
static int *prot_table;

#if SHOW_DEBUG
static void dump_icmp(struct mbuf *buf)
{
	ipv4_header *ip = mtod(buf, ipv4_header *);
	icmp_header *ic =  (icmp_header*)((caddr_t)ip + (ip->hl * 4));

        printf("ICMP: ");
        switch (ic->type) {
		case ICMP_ECHO_RQST:
			printf ("Echo request\n");
			break;
		case ICMP_ECHO_RPLY:
			printf("echo reply\n");
			break;
		default:
			printf("?? type = %d\n", ic->type);
	}
}
#endif

int icmp_input(struct mbuf *buf)
{
	ipv4_header *ip = mtod(buf, ipv4_header *);	
	icmp_header *ic;
	int icl = ntohs(ip->length) - (ip->hl * 4);

	ic = (icmp_header*)((caddr_t)ip + (ip->hl * 4));

#if SHOW_DEBUG
	dump_icmp(buf);
#endif

	if (in_cksum(buf, icl, (ip->hl * 4)) != 0) {
		printf("Checksum failed!\n");
		m_freem(buf);
		return 0;
	}	

	switch (ic->type) {
		case ICMP_ECHO_RQST: {
			icmp_echo *ie = (icmp_echo *)ic;
			struct sockaddr sa;
			ie->hdr.type = ICMP_ECHO_RPLY;

			/* fill in target structure... */
			sa.sa_family = AF_INET;
			sa.sa_len = 4;
			memcpy(&sa.sa_data, &ip->src, 4);

			ie->hdr.cksum = 0;
			ie->hdr.cksum = in_cksum(buf, icl, (ip->hl * 4));
			ip->src = ip->dst;

			global_modules[prot_table[NS_IPV4]].mod->output(buf, NS_ICMP, &sa);

			return 0;
			break;
		}
		case ICMP_ECHO_RPLY:
			break;
		default:
	}
	m_freem(buf);
	return 0;
}

static int icmp_init(loaded_net_module *ln, int *pt)
{
	global_modules = ln;
	prot_table = pt;
	return 0;
}

net_module net_module_data = {
	"ICMP Module",
	NS_ICMP,
	NET_LAYER2,
        0,      /* users can't create sockets in this module! */
        0,

	&icmp_init,
	NULL,
	&icmp_input,
	NULL,
	NULL
};

