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
#include "net_timer.h"
#include "ethernet/ethernet.h"
#include "netinet/in_var.h"

loaded_net_module *global_modules;
int *prot_table;

/* arp cache */
static net_hash *arphash;
static arp_cache_entry *arpcache;
static pool_ctl *arpp;
static sem_id arpc_lock;

/* arp queue */
static sem_id arpq_lock;
static arp_q_entry *arp_lookup_q; 

/* these are in seconds... */
static int arp_prune = 300; 	/* time interval we prune the arp cache? 5 minutes */
static int arp_keep  = 1200;	/* length of time we keep entries... (20 mins) */
static int arp_noflood = 20;	/* seconds between arp flooding (20 secs) */
static int arp_maxtries = 5;    /* max tries before a pause */

/* stats */
static int32 arp_inuse = 0;	/* how many entries do we have? */
static int32 arp_allocated = 0;	/* how many arp entries have we created? */

static net_timer_id arpq_timer;
extern struct in_ifaddr *primary_addr;

#if ARP_DEBUG
void walk_arp_cache(void)
{
	arp_cache_entry *c = arpcache;
	int i = 1;

	printf( "Arp Cache\n"
		"=========\n\n"
		"Total allocations : %ld\n"
		"Current entries   : %ld\n\n",
		arp_allocated, arp_inuse);

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
#endif /* ARP_DEBUG */

/* Returns 1 is new cache entry added, 0 if not */
static int insert_cache_entry(void *link, int lf, void *addr, int af)
{
	arp_cache_entry *ace;
	int alen = 4; /* ugh - fix me! */

	ace = (arp_cache_entry *)nhash_get(arphash, addr, alen);
	if (ace) {
		/* if we don't match the existing record, update it,
		 * otherwsie we don't worry about it...
		 */
		if (memcmp(&ace->ll_addr.sa_data, link, 6) != 0) {
			/* oh, change of ll_addr... */
			memcpy(&ace->ll_addr.sa_data, link, alen);
			ace->expires = real_time_clock() + arp_keep;
		}	
		return 0;
	}
	ace = (arp_cache_entry *)pool_get(arpp);
	memset(ace, 0, sizeof(*ace));

	ace->ip_addr.sa_family = af;
 	ace->ip_addr.sa_len = sizeof(struct sockaddr_in);
	memcpy(&ace->ip_addr.sa_data, addr, alen);
	alen = lf == AF_LINK ? 6 : 4;
	ace->ll_addr.sa_family = lf;
	ace->ll_addr.sa_len = sizeof(struct sockaddr_dl);
	memcpy(&ace->ll_addr.sa_data, link, alen);
	ace->expires = real_time_clock() + arp_keep;

	acquire_sem(arpc_lock);
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
	release_sem(arpc_lock);

	/* these should be OK outside the sem locked area as we use atomic_add and
	 * we don't reference them in many places...
	 */
	atomic_add(&arp_inuse, 1);
	atomic_add(&arp_allocated, 1);

#if ARP_DEBUG
	walk_arp_cache();
#endif
	return 1;
}

int arp_input(struct mbuf *buf, int hdrlen)
{
	ether_arp *arp = mtod(buf, ether_arp*);
	int rv = -1;
	struct in_ifaddr *ia = primary_addr, *maybe_ia = NULL;
	struct in_addr my_addr;

#if ARP_DEBUG
	dump_arp(arp);
#endif
	/* find out if it's for us... */
	for (;ia ; ia = ia->ia_next)
		if (ia->ia_ifp == buf->m_pkthdr.rcvif) {
			maybe_ia = ia;
			if ((ia->ia_addr.sin_addr.s_addr == arp->target_ip.s_addr) ||
			    (ia->ia_addr.sin_addr.s_addr == arp->sender_ip.s_addr))
				break;
		}
	if (!maybe_ia)
		goto out;
	my_addr = ia ? ia->ia_addr.sin_addr : maybe_ia->ia_addr.sin_addr;

	switch (ntohs(arp->arp_op)) {
		case ARP_RQST:
			/* The RFC states we should send this back on the same interface it came 
			 * in on, so we don't mess with the interface here...
			 */  
#if ARP_DEBUG
			printf("ARP Request\n");
#endif
			if (arp->target_ip.s_addr == my_addr.s_addr) {
				struct sockaddr sa;
				/* paranoid... */
				if (!buf->m_pkthdr.rcvif)
					buf->m_pkthdr.rcvif = ia->ia_ifp;

                                sa.sa_family = AF_UNSPEC;
                                sa.sa_len = sizeof(sa);
#if ARP_DEBUG
				printf("Request was for one of our addresses!\n");
#endif
				insert_cache_entry((char*)&arp->sender, AF_LINK, &arp->sender_ip, AF_INET);
				arp->target_ip = arp->sender_ip;
				arp->target = arp->sender;
				memcpy(&arp->sender, &((struct ether_device*)buf->m_pkthdr.rcvif)->e_addr, 6);
				arp->sender_ip = my_addr;
				memcpy(&sa.sa_data, &arp->target, 6);
				arp->arp_op = htons(ARP_RPLY);
				buf->m_pkthdr.rcvif->output(buf->m_pkthdr.rcvif, buf, &sa, NULL);
				return 0;
			}
#if ARP_DEBUG
			else 
				printf("Not for us, ignoring...\n");
#endif
			/* not for us, just let it be discarded */
			break;
		case ARP_RPLY: 
			/* we only accept replies aimed at us, others we discard... */
#if ARP_DEBUG
			printf("ARP Reply\n");
#endif
			if (arp->target_ip.s_addr == my_addr.s_addr) {
				rv = insert_cache_entry(&arp->sender, AF_LINK, &arp->sender_ip, AF_INET);
				if (rv == 1) {
					arp_q_entry *aqe;
					struct sockaddr res;
					/* we've added an entry... */
					acquire_sem(arpq_lock);
					aqe = arp_lookup_q;
					while (aqe && aqe->status == ARP_WAITING) {
						if (((struct sockaddr_in *)&aqe->tgt)->sin_addr.s_addr 
						    == arp->sender_ip.s_addr) {
							/* woohoo - it's a match! */
							res.sa_family = AF_LINK;
							res.sa_len = 6;
							memcpy(&aqe->ptr, &arp->sender, 6);
							aqe->rt->rt_flags &= ~RTF_REJECT;
							aqe->callback(ARP_LOOKUP_OK, aqe->buf);
							/* Don't worry about removing it, just set the status
							 * and let the next pass of arpq_run remove it
							 * for us. Less overhead.
							 */
							aqe->status = ARP_COMPLETE;
						}
						aqe = aqe->next;
					}
					release_sem(arpq_lock);
				}
			} else if (arp->sender_ip.s_addr == my_addr.s_addr) {
				printf("Duplicate IP address detected!\n");
				/* XXX - what now? */
			}	
		
			break;
		default:
			printf("unknown ARP packet accepted (op_code was %d)\n", ntohs(arp->arp_op));
	}


out:
	m_freem(buf);
	return 0;
}

#define satosin(sa)     ((struct sockaddr_in *)(sa))
static void arp_send_request(arp_q_entry *aqe)
{
	struct mbuf *buf = m_gethdr(MT_DATA);
	struct ethernetarp *arppkt = mtod(buf, struct ethernetarp *);
	struct sockaddr_in *sin = (struct sockaddr_in*)&aqe->tgt;

#if ARP_DEBUG
	dump_ipv4_addr("arp_send_request: target ", &sin->sin_addr);
#endif

	/* if we have no interface structure, how do we send data??? */
	if (!aqe->rt->rt_ifp)
		return;

	if (sin->sin_family != AF_INET) {
		m_freem(buf);
		return;
	}

	arppkt->arp.arp_ht = htons(1);
	arppkt->arp.arp_hsz = 6;
	arppkt->arp.arp_pro = htons(ETHER_IPV4);
	arppkt->arp.arp_psz = 4;
	arppkt->arp.arp_op = htons(ARP_RQST);

	arppkt->arp.sender_ip = satosin(aqe->rt->rt_ifa->ifa_addr)->sin_addr;
	arppkt->arp.target_ip = sin->sin_addr;

	/* we send to the broadcast address, ff:ff:ff:ff:ff:ff */
	memset(&arppkt->eth.dest, 0xff, 6);
	memcpy(&arppkt->eth.src, &((struct ether_device*)aqe->rt->rt_ifp)->e_addr, 6);
	arppkt->eth.type = htons(ETHER_ARP);

	/* send request on same interface we'll send packet */
	buf->m_pkthdr.rcvif = aqe->rt->rt_ifp;
	buf->m_flags |= M_BCAST;
	/* Now we've decided on the interface, add the link level address we'll be
	 * advertising and set the ip address we want to use
	 */
	memcpy(&arppkt->arp.sender, &((struct ether_device*)aqe->rt->rt_ifp)->e_addr, 6);
	memset(&arppkt->arp.target, 0, 6);

	/* setup buf details... */
	buf->m_flags |= M_BCAST;
	buf->m_pkthdr.len = buf->m_len = sizeof(struct ethernetarp);

	/* update the queue details... */
	aqe->status = ARP_WAITING;
	aqe->lasttx = real_time_clock();
	aqe->attempts++;

	aqe->rt->rt_flags |= RTF_REJECT; /* while we're waiting on a reply... */
	IFQ_ENQUEUE(aqe->rt->rt_ifp->txq, buf);
}

static void arp_cleanse(void *data)
{
	arp_cache_entry *ace = arpcache;
	arp_cache_entry *temp;

	acquire_sem(arpc_lock);
	while (ace) {
		if (ace->expires <= real_time_clock()) {
			/* we've expired... */
			/* XXX - can this be tidied up??? */
			if (ace->next != ace) {
				ace->prev->next = ace->next;
				ace->next->prev = ace->prev;
			} else {
				/* we're probably the only entry... */
				ace->next = NULL;
			}
			if (ace == arpcache)
				arpcache = ace->next;

			temp = ace;
			ace = ace->next;

			/* remove from hash table */
			nhash_set(arphash, &temp->ip_addr.sa_data, 
				temp->ip_addr.sa_len, NULL);
			pool_put(arpp, ace);
			atomic_add(&arp_inuse, -1);
			continue;
		}
		ace = ace->next;
		if (ace == arpcache)
			break;
	}
	release_sem(arpc_lock);

#if ARP_DEBUG
	printf("ARP: cache has been cleaned\n");
	walk_arp_cache();
#endif
}

static void arpq_run(void *data)
{
	arp_q_entry *aqe = arp_lookup_q;
	arp_q_entry *temp;

	/* if the q is empty, don't bother locking etc, just return */
	if (!arp_lookup_q)
		return;

	acquire_sem(arpq_lock);
	while (aqe) {
		if (aqe->status == ARP_COMPLETE) {
			temp = aqe;
			aqe = aqe->next;
			if (temp == arp_lookup_q)
				arp_lookup_q = aqe;
			free(temp);
			continue;
		}
		/* OK, so we're not yet done...*/
		/* is it time to send again? */
		if (aqe->lasttx < real_time_clock() - arp_noflood) {
			if (aqe->attempts++ > arp_maxtries) {
				/* No! we've run out of tries. */
				aqe->callback(ARP_LOOKUP_FAILED, aqe->buf);
				temp = aqe;
				aqe = aqe->next;
				if (temp == arp_lookup_q)
					arp_lookup_q = aqe;
				free(temp);
				continue;
			}
			/* yes, send another request */
			arp_send_request(aqe);
		}
		aqe = aqe->next;

	}

	if (!arp_lookup_q)
		net_remove_timer(arpq_timer);

	release_sem(arpq_lock);

#if ARP_DEBUG
	printf("ARP: lookup queue was run (%ld)\n", arpq_timer);
#endif
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
	arp_lookup_q = NULL;
	arpq_lock = create_sem(1, "arp_q_lock");
	arpc_lock = create_sem(1, "arp cache lock");

	/* now, start the cleanser... */
	net_add_timer(&arp_cleanse, NULL, arp_prune * USECS_PER_SEC);

	return 0;
}

static int arp_resolve(struct mbuf *buf, struct rtentry *rt, struct sockaddr *tgt, 
		       void *dptr, void (*callback)(int, struct mbuf *))
{
	arp_cache_entry *ace = NULL;
	arp_q_entry *aqe;
	struct sockaddr_in *sin = (struct sockaddr_in *)tgt;

#if ARP_DEBUG
	dump_ipv4_addr("arp_resolve: ", &sin->sin_addr);
#endif

	ace = (arp_cache_entry *)nhash_get(arphash, &sin->sin_addr, 4);

	if (ace) {
		ace->expires = real_time_clock() + arp_keep;
		memcpy(dptr, ace->ll_addr.sa_data, 6);
		return ARP_LOOKUP_OK;
	}

#if ARP_DEBUG
	printf("Queueing ARP request\n");
#endif

	/* ok, we didn't get one! */
	aqe = arp_lookup_q;
	while (aqe) {
		if ((satosin(&aqe->tgt))->sin_addr.s_addr == sin->sin_addr.s_addr) 
			/* XXX - need to add code to add this packet to
			 * the list of waiting packets, or discard and reject...
			 */
			break;
		aqe = aqe->next;
	}

	if (!aqe) {
		aqe = malloc(sizeof(struct arp_q_entry));
		aqe->buf = buf;
		aqe->tgt = *tgt;
		aqe->ptr = dptr;
		aqe->rt = rt;
		aqe->callback = callback;
		if (aqe->buf->m_pkthdr.rcvif == NULL)
			aqe->buf->m_pkthdr.rcvif = rt->rt_ifp;

		if (arp_lookup_q)
			aqe->next = arp_lookup_q;
		else
			aqe->next = NULL;

		arp_lookup_q = aqe;
		arp_send_request(aqe);
	}

	arpq_timer = net_add_timer(&arpq_run, NULL, arp_noflood * USECS_PER_SEC);

	return ARP_LOOKUP_QUEUED;
}

net_module net_module_data = {
	"ARP module",
	NS_ARP,
	NET_LAYER1,
	0, 	/* users can't create sockets in this module! */
	0,
	0,

	&arp_init,
	NULL,
	&arp_input, 
	NULL,
	&arp_resolve,
	NULL
};

