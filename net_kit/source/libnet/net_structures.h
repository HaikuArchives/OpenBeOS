/* net_structures.h
 * structures and defines to deal with the device driver...
 */

#ifndef NET_STRCUTURES_H
#define NET_STRCUTURES_H

#include <Drivers.h>
#include "sys/select.h"
#include <sys/time.h>

struct socket_args {
	int	rv;
	int dom;
	int type;
	int prot;
};

struct bind_args {
	int	rv;
	caddr_t data;
	int dlen;
};

struct listen_args {
	int rv;
	int backlog;
};

struct connect_args {
	int rv;
	caddr_t name;
	int namelen;
};

struct select_args {
	int rv;
	int mfd;
	struct fd_set *rbits;
	struct fd_set *wbits;
	struct fd_set *ebits;
	struct timeval *tv;
};

struct shutdown_args {
	int rv;
	int how;
};

struct sysctl_args {
	int rv;
	int *name;
	uint namelen;
	void *oldp;
	size_t *oldlenp;
	void *newp;
	size_t newlen;
};

struct getopt_args {
	int rv;
	int level;
	int optnum;
	caddr_t val;
	size_t *valsize;
};

struct setopt_args {
	int rv;
	int level;
	int optnum;
	const void *val;
	size_t valsize;
};

struct getname_args {
	int rv;
	struct sockaddr *name;
	uint32 *namelen;
};

struct accept_args {
	int rv;
	struct sockaddr *name;
	uint32 *namelen;
};

/* To simplify the addition of ones it's an enum! */
enum {
	NET_SOCKET_CREATE = B_DEVICE_OP_CODES_END + 0x100,
	NET_SOCKET_BIND,
	NET_SOCKET_RECVFROM,
	NET_SOCKET_SENDTO,
	NET_SOCKET_LISTEN,
	NET_SOCKET_ACCEPT,
	NET_SOCKET_CONNECT,
	NET_SOCKET_SELECT,
	NET_SOCKET_SHUTDOWN,
	NET_SOCKET_SYSCTL,
	NET_SOCKET_GETSOCKOPT,
	NET_SOCKET_SETSOCKOPT,
	NET_SOCKET_GETSOCKNAME,
	NET_SOCKET_GETPEERNAME
};


#endif /* NET_STRCUTURES_H */
