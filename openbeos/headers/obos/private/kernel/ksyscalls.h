/*
** Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <types.h>
#ifndef _KERNEL_SYSCALLS_H
#define _KERNEL_SYSCALLS_H

enum {
	SYSCALL_NULL = 0,
	SYSCALL_MOUNT,
	SYSCALL_UNMOUNT,
	SYSCALL_SYNC,
	SYSCALL_OPEN,
	SYSCALL_CLOSE,
	SYSCALL_FSYNC,
	SYSCALL_READ,
	SYSCALL_WRITE,
	SYSCALL_SEEK,
	SYSCALL_IOCTL,	/* 10 */
	SYSCALL_CREATE,
	SYSCALL_UNLINK,
	SYSCALL_RENAME,
	SYSCALL_RSTAT,
	SYSCALL_WSTAT,
	SYSCALL_SYSTEM_TIME,
	SYSCALL_SNOOZE,
	SYSCALL_SEM_CREATE,
	SYSCALL_SEM_DELETE,
	SYSCALL_SEM_ACQUIRE,	/* 20 */
	SYSCALL_SEM_ACQUIRE_ETC,
	SYSCALL_SEM_RELEASE,
	SYSCALL_SEM_RELEASE_ETC,
	SYSCALL_GET_CURRENT_THREAD_ID,
	SYSCALL_EXIT_THREAD,
	SYSCALL_PROC_CREATE_PROC,
	SYSCALL_THREAD_WAIT_ON_THREAD,
	SYSCALL_PROC_WAIT_ON_PROC,
	SYSCALL_VM_CREATE_ANONYMOUS_REGION,
	SYSCALL_VM_CLONE_REGION, /* 30 */
	SYSCALL_VM_MAP_FILE,
	SYSCALL_VM_DELETE_REGION,
	SYSCALL_VM_GET_REGION_INFO,
	SYSCALL_VM_FIND_REGION_BY_NAME,
	SYSCALL_SPAWN_THREAD,
	SYSCALL_KILL_THREAD,
	SYSCALL_SUSPEND_THREAD,
	SYSCALL_RESUME_THREAD,
	SYSCALL_PROC_KILL_PROC,
	SYSCALL_GET_CURRENT_PROC_ID,
	SYSCALL_GETCWD, /* 40 */
	SYSCALL_SETCWD,
	SYSCALL_PORT_CREATE,
	SYSCALL_PORT_CLOSE,
	SYSCALL_PORT_DELETE,
	SYSCALL_PORT_FIND,
	SYSCALL_PORT_GET_INFO,
	SYSCALL_PORT_GET_NEXT_PORT_INFO,
	SYSCALL_PORT_BUFFER_SIZE,
	SYSCALL_PORT_BUFFER_SIZE_ETC,
	SYSCALL_PORT_COUNT, /* 50 */
	SYSCALL_PORT_READ,
	SYSCALL_PORT_READ_ETC,
	SYSCALL_PORT_SET_OWNER,
	SYSCALL_PORT_WRITE,
	SYSCALL_PORT_WRITE_ETC,
	SYSCALL_SEM_GET_COUNT,
	SYSCALL_SEM_GET_SEM_INFO,
	SYSCALL_SEM_GET_NEXT_SEM_INFO,
	SYSCALL_SEM_SET_SEM_OWNER,
	SYSCALL_FDDUP, /* 60 */
	SYSCALL_FDDUP2,
	SYSCALL_GET_PROC_TABLE,
	SYSCALL_GETRLIMIT,
	SYSCALL_SETRLIMIT,
	SYSCALL_ATOMIC_ADD,
	SYSCALL_ATOMIC_AND,
	SYSCALL_ATOMIC_OR,
	SYSCALL_ATOMIC_SET,
	SYSCALL_TEST_AND_SET,/* 70 */
	SYSCALL_SYSCTL,
	SYSCALL_SOCKET,
	SYSCALL_GETDTABLESIZE,
	SYSCALL_FSTAT
};

int syscall_dispatcher(unsigned long call_num, void *arg_buffer, uint64 *call_ret);

#endif

