/* arp.h
 * simple definitions for ARP
 */
 
#ifndef OBOS_ARP_H
#define OBOS_ARP_H

#include "mbuf.h"
#include "net_misc.h"
#include "sys/socket.h"
#include "ethernet/ethernet.h"

/* arp codes */
enum {
	ARP_RQST = 1,
	ARP_RPLY,
	RARP_RQST,
	RARP_RPLY
};

/* These are used by the arp lookup queue structures */
enum {
	ARP_WAITING,
	ARP_COMPLETE
};

#define USECS_PER_SEC		1000000

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
	struct in_addr	sender_ip;
	
	ether_addr	target;
	struct in_addr	target_ip;
} _PACKED ether_arp;
#define arp_ht		arp.hard_type
#define arp_pro		arp.prot
#define arp_hsz		arp.hard_size
#define arp_psz		arp.prot_size
#define arp_op		arp.op

struct ethernetarp {
	struct ethernet_header	eth;
	struct ether_arp	arp;
};


typedef struct arp_cache_entry	arp_cache_entry;
struct arp_cache_entry {
	arp_cache_entry *next;
	arp_cache_entry *prev;
	struct sockaddr	ip_addr; /* we use this so we can deal with other types of address */
	struct sockaddr	ll_addr; /* link-level address */
	int		status;
	uint32		expires; /* when does this expire? */
};

typedef struct arp_q_entry	arp_q_entry;
struct arp_q_entry {
	arp_q_entry 	*next;
	struct mbuf 	*buf;
	struct rtentry 	*rt;
	struct sockaddr tgt;
	int 		attempts;
	uint32		lasttx;
	int 		status;
	void 		(*callback)(int, struct mbuf *);
	void 		*ptr;
};


#endif /* OBOS_ARP_H */

