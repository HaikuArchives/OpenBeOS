/* 
** Copyright 2001-2002, Mark-Jan Bastian. All rights reserved.
** Distributed under the terms of the NewOS License.
*/

#ifndef _KERNEL_PORT_H
#define _KERNEL_PORT_H

/*
 *	beos-style kernel ports
 *	provides a queue of msg'code and variable-length data
 *	that can be used by any two threads to exchange data
 *	with each other
 */

#include <thread.h>
#include <port_types.h>
#include <sem.h>
#include <port_types.h>

int port_init(kernel_args *ka);

// kernel API
port_id		create_port(int32 queue_length, const char *name);
int			close_port(port_id id);
int			delete_port(port_id id);
port_id		find_port(const char *port_name);
int			_get_port_info(port_id id, struct port_info *info, size_t);
int		 	_get_next_port_info(proc_id proc, uint32 *, 
                                struct port_info *, size_t);
ssize_t		port_buffer_size(port_id port);
ssize_t		port_buffer_size_etc(port_id port,
				uint32 flags,
				bigtime_t timeout);
int32		port_count(port_id port);
ssize_t		read_port(port_id port,
				int32 *msg_code,
				void *msg_buffer,
				size_t buffer_size);
ssize_t		read_port_etc(port_id port,
				int32 *msg_code,
				void *msg_buffer,
				size_t buffer_size,
				uint32 flags,
				bigtime_t timeout);
int			set_port_owner(port_id port, proc_id proc);
int			write_port(port_id port,
				int32 msg_code,
				void *msg_buffer,
				size_t buffer_size);
int		 	write_port_etc(port_id port,
				int32 msg_code,
				void *msg_buffer,
				size_t buffer_size,
				uint32 flags,
				bigtime_t timeout);
int delete_owned_ports(proc_id owner);

#define get_port_info(port, info)    \
             _get_port_info((port), (info), sizeof(*(info)))
	
#define get_next_port_info(team, cookie, info)   \
	         _get_next_port_info((team), (cookie), (info), sizeof(*(info)))


// temp: test
void port_test(void);
int	 port_test_thread_func(void* arg);

// user-level API
port_id		user_create_port(int32 queue_length, const char *name);
int			user_close_port(port_id id);
int			user_delete_port(port_id id);
port_id		user_find_port(const char *port_name);
int			user_get_port_info(port_id id, struct port_info *info);
int		 	user_get_next_port_info(proc_id proc,
				uint32 *cookie,
				struct port_info *info);
ssize_t		user_port_buffer_size_etc(port_id port,
				uint32 flags,
				bigtime_t timeout);
int32		user_port_count(port_id port);
ssize_t		user_read_port_etc(port_id port,
				int32 *msg_code,
				void *msg_buffer,
				size_t buffer_size,
				uint32 flags,
				bigtime_t timeout);
int			user_set_port_owner(port_id port, proc_id proc);
int			user_write_port_etc(port_id port,
				int32 msg_code,
				void *msg_buffer,
				size_t buffer_size,
				uint32 flags,
				bigtime_t timeout);

#endif
