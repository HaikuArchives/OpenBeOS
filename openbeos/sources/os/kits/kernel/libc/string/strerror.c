/* 
** Copyright 2001, Dan Sinclair. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <types.h>
#include <errors.h>
#include <string.h>

char const *
strerror(int errnum)
{
	switch(errnum) {
		/* General Errors */
		case NO_ERROR:
			return "No Error";
		;
		
		case ERR_GENERAL:
			return "General Error";
		;

//		case ERR_NO_MEMORY:
		case ENOMEM:
			return "Cannot allocate memory";
		;

		case ERR_IO_ERROR:
			return "Input/Output error";
		;

		case ERR_INVALID_ARGS:
			return "Invalid Arguments";
		;

//		case ERR_TIMED_OUT:
		case ETIMEDOUT:
			return "Timed out";
		;

//		case ERR_NOT_ALLOWED:
		case EPERM:
			return "Operation not permitted";
		;

//		case ERR_PERMISSION_DENIED:
		case EACCES:
			return "Operation not permitted";
		;

		case ERR_INVALID_BINARY:
			return "Invalid Binary";
		;

		case ERR_INVALID_HANDLE:
			return "Invalid ID Handle";
		;

		case ERR_NO_MORE_HANDLES:
			return "No more handles";
		;

//		case ERR_UNIMPLEMENTED:
		case ENOSYS:
			return "Unimplemented";
		;

//		case ERR_TOO_BIG:
		case EDOM:
			return "Numerical argument out of range";
		;

		case ERR_NOT_FOUND:
			return "Not found";
		;

		case ERR_NOT_IMPLEMENTED_YET:
			return "Not implemented yet";
		;


		/* Semaphore errors */
		case ERR_SEM_GENERAL:
			return "General semaphore error";
		;

		case ERR_SEM_DELETED:
			return "Semaphore deleted";
		;

		case ERR_SEM_TIMED_OUT:
			return "Semaphore timed out";
		;

		case ERR_SEM_OUT_OF_SLOTS:
			return "Semaphore out of slots";
		;

		case ERR_SEM_NOT_ACTIVE:
			return "Semaphore not active";
		;

		case ERR_SEM_INTERRUPTED:
			return "Semaphore interrupted";
		;

		case ERR_SEM_NOT_INTERRUPTABLE:
			return "Semaphore not interruptable";
		;

		case ERR_SEM_NOT_FOUND:
			return "Semaphore not found";
		;

		/* Tasker errors */
		case ERR_TASK_GENERAL:
			return "General Tasker error";
		;

		case ERR_TASK_PROC_DELETED:
			return "Tasker process deleted";
		;

		/* VFS errors */
		case ERR_VFS_GENERAL:
			return "General VFS error";
		;

		case ERR_VFS_INVALID_FS:
			return "VFS invalid filesystem";
		;

		case ERR_VFS_NOT_MOUNTPOINT:
			return "VFS not a mount point";
		;

		case ERR_VFS_PATH_NOT_FOUND:
			return "VFS path not found";
		;

//		case ERR_VFS_INSUFFICIENT_BUF:
		case ENOBUFS:
			return "VFS insufficient buffer";
		;

//		case ERR_VFS_READONLY_FS:
		case EROFS:
			return "VFS readonly filesystem";
		;

//		case ERR_VFS_ALREADY_EXISTS:
		case EEXIST:
			return "VFS already exists";
		;

//		case ERR_VFS_FS_BUSY:
		case EBUSY:
			return "Device busy";
		;

		case ERR_VFS_FD_TABLE_FULL:
			return "VFS file descriptor table full";
		;

		case ERR_VFS_CROSS_FS_RENAME:
			return "VFS cross filesystem rename";
		;

		case ERR_VFS_DIR_NOT_EMPTY:
			return "Directory not empty";
		;

//		case ERR_VFS_NOT_DIR:
		case ENOTDIR:
			return "Not a directory";
		;

		case ERR_VFS_WRONG_STREAM_TYPE:
			return "VFS wrong stream type";
		;

		case ERR_VFS_ALREADY_MOUNTPOINT:
			return "VFS already a mount point";
		;

		/* VM errors */
		case ERR_VM_GENERAL:
			return "General VM error";
		;

		case ERR_VM_INVALID_ASPACE:
			return "VM invalid aspace";
		;

		case ERR_VM_INVALID_REGION:
			return "VM invalid region";
		;

		case ERR_VM_BAD_ADDRESS:
			return "VM bad address";
		;

		case ERR_VM_PF_FATAL:
			return "VM PF fatal";
		;

		case ERR_VM_PF_BAD_ADDRESS:
			return "VM PF bad address";
		;

		case ERR_VM_PF_BAD_PERM:
			return "VM PF bad permissions";
		;

		case ERR_VM_PAGE_NOT_PRESENT:
			return "VM page not present";
		;

		case ERR_VM_NO_REGION_SLOT:
			return "VM no region slot";
		;

		case ERR_VM_WOULD_OVERCOMMIT:
			return "VM would overcommit";
		;

		case ERR_VM_BAD_USER_MEMORY:
			return "VM bad user memory";
		;

		/* Elf errors */
		case ERR_ELF_GENERAL:
			return "General ELF error";
		;

		case ERR_ELF_RESOLVING_SYMBOL:
			return "ELF resolving symbol";
		;

		/* Ports errors */
		case ERR_PORT_GENERAL:
			return "General port error";
		;

		case ERR_PORT_DELETED:
			return "Port deleted";
		;

		case ERR_PORT_OUT_OF_SLOTS:
			return "Port out of slots";
		;

		case ERR_PORT_NOT_ACTIVE:
			return "Port not active";
		;

		case ERR_PORT_CLOSED:
			return "Port closed";
		;

		case ERR_PORT_TIMED_OUT:
			return "Port timed out";
		;

		case ERR_PORT_INTERRUPTED:
			return "Port interrupted";
		;

		case ERR_PORT_NOT_FOUND:
			return "Port not found";
		;

		default:
			return "Unknown errnum";
		;
	}
}
