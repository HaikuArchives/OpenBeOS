/* arp.h
 * simple definitions for ARP
 */
 
#ifndef OBOS_ARP_H
#define OBOS_ARP_H

#include "mbuf.h"
#include "net_misc.h"
#include "sys/socket.h"

/* arp codes */
enum {
	ARP_RQST = 1,
	ARP_RPLY,
	RARP_RQST,
	RARP_RPLY
};

/* split the arp header into 2 ytpes in case we have different hardware
 * types we want to use the cache!
 */
typedef struct arp_hdr {
        uint16  hard_type;
        uint16  prot;
        uint8   hard_size;
        uint8   prot_size;
        uint16  op;
} arp_hdr;

typedef struct ether_arp {
	arp_hdr		arp;
	ether_addr	sender;
	ipv4_addr	sender_ip;
	
	ether_addr	target;
	ipv4_addr	target_ip;
} _PACKED ether_arp;
#define arp_ht		arp.hard_type
#define arp_pro		arp.prot
#define arp_hsz		arp.hard_size
#define arp_psz		arp.prot_size
#define arp_op		arp.op

typedef struct arp_cache_entry	arp_cache_entry;
struct arp_cache_entry {
	arp_cache_entry *next;
	arp_cache_entry *prev;
	struct sockaddr	ip_addr; /* we use this so we can deal with other types of address */
	struct sockaddr	ll_addr; /* link-level address */
	int		status;
	bigtime_t	expires; /* when does this expire? */
};

typedef struct arp_q_entry	arp_q_entry;
struct arp_q_entry {
	arp_q_entry *next;
	struct sockaddr *src;
	struct sockaddr *tgt;
	ifnet *ifn;
	int attempts;
	bigtime_t lasttx;
};


#endif /* OBOS_ARP_H */

