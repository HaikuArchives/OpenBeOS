/* in.h */

#include <ByteOrder.h> /* for htonl */

#include "net_misc.h"

#ifndef _NETINET_IN_H_
#define _NETINET_IN_H_

/* Protocol definitions - add to as required... */
enum {
	IPPROTO_IP	= 0,	/* IPv4 */
	IPPROTO_ICMP	= 1,	/* ICMP (v4) */
	IPPROTO_IGMP	= 2,	/* IGMP (group management) */
	IPPROTO_TCP	= 6,	/* tcp */
	IPPROTO_UDP	= 17,	/* UDP */
        IPPROTO_IPV6    = 41,   /* IPv6 in IPv6 */
	IPPROTO_ROUTING	= 43,	/* Routing */
	IPPROTO_ICMPV6	= 58,	/* IPv6 ICMP */
	IPPROTO_ETHERIP	= 97,	/* Ethernet in IPv4 */
	IPPROTO_RAW	= 255
};

#define IPPROTO_MAX	256

/* Port numbers...
 * < IPPORT_RESERVED are privileged and should be
 * accessible only by root
 * > IPPORT_USERRESERVED are reserved for servers, though
 * not requiring privileged status
 */

#define IPPORT_RESERVED         1024
#define IPPORT_USERRESERVED     49151

/* This is an IPv4 address structure. Why is it a structure?
 * Historical reasons.
 */ 
struct in_addr {
	in_addr_t s_addr;
};

/*
 * IP Version 4 socket address.
 */
struct sockaddr_in {
        uint8		sin_len;
        uint8		sin_family;
        uint16		sin_port;
        struct		in_addr sin_addr;
        int8		sin_zero[8];
};
/* the address is therefore at sin_addr.s_addr */

/* Hmm, need to think about this byte ordering. So far I've
 * tried to keep everything in the stack in network order
 * but that won't be possible once we move to userland,
 * so we'll probably need to have a define that tells
 * these headers we're building in/out of the stack code...
 */
 
#define __IPADDR(x)     ((uint32) htonl((uint32)(x)))

#define INADDR_ANY              __IPADDR(0x00000000)
#define INADDR_LOOPBACK         __IPADDR(0x7f000001)


/* some helpful macro's :) */
#define in_hosteq(s,t)  ((s).s_addr == (t).s_addr)
#define in_nullhost(x)  ((x).s_addr == INADDR_ANY)

#define satosin(sa)     ((struct sockaddr_in *)(sa))
#define sintosa(sin)    ((struct sockaddr *)(sin))

#endif /* NETINET_IN_H */
