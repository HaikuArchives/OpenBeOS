/* arp.h
 * simple definitions for ARP
 */
 
#ifndef OBOS_ARP_H
#define OBOS_ARP_H

#include "mbuf.h"
#include "net_misc.h"

enum {
	ARP_RQST = 1,
	ARP_RPLY,
	RARP_RQST,
	RARP_RPLY
};

typedef struct arp_header {
	uint16	hard_type;
	uint16	prot;
	uint8	hard_size;
	uint8	prot_size;
	uint16	op;
	
	ether_addr	sender;
	ipv4_addr	sender_ip;
	
	ether_addr	target;
	ipv4_addr	target_ip;
} _PACKED arp_header;

int arp_input(struct mbuf *buf);

#endif /* OBOS_ARP_H */