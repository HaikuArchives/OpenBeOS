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
#define	SHOW_MALLOC_USAGE	0	/* show instances of malloc */

typedef struct ifnet	ifnet;

/* ARP lookup return codes... */
enum {
	ARP_LOOKUP_OK 		= 1,
	ARP_LOOKUP_QUEUED 	= 2,
	ARP_LOOKUP_FAILED 	= 3
};

/* structure for Ethernet MAC address xx:xx:xx:xx:xx:xx */
typedef struct ether_addr {
        uint8 addr[6];
} ether_addr;

typedef	uint32	ipv4_addr;

/* XXX - add some macro's for inserting various types of address
 */

void net_server_add_device(ifnet *ifn);
uint16 in_cksum(struct mbuf *m, int len, int off);
void local_init(void);
void insert_local_address(struct sockaddr *sa, ifnet *dev);
int is_address_local(void *ptr, int len);
ifnet *interface_for_address(void *data, int len);

/* sockets and in_pcb init */
int sockets_init(void);
void sockets_shutdown(void);
int inpcb_init(void);

void *protocol_address(ifnet *ifa, int family);
#define paddr(if, fam, t)	((t)(protocol_address(if, fam)))

int compare_sockaddr(struct sockaddr *a, struct sockaddr *b);

/* Useful debugging functions */
void dump_ipv4_addr(char *msg, void *ad);
void print_ipv4_addr(void *ad);
void dump_ether_addr(char *msg, void *ma);
void print_ether_addr(void *ea);
void dump_buffer(char *buffer, int len);
void dump_sockaddr(void *ptr);

struct sock_fd {
        struct sock_fd *next;
        struct sock_fd *prev;
        int fd;
        struct socket *so;
};
struct sock_fd *make_sock_fd(void);
void release_sock_fd(struct sock_fd *sfd);

#ifdef USE_DEBUG_MALLOC
void *net_malloc(int size);
void free(void *ptr);
#endif

#endif /* OBOS_NET_MISC_H */

#endif /* _NETWORK_STACK */

