#ifndef _PORT_TYPES_H
#define _PORT_TYPES_H

#include <ktypes.h>
#include <types.h>
#include <defines.h>

// PORT_FLAG_INTERRUPTABLE must be the same as SEM_FLAG_INTERRUPTABLE
// PORT_FLAG_TIMEOUT       must be the same as SEM_FLAG_TIMEOUT
#define PORT_FLAG_TIMEOUT 2
#define PORT_FLAG_INTERRUPTABLE 4
#define PORT_FLAG_USE_USER_MEMCPY 0x80000000
typedef struct port_info {
	port_id id;
	proc_id owner;
	char name[SYS_MAX_OS_NAME_LEN];
	int32 capacity;
	int32 queue_count;
	int32 total_count;
} port_info;

#endif

