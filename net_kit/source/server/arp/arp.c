/* arp.c
 * simple arp implentation
 */
 
#include <stdio.h>
#include <malloc.h>

#include "mbuf.h"
#include "if.h"
#include "net_misc.h"
#include "arp/arp.h"
#include "protocols.h"
#include "net_module.h"
#include "nhash.h"
#include "pools.h"
#include "net_misc.h"

loaded_net_module *global_modules;
int *prot_table;
static net_hash *arphash;
static arp_cache_entry *arpcache;
static pool_ctl *arpp;
static sem_id arpq_lock;
static arp_q_entry *arp_lookup_q; 
/* these are in seconds... */
static int arp_prune = 300; 	/* time interval we prune the arp cache? 5 minutes */
static int arp_keep  = 1200;	/* length of time we keep entries... (20 mins) */
static int arp_noflood = 1200;	/* seconds between arp flooding */

/* stats */
static int arp_inuse = 0;	/* how many entries do we have? */
static int arp_allocated = 0;	/* how many arp entries have we created? */
static int arp_maxtries = 5;	/* max tries before a pause */

int arp_input(struct mbuf *buf)
{
	ether_arp *arp = mtod(buf, ether_arp*);

	printf("arp request :\n");
	printf("            : hardware type : %s\n",
			ntohs(arp->arp_ht) == 1 ? "ethernet" : "unknown");
	printf("            : protocol type : %s\n",
			ntohs(arp->arp_pro) == ETHER_IPV4 ? "IPv4" : "unknown");
	printf("            : hardware size : %d\n", arp->arp_hsz);
	printf("            : protocol size : %d\n", arp->arp_psz);
	printf("            : op code       : ");
	
	switch (ntohs(arp->arp_op)) {
		case ARP_RQST:
			printf("ARP request\n            : ");
			print_ether_addr(&arp->sender);
			printf(" [");
			print_ipv4_addr(&arp->sender_ip);
			printf("] wants MAC address for ");
			print_ipv4_addr(&arp->target_ip);
			printf("\n");
			break;
		case ARP_RPLY: {
			arp_cache_entry *ace;
			ace = (arp_cache_entry *)nhash_get(arphash, &arp->target_ip, 4);
			if (!ace) {
				ace = (arp_cache_entry *)pool_get(arpp);
				ace->ip_addr.sa_family = AF_INET;
				ace->ip_addr.sa_len = 4;
				memcpy(&ace->ip_addr.sa_data, &arp->target_ip, 4);
				ace->ll_addr.sa_family = AF_LINK;
                        	ace->ll_addr.sa_len = 6;
                        	memcpy(&ace->ll_addr.sa_data, &arp->target, 6);
				ace->expires = system_time() + (arp_keep * 1000000);
				if (arpcache) {
					arpcache->prev->next = ace;
					ace->prev = arpcache->prev;
					ace->next = arpcache;
					arpcache->prev = ace;
				} else {
					arpcache = ace;
					arpcache->next = ace;
					arpcache->prev = ace;
				}
				nhash_set(arphash, &ace->ip_addr.sa_data, 4, (void*)ace);
				/* see if we have any waiters... */
			} else {
				printf("Got cached value!\n");
			}

			printf("ARP reply\n");

			break;
		}
		default:
			printf("unknown (%d)\n", ntohs(arp->arp_op));
	}
printf("freeing mbuf %p\n", buf);
	m_freem(buf);
	return 0;
}

static void arp_send_request(ifnet *ifa, struct sockaddr *from, 
				struct sockaddr *tgt)
{
	struct mbuf *buf = m_gethdr(MT_DATA);
	ether_arp *arp;

	m_reserve(buf, 16);
	arp =  mtod(buf, ether_arp*);

	arp->arp_ht = htons(1);
	arp->arp_hsz = 6;

	if (tgt->sa_family != AF_INET) {
		printf("target type doesn't agree with IPv4, 
			killing packet %p\n", buf);
		m_freem(buf);
		return;
	}
	arp->arp_pro = htons(ETHER_IPV4);
	arp->arp_psz = 4;

	arp->arp_op = htons(ARP_RQST);

	memcpy(&arp->sender, &ifa->link_addr->sa_data, 6);
	memcpy(&arp->sender_ip, &from->sa_data, 4);
	memset(&arp->target, 0, 6);
	memcpy(&arp->target_ip, &tgt->sa_data, 4);

	buf->m_flags |= M_BCAST;
	buf->m_pkthdr.len = 28;
	buf->m_len = 28;

	global_modules[prot_table[NS_ETHER]].mod->output(buf, NS_ARP, ifa, tgt);
}

int arp_init(loaded_net_module *ln, int *pt)
{
	global_modules = ln;
	prot_table = pt;

	if (!arphash)
		arphash = nhash_make();

	if (!arpp) {
		pool_init(&arpp, sizeof(arp_cache_entry));
		if (!arpp) {
			printf("failed to create a pool!\n");
			return -1;
		}
	}
	arpcache = NULL;
	arpq_lock = create_sem(1, "arp_q_lock");

	return 0;
}

struct sockaddr *arp_lookup(struct sockaddr *src, struct sockaddr *tgt)
{
	arp_cache_entry *ace;
	arp_q_entry *aqe;

	ace = (arp_cache_entry *)nhash_get(arphash, &tgt->sa_data, 
			tgt->sa_family == AF_INET ? 4 : 6);

	if (ace) {
		ace->expires = system_time() + (arp_keep * 1000000);
		return &ace->ll_addr;
	}

	/* ok, we didn't get one! */
	acquire_sem(arpq_lock);
	aqe = arp_lookup_q;
	while (aqe) {
		if (compare_sockaddr(aqe->tgt, tgt))
			break;
		aqe = aqe->next;
	}
	if (!aqe) {
		aqe = malloc(sizeof(struct arp_q_entry));
		aqe->src = src;
		aqe->tgt = tgt;
		aqe->ifn = interface_for_address(src);
		aqe->attempts = 1;
		if (arp_lookup_q)
			aqe->next = arp_lookup_q;
		arp_lookup_q = aqe;
		arp_send_request(aqe->ifn, src, tgt);
	}
	release_sem(arpq_lock);

	return NULL;
}

net_module net_module_data = {
	"ARP module",
	NS_ARP,
	NET_LAYER1,

	&arp_init,
	NULL, //&arp_dev_init,
	&arp_input, 
	NULL,
	&arp_lookup
};

