/* net_misc.h
 * Miscellaneous networking stuff that doesn't yet have a home.
 */

#ifdef _NETWORK_STACK

#include <kernel/OS.h>
#include <ByteOrder.h>
#include <Errors.h>

#include "mbuf.h"

#ifndef OBOS_NET_MISC_H
#define OBOS_NET_MISC_H

#ifdef _KERNEL_MODE
#include <KernelExport.h>
#define printf  dprintf
#endif  

/* Not really sure if this is safe... */
#define EHOSTDOWN	(B_POSIX_ERROR_BASE + 45)


/* DEBUG Options!
 *
 * Having a single option is sort of lame so I'm going to start adding more here...
 *
 */
#define SHOW_DEBUG 	0	/* general debugging stuff (verbose!) */
#define SHOW_ROUTE	0	/* show routing information */
#define ARP_DEBUG	0	/* show ARP debug info */
#define USE_DEBUG_MALLOC   0   /* use the debug malloc code*/

typedef struct ifnet	ifnet;

/* ARP lookup return codes... */
enum {
	ARP_LOOKUP_OK 		= 1,
	ARP_LOOKUP_QUEUED 	= 2,
	ARP_LOOKUP_FAILED 	= 3
};

typedef	uint32	ipv4_addr;

/* These should be in KernelExport.h */
#define B_SELECT_READ        1
#define B_SELECT_WRITE       2
#define B_SELECT_EXCEPTION   3
//extern notify_select_event(selectsync *sync, uint32 ref);

/* XXX - add some macro's for inserting various types of address
 */

void net_server_add_device(ifnet *ifn);
uint16 in_cksum(struct mbuf *m, int len, int off);
void local_init(void);

/* sockets and in_pcb init */
int sockets_init(void);
void sockets_shutdown(void);
int inpcb_init(void);

void start_tx_thread(ifnet *dev);
void start_rx_thread(ifnet *dev);

/* Useful debugging functions */
void dump_ipv4_addr(char *msg, void *ad);
void print_ipv4_addr(void *ad);
void dump_ether_addr(char *msg, void *ma);
void print_ether_addr(void *ea);
void dump_buffer(char *buffer, int len);
void dump_sockaddr(void *ptr);

#endif /* OBOS_NET_MISC_H */

#endif /* _NETWORK_STACK */

