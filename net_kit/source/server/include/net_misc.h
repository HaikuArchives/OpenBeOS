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

typedef	uint32	ipv4_addr;

/* XXX - add some macro's for inserting various types of address
 */

void net_server_add_device(ifnet *ifn);
int in_cksum(struct mbuf *m, int len);
void local_init(void);
void insert_local_address(struct sockaddr *sa, ifnet *dev);
int is_address_local(void *ptr, int len);
ifnet *interface_for_address(void *data, int len);
int compare_sockaddr(struct sockaddr *a, struct sockaddr *b);

/* Useful debugging functions */
void dump_ipv4_addr(char *msg, void *ad);
void print_ipv4_addr(void *ad);
void dump_ether_addr(char *msg, void *ma);
void print_ether_addr(void *ea);
void dump_buffer(char *buffer, int len);

#endif /* OBOS_NET_MISC_H */
