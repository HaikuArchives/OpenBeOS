/* ipv4.h
 * definitions for ipv4 protocol
 */
 
#ifndef OBOS_IPV4_H
#define OBOS_IPV4_H

#include <ByteOrder.h>

#include "mbuf.h"
#include "net_misc.h"

enum {
	IP_ICMP = 1,
	IP_TCP = 6,
	IP_UDP = 17,
};

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
	ipv4_addr	src;
	ipv4_addr	dst;
} _PACKED ipv4_header;

#define IPV4_FLAGS(a)		(a)->flags_frag >> 13
#define IPV4_FRAG(a)		(a)->flags_frag & 0x1fff

int ipv4_input(struct mbuf* buf);

#endif /* OBOS_IPV4_H */
