/* net_misc.h
 * Miscellaneous networking stuff that doesn't yet have a home.
 */

#ifndef OBOS_NET_MISC_H
#define OBOS_NET_MISC_H

#include <kernel/OS.h>

/* This idea taken from newos. Type independant address storage
 */

/* structure for Ethernet MAC address xx:xx:xx:xx:xx:xx */
typedef struct ether_addr {
        uint8 addr[6];
} ether_addr;

typedef struct netaddr {
	uint8	type;
	uint8	len;
	uint8 addr[30];
} netaddr;

/* XXX - add some macro's for inserting various types of address
 */

enum {
	ADDR_TYPE_ETHERNET	= 0, /* ethernet address */
	ADDR_TYPE_IPV4		= 1, /* IPv4 address */
	ADDR_TYPE_IPV6		= 2, /* IPv6 address */
};


/* NB xxx - Fix these for different types of endianness... */
#define ntohs(n) \
	((((uint16)(n) & 0xff) << 8) | ((uint16)(n) >> 8))
#define htons(h) \
	((((uint16)(h) & 0xff) << 8) | ((uint16)(h) >> 8))
#define ntohl(n) \
	(((uint32)(n) << 24) | (((uint32)(n) & 0xff00) << 8) |(((uint32)(n) & 0x00ff0000) >> 8) | ((uint32)(n) >> 24))
#define htonl(n) \
	(((uint32)(n) << 24) | (((uint32)(n) & 0xff00) << 8) |(((uint32)(n) & 0x00ff0000) >> 8) | ((uint32)(n) >> 24))

#endif /* OBOS_NET_MISC_H */
