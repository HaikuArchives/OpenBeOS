/* if.h
 * Interface definitions for beos
 */
#include <kernel/OS.h>

#ifndef OBOS_IF_H
#define OBOS_IF_H

#include <Drivers.h>
#include "sys/socket.h"
#include "net/if_types.h"
#include "netinet/in.h"
#include "net/route.h"
 
enum {
	IF_GETADDR = B_DEVICE_OP_CODES_END,
	IF_INIT,
	IF_NONBLOCK,
	IF_ADDMULTI,
	IF_REMMULTI,
	IF_SETPROMISC,
	IF_GETFRAMESIZE
};

/* Media types are now listed in net/if_types.h */

/* Interface flags */
enum {
	IFF_UP 		= 0x0001,
	IFF_DOWN	= 0x0002,
	IFF_PROMISC	= 0x0004,
	IFF_RUNNING	= 0x0008,
	IFF_MULTICAST	= 0x0010,
	IFF_BROADCAST	= 0x0020,
	IFF_POINTOPOINT = 0x0040,
	IFF_LOOPBACK	= 0x0080
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
	/* XXX - change this to ifa_next as it points to next ifaddr! */
        struct ifaddr 	*ifn_next;	/* the next address for the interface */
        struct ifnet 	*ifn;		/* pointer to the interface structure */

        struct sockaddr *ifa_addr;	/* the address - cast to be a suitable type, so we
					 * use this structure to store any type of address that
					 * we have a struct sockaddr_? for. e.g.
					 * link level address via sockaddr_dl and
					 * ipv4 via sockeddr_in
					 * same for next 2 pointers as well
					 */ 
	struct sockaddr	*ifa_dstaddr;	/* if we're on a point-to-point link this is
					 * the other end */
	struct sockaddr	*ifa_netmask;	/* The netmask we're using */
	uint8	ifa_flags;		/* flags (mainly routing */
	uint16	ifa_refcnt;		/* how many references are there to this structure? */
	int	ifa_metric;		/* the metirc for this sinterface/address */
};
#define ifa_broadaddr	ifa_dstaddr

struct if_data {
	uint8	ifi_type;		/* type of media */
	uint8	ifi_addrlen;		/* length of media address length */
	uint8	ifi_hdrlen;		/* size of media header */
	uint32	ifi_mtu;		/* mtu */
	uint32	ifi_metric;		/* routing metric */
	uint32	ifi_baudrate;		/* baudrate of line */
        /* statistics!! */
        int32  ifi_ipackets;           /* packets received on interface */
        int32  ifi_ierrors;            /* input errors on interface */
        int32  ifi_opackets;           /* packets sent on interface */
        int32  ifi_oerrors;            /* output errors on interface */
        int32  ifi_collisions;         /* collisions on csma interfaces */
        int32  ifi_ibytes;             /* total number of octets received */
        int32  ifi_obytes;             /* total number of octets sent */
        int32  ifi_imcasts;            /* packets received via multicast */
        int32  ifi_omcasts;            /* packets sent via multicast */
        int32  ifi_iqdrops;            /* dropped on input, this interface */
        int32  ifi_noproto;            /* destined for unsupported protocol */
};
	
struct ifnet {
	struct ifnet *next;		/* next device */
	ifaddr *if_addrlist;		/* linked list of addresses */
	int devid;			/* our device id if we have one... */
	int id;				/* id within the stack's device list */
	char *name;			/* name of driver e.g. tulip */
	int unit;			/* number of unit e.g  0 */
	char *if_name;			/* full name, e.g. tulip0 */
	struct if_data ifd;		/* if_data structure, shortcuts below */
	int flags;			/* if flags */
	
	ifq *rxq;
	thread_id rx_thread;
	ifq *txq;
	thread_id tx_thread;

