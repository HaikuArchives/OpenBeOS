/* sys/socket.h */

#ifndef OBOS_SYS_SOCKET_H
#define OBOS_SYS_SOCKET_H

/* These are the address/protocol families we'll be using... */
/* NB these should be added to as required... */

#define	AF_UNSPEC		0
#define AF_INET			2
#define AF_ROUTE		3
#define AF_LINK			18
#define AF_IPX			23
#define AF_INET6		24

#define AF_MAX			24

#define PF_UNSPEC		AF_UNSPEC
#define PF_INET			AF_INET
#define PF_ROUTE		AF_ROUTE
#define PF_LINK			AF_LINK
#define PF_INET6		AF_INET6	
#define PF_IPX			AF_IPX

/* Types of socket we can create (eventually) */
#define SOCK_STREAM 	1
#define SOCK_DGRAM	2
#define SOCK_RAW	3
#define SOCK_MISC	255

/*
 * Option flags per-socket.
 */
#define SO_DEBUG        0x0001          /* turn on debugging info recording */
#define SO_ACCEPTCONN   0x0002          /* socket has had listen() */
#define SO_REUSEADDR    0x0004          /* allow local address reuse */
#define SO_KEEPALIVE    0x0008          /* keep connections alive */
#define SO_DONTROUTE    0x0010          /* just use interface addresses */
#define SO_BROADCAST    0x0020          /* permit sending of broadcast msgs */
#define SO_USELOOPBACK  0x0040          /* bypass hardware when possible */
#define SO_LINGER       0x0080          /* linger on close if data present */
#define SO_OOBINLINE    0x0100          /* leave received OOB data in line */
#define SO_REUSEPORT    0x0200          /* allow local address & port reuse */

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF       0x1001          /* send buffer size */
#define SO_RCVBUF       0x1002          /* receive buffer size */
#define SO_SNDLOWAT     0x1003          /* send low-water mark */
#define SO_RCVLOWAT     0x1004          /* receive low-water mark */
#define SO_SNDTIMEO     0x1005          /* send timeout */
#define SO_RCVTIMEO     0x1006          /* receive timeout */
#define SO_ERROR        0x1007          /* get error status and clear */
#define SO_TYPE         0x1008          /* get socket type */
#define SO_NETPROC      0x1020          /* multiplex; network processing */

struct sockaddr {
	uint8	sa_len;
	uint8	sa_family;
	uint8	sa_data[30];
};

				/* Max listen queue for a socket */
#define SOMAXCONN	5	/* defined as 128 in OpenBSD */

struct msghdr {
	caddr_t	msg_name;	/* address we're using (optional) */
	uint msg_namelen;	/* length of address */
	struct iovec *msg_iov;	/* scatter/gather array we'll use */
	uint msg_iovlen;	/* # elements in msg_iov */
	caddr_t msg_control;	/* extra data */
	uint msg_controllen;	/* length of extra data */
	int msg_flags;		/* flags */
};

/* Defines used in msghdr structure. */
#define MSG_OOB         0x1             /* process out-of-band data */
#define MSG_PEEK        0x2             /* peek at incoming message */
#define MSG_DONTROUTE   0x4             /* send without using routing tables */
#define MSG_EOR         0x8             /* data completes record */
#define MSG_TRUNC       0x10            /* data discarded before delivery */
#define MSG_CTRUNC      0x20            /* control data lost before delivery */
#define MSG_WAITALL     0x40            /* wait for full request or error */
#define MSG_DONTWAIT    0x80            /* this message should be nonblocking */
#define MSG_BCAST       0x100           /* this message rec'd as broadcast */
#define MSG_MCAST       0x200           /* this message rec'd as multicast */

struct cmsghdr {
	uint	cmsg_len;
	int	cmsg_level;
	int	cmsg_type;
	/* there now follows uchar[] cmsg_data */
};

/* Function declarations */
int     socket (int, int, int);
int     closesocket(int);
int     bind(int, const struct sockaddr *, int);
int     sendto(int, caddr_t, size_t, int, const struct sockaddr*, size_t);
int     recvfrom(int, caddr_t, size_t, int, struct sockaddr *, size_t);

#endif /* OBOS_SYS_SOCKET_H */

