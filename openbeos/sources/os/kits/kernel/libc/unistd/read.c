/* 
** Copyright 2001, Manuel J. Petit. All rights reserved.
** Distributed under the terms of the NewOS License.
*/

#include <unistd.h>
#include <syscalls.h>


ssize_t
read(int fd, void *buf, size_t len)
{
	int retval;

	retval= sys_read(fd, buf, -1, len);

	if(retval< 0) {
		// set errno
	}

	return retval;
}