	int	(*start) (struct ifnet *);
	int	(*stop)  (struct ifnet *);	
	int	(*input) (struct mbuf*, int len);
	int	(*output)(struct ifnet *, struct mbuf*, 
			  struct sockaddr*, struct rtentry *); 
	int	(*ioctl) (struct ifnet *, int, caddr_t);
};

#define if_mtu		ifd.ifi_mtu
#define if_type		ifd.ifi_type
#define if_addrlen	ifd.ifi_addrlen
#define if_hdrlen	ifd.ifi_hdrlen
#define if_metric	ifd.ifi_metric
#define if_baudrate	ifd.ifi_baurdate
#define if_ipackets     ifd.ifi_ipackets
#define if_ierrors      ifd.ifi_ierrors
#define if_opackets     ifd.ifi_opackets
#define if_oerrors      ifd.ifi_oerrors
#define if_collisions   ifd.ifi_collisions
#define if_ibytes       ifd.ifi_ibytes
#define if_obytes       ifd.ifi_obytes
#define if_imcasts      ifd.ifi_imcasts
#define if_omcasts      ifd.ifi_omcasts
#define if_iqdrops      ifd.ifi_iqdrops
#define if_noproto      ifd.ifi_noproto


/* link level sockaddr structure */
struct sockaddr_dl {
	uint8	sdl_len;      /* Total length of sockaddr */
	uint8	sdl_family;   /* AF_LINK */
	uint16	sdl_index;    /* if != 0, system given index for interface */
	uint8	sdl_type;     /* interface type */
	uint8	sdl_nlen;     /* interface name length, no trailing 0 reqd. */
	uint8	sdl_alen;     /* link level address length */
	uint8	sdl_slen;     /* link layer selector length */
	char	sdl_data[16]; /* minimum work area, can be larger;
                                   contains both if name and ll address */
};

/* Macro to get a pointer to the link level address */
#define LLADDR(s)	((caddr_t)((s)->sdl_data + (s)->sdl_nlen)

#define IFNAMSIZ	16

/* This structure is used for passing interface requests via ioctl */
struct ifreq {
	char	ifr_name[IFNAMSIZ];	/* name of interface */
	union {
		struct sockaddr_in ifru_addr;
		struct sockaddr_in ifru_dstaddr;
		struct sockaddr_in ifru_broadaddr;
		uint16 ifru_flags;
		int ifru_metric;
		caddr_t ifru_data;
	} ifr_ifru;
};
#define ifr_addr	ifr_ifru.ifru_addr
#define ifr_dstaddr	ifr_ifru.ifru_dstaddr
#define ifr_broadaddr	ifr_ifru.ifru_broadaddr
#define ifr_flags	ifr_ifru.ifru_flags
#define ifr_metric	ifr_ifru.ifru_metric
#define	ifr_data	ifr_ifru.ifru_data

struct ifconf {
	int ifc_len;	/* length of associated buffer */
	union {
		caddr_t ifcu_buf;
		struct ifreq *ifcu_req;
	} ifc_ifcu;
};
#define ifc_buf		ifc_ifcu.ifcu_buf
#define ifc_req		ifc_ifcu.ifcu_req


#define IFAFREE(ifa) \
do { \
        if ((ifa)->ifa_refcnt <= 0) \
                ifafree(ifa); \
        else \
                (ifa)->ifa_refcnt--; \
} while (0)

struct	ifnet *ifunit(char *name);
struct	ifaddr *ifa_ifwithaddr (struct sockaddr *);
struct	ifaddr *ifa_ifwithaf (int);
struct	ifaddr *ifa_ifwithdstaddr (struct sockaddr *);
struct	ifaddr *ifa_ifwithnet (struct sockaddr *);
struct	ifaddr *ifa_ifwithroute (int, struct sockaddr *,
                                        struct sockaddr *);
struct	ifaddr *ifaof_ifpforaddr (struct sockaddr *, struct ifnet *);
void	ifafree (struct ifaddr *);


#endif /* OBOS_IF_H */

