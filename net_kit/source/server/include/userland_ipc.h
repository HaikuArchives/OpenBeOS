#ifndef USERLAND_IPC_H
#define USERLAND_IPC_H
/* userland_ipc - Communication between the network driver
**		and the userland stack.
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include <OS.h>
#include "net_stack_driver.h"


#define NET_STACK_PORTNAME "net_server connection"

enum {
	NET_STACK_OPEN = NET_STACK_IOCTL_MAX,
	NET_STACK_CLOSE,
	NET_STACK_NEW_CONNECTION,
};

typedef struct {
	int32	op;
	int32	result;
} net_command;

#define CONNECTION_QUEUE_LENGTH 128
#define NET_BUFFER_SIZE 2048

typedef struct {
	port_id	port;
	area_id area;

	sem_id	commandSemaphore;	// command queue
	uint32	numCommands;
	uint32	numReadBuffers,numWriteBuffers;
} net_connection;

#endif	/* USERLAND_IPC_H */
