/* 
** Copyright 2001, Manuel J. Petit. All rights reserved.
** Distributed under the terms of the NewOS License.
*/

#include <unistd.h>
#include <syscalls.h>


int
open(char const *path, int omode, ...)
{
	int retval;

	retval= sys_open(path, STREAM_TYPE_ANY , omode);

	if(retval< 0) {
		// set errno
	}

	return retval;
}
