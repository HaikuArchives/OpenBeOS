/* if.h
 * Interface definitions for beos
 */
 
#ifndef OBOS_IF_H
#define OBOS_IF_H

#include <Drivers.h>

enum {
	IF_GETADDR = B_DEVICE_OP_CODES_END,
	IF_INIT,
	IF_NONBLOCK,
	IF_ADDMULTI,
	IF_REMMULTI,
	IF_SETPROMISC,
	IF_GETFRAMESIZE
};

#endif /* OBOS_IF_H */
