/* sys/socket.h */

#ifndef OBOS_SYS_SOCKET_H
#define OBOS_SYS_SOCKET_H

/* These are the address/protocol families we'll be using... */
/* NB these should be added to as required... */

#define	AF_UNSPEC		0
#define AF_INET			1
#define AF_ROUTE		3
#define AF_LINK			4
#define AF_INET6		4
#define AF_IPX			7

#define AF_MAX			9

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


struct sockaddr {
	uint8	sa_len;
	uint8	sa_family;
	uint	sa_data[30];
};

#endif /* OBOS_SYS_SOCKET_H */

