/* compat.c */

/* These routines are included in libnet simply because R5 expects them
 * to be there. They should mostly be no-ops...
 */

#include <fcntl.h>
#include <unistd.h>
#include <kernel/OS.h>
#include <iovec.h>
#include <stdio.h>

int herror()
{
	printf("herror() not yet supported.");
	return 0;
}

int _socket_interrupt()
{
	printf("_socket_interrupt\n");
	return 0;
}

int _netconfig_find()
{
	printf("_netconfig_find\n");
	return 0;
}

int getusername(char *username, size_t userlen)
{
	if (userlen < 6)
		return -1;
	memcpy(username, "baron\0", 6);
	return 6;
}
