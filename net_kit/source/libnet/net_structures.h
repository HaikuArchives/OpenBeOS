/* net_structures.h
 * structures and defines to deal with the device driver...
 */

#ifndef NET_STRCUTURES_H
#define NET_STRCUTURES_H

#include <Drivers.h>
#include "sys/select.h"
#include <sys/time.h>

struct socket_args {
	int dom;
	int type;
	int prot;
};

struct bind_args {
	caddr_t data;
	int dlen;
};

struct listen_args {
	int backlog;
};

struct connect_args {
	caddr_t name;
	int namelen;
};

struct select_args {
	int mfd;
	struct fd_set *rbits;
	struct fd_set *wbits;
	struct fd_set *ebits;
	struct timeval *tv;
};
	
/* To simplyify the addition of ones! */
enum {
	NET_SOCKET_CREATE = B_DEVICE_OP_CODES_END + 0x100,
	NET_SOCKET_BIND,
	NET_SOCKET_RECVFROM,
	NET_SOCKET_SENDTO,
	NET_SOCKET_LISTEN,
	NET_SOCKET_CONNECT,
	NET_SOCKET_SELECT
};


#endif /* NET_STRCUTURES_H */
