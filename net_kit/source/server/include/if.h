/* if.h
 * Interface definitions for beos
 */
 
#ifndef OBOS_IF_H
#define OBOS_IF_H

#include <Drivers.h>
#include "net_misc.h"

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

typedef struct ifnet {
	int dev;	/* device handle */
	int id;		/* id within the stack's device list */
	char *name;	/* name of driver */
	int type;
	ether_addr mac;	/* The ethernet address if there is one... */
	int flags;	/* flags */
	int mtu;	/* mtu */

	thread_id rx_thread;	
	thread_id tx_thread;
} ifnet;

#endif /* OBOS_IF_H */
