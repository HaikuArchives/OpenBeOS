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
static struct route icmprt;

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

int icmp_input(struct mbuf *buf, int hdrlen)
{
	ipv4_header *ip = mtod(buf, ipv4_header *);	
	icmp_header *ic;
	int icl = ip->length - hdrlen;
	struct route rt;
	int error = 0;
	struct in_addr tmp;

	ic = (icmp_header*)((caddr_t)ip + hdrlen);
	rt.ro_rt = NULL;

#if SHOW_DEBUG
	dump_icmp(buf);
#endif

	if (in_cksum(buf, icl, hdrlen) != 0) {
		printf("Checksum failed!\n");
		m_freem(buf);
		return 0;
	}	

	switch (ic->type) {
		case ICMP_ECHO_RQST: {
			icmp_echo *ie = (icmp_echo *)ic;
			ie->hdr.type = ICMP_ECHO_RPLY;

			ie->hdr.cksum = 0;
			ie->hdr.cksum = in_cksum(buf, icl, hdrlen);
			tmp = ip->src;
			ip->src = ip->dst;
			ip->dst = tmp;

			if (satosin(&icmprt.ro_dst)->sin_addr.s_addr == ip->src.s_addr)
				rt = icmprt;
			else {
				/* fill in target structure... */
				satosin(&rt.ro_dst)->sin_family = AF_INET;
				satosin(&rt.ro_dst)->sin_len = sizeof(rt.ro_dst);
				satosin(&rt.ro_dst)->sin_addr = ip->src;
			}
			error = global_modules[prot_table[NS_IPV4]].mod->output(buf, NULL, &rt, 0, NULL);
			if (error == 0)
				icmprt = rt;
			return error;
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
	memset(&icmprt, 0, sizeof(icmprt));
	return 0;
}

net_module net_module_data = {
	"ICMP Module",
	NS_ICMP,
	NET_LAYER2,
	0,      /* users can't create sockets in this module! */
	0,
	0,

	&icmp_init,
	NULL,
	&icmp_input,
	NULL,
	NULL,
	NULL
};

