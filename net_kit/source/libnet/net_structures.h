/* net_structures.h
 * structures and defines to deal with the device driver...
 */

#ifndef NET_STRCUTURES_H
#define NET_STRCUTURES_H

#include <Drivers.h>

struct socket_args {
	int dom;
	int type;
	int prot;
};

/* To simplyify the addition of ones! */
enum {
	NET_SOCKET_CREATE = B_DEVICE_OP_CODES_END + 0x100
};


#endif /* NET_STRCUTURES_H */
