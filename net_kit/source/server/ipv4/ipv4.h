/* ipv4.h
 * definitions for ipv4 protocol
 */
 
#ifndef OBOS_IPV4_H
#define OBOS_IPV4_H

#include <ByteOrder.h>

#include "mbuf.h"
#include "net_misc.h"
#include "netinet/in.h"

typedef struct ipv4_header {
#if B_HOST_IS_BENDIAN
	uint8	ver:4;
	uint8	hl:4;
#else
	uint8	hl:4;
	uint8	ver:4;
#endif
	uint8	tos;
	uint16	length;
	uint16	id;
	uint16	flags_frag;
	uint8	ttl;
	uint8	prot;
	uint16	hdr_cksum;
	struct in_addr	src;
	struct in_addr 	dst;
} _PACKED ipv4_header;

#define IPV4_FLAGS(a)		(a)->flags_frag >> 13
#define IPV4_FRAG(a)		(a)->flags_frag & 0x1fff



#define IP_FORWARDING           0x1             /* most of ip header exists */
#define IP_RAWOUTPUT            0x2             /* raw ip header exists */
#define IP_ROUTETOIF            SO_DONTROUTE    /* bypass routing tables */
#define IP_ALLOWBROADCAST       SO_BROADCAST    /* can send broadcast packets */
#define IP_MTUDISC              0x0400          /* pmtu discovery, set DF */


#endif /* OBOS_IPV4_H */
