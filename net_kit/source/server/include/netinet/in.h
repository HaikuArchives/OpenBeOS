/* in.h */

#include <ByteOrder.h> /* for htonl */

#include "net_misc.h"

#ifndef _NETINET_IN_H_
#define _NETINET_IN_H_

/* XXX - This really doesn't belong in here... */
typedef uint32	in_addr_t; 

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
        struct in_addr 	sin_addr;
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
#define INADDR_NONE             __IPADDR(0xffffffff)

#define IN_CLASSA(i)            (((uint32)(i) & __IPADDR(0x80000000)) == \
                                 __IPADDR(0x00000000))
#define IN_CLASSA_NET           __IPADDR(0xff000000)
#define IN_CLASSA_NSHIFT        24
#define IN_CLASSA_HOST          __IPADDR(0x00ffffff)
#define IN_CLASSA_MAX           128

#define IN_CLASSB(i)            (((uint32)(i) & __IPADDR(0xc0000000)) == \
                                 __IPADDR(0x80000000))
#define IN_CLASSB_NET           __IPADDR(0xffff0000)
#define IN_CLASSB_NSHIFT        16
#define IN_CLASSB_HOST          __IPADDR(0x0000ffff)
#define IN_CLASSB_MAX           65536

#define IN_CLASSC(i)            (((uint32)(i) & __IPADDR(0xe0000000)) == \
                                 __IPADDR(0xc0000000))
#define IN_CLASSC_NET           __IPADDR(0xffffff00)
#define IN_CLASSC_NSHIFT        8
#define IN_CLASSC_HOST          __IPADDR(0x000000ff)

#define IN_CLASSD(i)            (((uint32)(i) & __IPADDR(0xf0000000)) == \
                                 __IPADDR(0xe0000000))
/* These ones aren't really net and host fields, but routing needn't know. */
#define IN_CLASSD_NET           __IPADDR(0xf0000000)
#define IN_CLASSD_NSHIFT        28
#define IN_CLASSD_HOST          __IPADDR(0x0fffffff)

#define IN_MULTICAST(i)		IN_CLASSD(i)

/* some helpful macro's :) */
#define in_hosteq(s,t)  ((s).s_addr == (t).s_addr)
#define in_nullhost(x)  ((x).s_addr == INADDR_ANY)

#define satosin(sa)     ((struct sockaddr_in *)(sa))
#define sintosa(sin)    ((struct sockaddr *)(sin))

#ifdef _NETWORK_STACK
int in_control(struct socket *so, int cmd, caddr_t data, struct ifnet *ifp);
#endif

#endif /* NETINET_IN_H */
