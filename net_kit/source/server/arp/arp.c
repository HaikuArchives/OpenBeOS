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

void walk_arp_cache(void)
{
	arp_cache_entry *c = arpcache;
	int i = 1;

	while (c) {
		printf("%2d: ", i++);
		print_ipv4_addr(&c->ip_addr.sa_data);
		printf("  ");
		print_ether_addr(&c->ll_addr.sa_data);
		printf("\n");
		c = c->next;
		if (c == arpcache)
			break;
	}
	printf("\n");
}

static void dump_arp(void *buffer)
{
	ether_arp *arp = (ether_arp*)buffer;

	printf("arp request :\n");
	printf("            : hardware type : %s\n",
                        ntohs(arp->arp_ht) == 1 ? "ethernet" : "unknown");
	printf("            : protocol type : %s\n",
		ntohs(arp->arp_pro) == ETHER_IPV4 ? "IPv4" : "unknown");
	printf("            : hardware size : %d\n", arp->arp_hsz);
	printf("            : protocol size : %d\n", arp->arp_psz);
	printf("            : op code       : ");
	switch(ntohs(arp->arp_op)) {
		case ARP_RPLY:
			printf("ARP reply\n");
			break;
		case ARP_RQST:
			printf("ARP Request\n");
			break;
		default:
			printf("Who knows? %04x\n", ntohs(arp->arp_op));
	}
	printf("            : sender        : ");
	print_ether_addr(&arp->sender);
	printf(" [");
	print_ipv4_addr(&arp->sender_ip);
	printf("]\n");
	printf("            : target        : ");
	print_ether_addr(&arp->target);
	printf(" [");
	print_ipv4_addr(&arp->target_ip);
	printf("]\n");
}

/* Returns 1 is new cache entry added, 0 if not */
static int insert_cache_entry(void *link, int lf, void *addr, int af)
{
	arp_cache_entry *ace;
	int alen = af == AF_LINK ? 6 : 4; /* ugh - fix me! */
	ace = (arp_cache_entry *)nhash_get(arphash, addr, alen);
	if (ace) {
		/* we already have it... */
		/* ??? should we update the timer here ?? */
		return 0;
	}
	ace = (arp_cache_entry *)pool_get(arpp);
	ace->ip_addr.sa_family = af;
 	ace->ip_addr.sa_len = alen;
	memcpy(&ace->ip_addr.sa_data, addr, alen);
	alen = lf == AF_LINK ? 6 : 4;
	ace->ll_addr.sa_family = lf;
	ace->ll_addr.sa_len = alen;
	memcpy(&ace->ll_addr.sa_data, link, alen);
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
	nhash_set(arphash, addr, ace->ip_addr.sa_len, (void*)ace);
	/* this is handy! */
	//walk_arp_cache();
	return 1;
}

int arp_input(struct mbuf *buf)
{
	ether_arp *arp = mtod(buf, ether_arp*);

	dump_arp(arp);
	
	switch (ntohs(arp->arp_op)) {
		case ARP_RQST:
			/* The RFC states we should send this back on the same interface it came 
			 * in on, so we don't mess with the interface here...
			 */  
			if (is_address_local((char*)&arp->target_ip, 4)) {
				struct sockaddr sa;
                                sa.sa_family = AF_LINK;
                                sa.sa_len = 6;
				insert_cache_entry((char*)&arp->sender, AF_LINK, &arp->sender_ip, AF_INET);
				//buf->m_pkthdr.rcvif = interface_for_address(&arp->target_ip, 4);
				memcpy(&arp->target_ip, &arp->sender_ip, 4);
				memcpy(&arp->target, &arp->sender, 6);
				memcpy(&arp->sender, &buf->m_pkthdr.rcvif->link_addr->sa_data, 6);
				memcpy(&arp->sender_ip, &sa.sa_data, 4);
				memcpy(&sa.sa_data, &arp->target, 6);
				arp->arp_op = htons(ARP_RPLY);
				buf->m_len = 28;
				if (buf->m_flags & M_PKTHDR)
					buf->m_pkthdr.len = 28;
				global_modules[prot_table[NS_ETHER]].mod->output(buf, NS_ARP, &sa);
				return 1;
			}
			/* not for us, just let it be discarded */
			break;
		case ARP_RPLY: 
			/* we only accept our own replies... */
			if (is_address_local(&arp->target_ip, 4))
				insert_cache_entry(&arp->sender, AF_LINK, &arp->sender_ip, AF_INET);
			/* we need to check the return value and find out if we have any waiters on the
			 * arp_q for this ip address
			 */
			break;
		default:
			printf("unknown ARP packet accepted (op_code was %d)\n", ntohs(arp->arp_op));
	}

	m_freem(buf);
	return 0;
}

static void arp_send_request(ifnet *ifa, struct sockaddr *from, 
				struct sockaddr *tgt)
{
	struct mbuf *buf = m_gethdr(MT_DATA);
	ether_arp *arp;
	struct sockaddr ether;

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
	arp->sender_ip = *(ipv4_addr*)&from->sa_data;
	arp->target_ip = *(ipv4_addr*)&tgt->sa_data;

	memset(&ether.sa_data, 0xff, 6);
	ether.sa_family = AF_LINK;
	ether.sa_len = 6;

	memcpy(&arp->sender, &ifa->link_addr->sa_data, 6);
	memset(&arp->target, 0, 6);

	buf->m_flags |= M_BCAST;
	buf->m_pkthdr.len = 28;
	buf->m_len = 28;
	buf->m_pkthdr.rcvif = ifa;

	global_modules[prot_table[NS_ETHER]].mod->output(buf, NS_ARP, &ether);
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
		aqe->ifn = interface_for_address(&src->sa_data, src->sa_len);

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
	NULL,
	&arp_input, 
	NULL,
	&arp_lookup
};

