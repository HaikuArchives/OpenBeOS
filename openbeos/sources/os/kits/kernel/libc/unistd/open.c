/* 
** Copyright 2001, Manuel J. Petit. All rights reserved.
** Distributed under the terms of the NewOS License.
*/

#include <unistd.h>
#include <syscalls.h>
#include <fcntl.h>
#include <errno.h>

/* XXX - no support yet for setting modes (mode_t as a 3rd argument)
 * but va_args commented out to provide the variable. Need to add support to
 * sys_open and kernel first
 */

int
open(const char *path, int oflags, ...)
{
	int retval;
//	va_args args;
printf("open: %s: %d\n", path, oflags);
//	va_start(args, oflags);
	retval= sys_open(path, STREAM_TYPE_ANY, oflags);
//	va_end(args);

	if (retval < 0) {
		errno = retval;
		retval = -1;
	}

	return retval;
}
