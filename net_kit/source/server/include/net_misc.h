/* net_misc.h
 * Miscellaneous networking stuff that doesn't yet have a home.
 */

#include <kernel/OS.h>
#include <ByteOrder.h>

#include "mbuf.h"

#ifndef OBOS_NET_MISC_H
#define OBOS_NET_MISC_H

/* This idea taken from newos. Type independant address storage
 */

typedef struct ifnet	ifnet;

/* structure for Ethernet MAC address xx:xx:xx:xx:xx:xx */
typedef struct ether_addr {
        uint8 addr[6];
} ether_addr;

typedef struct sockaddr {
	uint8	type;
	uint8	len;
	uint8	addr[30];
} sockaddr;

typedef	uint32	ipv4_addr;

/* XXX - add some macro's for inserting various types of address
 */

enum {
	AF_LINK		= 0, /* ethernet address */
	AF_INET		= 1, /* IPv4 address */
	AF_INET6	= 2, /* IPv6 address */
};


void net_server_add_device(ifnet *ifn);
int in_cksum(struct mbuf *m, int len);

/* Useful debugging functions */
void dump_ipv4_addr(char *msg, ipv4_addr *ad);
void print_ipv4_addr(ipv4_addr *ad);
void dump_ether_addr(char *msg, ether_addr *ma);
void print_ether_addr(ether_addr *ea);
void dump_buffer(char *buffer, int len);

#endif /* OBOS_NET_MISC_H */
