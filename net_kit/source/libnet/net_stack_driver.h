/* net_stack_driver.h
 * structures and defines to deal with the network stack pseudo-driver...
 */

#ifndef NET_STACK_DRIVER_H
#define NET_STACK_DRIVER_H

#define NET_STACK_DRIVER_PATH  "net/stack"

enum {
	// Paranoia mode: be far away of B_DEVICE_OP_CODES_END opcodes!!!
	// You never know what another device driver ioctl() will do
	// if think our NET_STACK_* is in fact his DO_RISKY_BUSINESS opcode, or whatever...
 	NET_IOCTL_BASE = 0xbe230000,			
	NET_STACK_IOCTL_BASE = NET_IOCTL_BASE + 0x200,
};

enum {
	NET_STACK_SOCKET = NET_STACK_IOCTL_BASE,	// socket_args *
	NET_STACK_BIND,								// sockaddr_args *
	NET_STACK_RECVFROM,							// data_xfer_args *
	NET_STACK_RECV,								// data_xfer_args *
	NET_STACK_SENDTO,							// data_xfer_args *
	NET_STACK_SEND,								// data_xfer_args *
	NET_STACK_LISTEN,							// int_args * (value = backlog)
	NET_STACK_ACCEPT,							// accept_args *
	NET_STACK_CONNECT,							// sockaddr_args *
	NET_STACK_SHUTDOWN,							// int_args * (value = how)
	NET_STACK_GETSOCKOPT,						// sockopt_args *
	NET_STACK_SETSOCKOPT,						// sockopt_args *
	NET_STACK_GETSOCKNAME,						// sockaddr_args *
	NET_STACK_GETPEERNAME,						// sockaddr_args *

	NET_STACK_SYSCTL,							// sysctl_args *
	NET_STACK_SELECT,							// select_args *
	NET_STACK_GET_COOKIE,						// void **

	NET_STACK_IOCTL_MAX
};

struct int_args {	// used by NET_STACK_LISTEN/_SHUTDOWN 
	int value;
};

struct sockaddr_args {	// used by NET_STACK_CONNECT/_BIND/_GETSOCKNAME/_GETPEERNAME
	struct sockaddr * addr;
	int addrlen;
};

struct sockopt_args {	// used by NET_STACK_SETSOCKOPT/_GETSOCKOPT
	int   	level;
	int   	option;
	void  *	optval;
	int   	optlen;
};

struct data_xfer_args {	// used by NET_STACK_SEND/_SENDTO/_RECV/_RECVFROM
	void * data;
	size_t datalen;
	int flags;
	struct sockaddr * addr;	// unused in *_SEND and *_RECV cases
	int addrlen;			// unused in *_SEND and *_RECV cases
};

struct socket_args {	// used by NET_STACK_SOCKET
	int family;
	int type;
	int proto;
};

struct accept_args {	// used by NET_STACK_ACCEPT
	void * cookie;
	struct sockaddr * addr;
	int addrlen;
};

struct select_args {	// used by NET_STACK_SELECT
	int nbits;
	struct fd_set * rbits;
	struct fd_set * wbits;
	struct fd_set * ebits;
	struct timeval * timeout;
};

struct sysctl_args {	// used by NET_STACK_SYSCTL
	int * name;
	uint namelen;
	void * oldp;
	size_t * oldlenp;
	void * newp;
	size_t newlen;
};

#endif /* NET_STACK_DRIVER_H */
