/* if.h
 * Interface definitions for beos
 */

#include <Drivers.h>
#include "../include/sys/socket.h"
 
#ifndef OBOS_IF_H
#define OBOS_IF_H

enum {
	IF_GETADDR = B_DEVICE_OP_CODES_END,
	IF_INIT,
	IF_NONBLOCK,
	IF_ADDMULTI,
	IF_REMMULTI,
	IF_SETPROMISC,
	IF_GETFRAMESIZE
};

enum {
	IFD_ETHERNET = 1,
	IFD_LOOPBACK
};

/* Interface flags */
enum {
	IFF_UP 		= 0x0001,
	IFF_DOWN	= 0x0002,
	IFF_PROMISC	= 0x0004,
	IFF_RUNNING	= 0x0008,
	IFF_MULTICAST	= 0x0010,
	IFF_BROADCAST	= 0x0020
};

typedef struct ifq	ifq;
typedef struct ifaddr   ifaddr;

struct ifq {
	struct mbuf *head;
	struct mbuf *tail;
	int maxlen;
	int len;
	sem_id lock;
	sem_id pop;
};

/* function declaration */
ifq *start_ifq(void);

#define IFQ_ENQUEUE(ifq, m) { \
	acquire_sem((ifq)->lock); \
        (m)->m_nextpkt = 0; \
        if ((ifq)->tail == 0) \
                (ifq)->head = m; \
        else \
                (ifq)->tail->m_nextpkt = m; \
        (ifq)->tail = m; \
        (ifq)->len++; \
	release_sem((ifq)->lock); \
	release_sem((ifq)->pop); \
}

#define IFQ_DEQUEUE(ifq, m) { \
	acquire_sem((ifq)->lock); \
        (m) = (ifq)->head; \
        if (m) { \
                if (((ifq)->head = (m)->m_nextpkt) == 0) \
                        (ifq)->tail = 0; \
                (m)->m_nextpkt = 0; \
                (ifq)->len--; \
        } \
	release_sem((ifq)->lock); \
}

struct ifaddr {
        struct ifaddr *next;
        struct ifnet *ifn;

        struct sockaddr if_addr;
};

struct ifnet {
	struct ifnet *next;		/* next device */
	ifaddr *if_addrlist;	/* linked list of addresses */
	int dev;		/* device handle */
	int id;			/* id within the stack's device list */
	char *name;		/* name of driver e.g. tulip*/
	int unit;		/* number of unit e.g  0*/
	int type;		/* what type of interface are we? */
	struct sockaddr *link_addr;	/* pointer to sockaddr structure with link address */
	int flags;		/* flags */
	int mtu;		/* mtu */

	thread_id if_thread;
	ifq *rxq;
	thread_id rx_thread;
	ifq *txq;
	thread_id tx_thread;

	int	(*input)(struct mbuf*);
	int	(*output)(struct mbuf*, int, struct sockaddr*); 
};

struct sockaddr *in_ifaddr;	/* this will point to our primary address */
 
#endif /* OBOS_IF_H */

