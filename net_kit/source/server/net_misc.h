/* net_misc.h
 * Miscellaneous networking stuff that doesn't yet have a home.
 */

#ifndef OBOS_NET_MISC_H
#define OBOS_NET_MISC_H

/* This idea taken from newos. Type independant address storage
 */

/* structure for Ethernet MAC address xx:xx:xx:xx:xx:xx */
typedef struct ether_addr {
        uint8 eth_addr[6];
} ether_addr;

typedef struct netaddr {
	uint8	type;
	uint8	len;
	uint8 addr[30];
} netaddr;

/* XXX - add soem macro's for inserting various types of address
 */

enum {
	ADDR_TYPE_ETHERNET	= 0, /* ethernet address */
	ADDR_TYPE_IPV4		= 1, /* IPv4 address */
	ADDR_TYPE_IPV6		= 2, /* IPv6 address */
};

#endif /* OBOS_NET_MISC_H */
