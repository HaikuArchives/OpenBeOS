/* libnet.c */

/* This is a very simple little shared library that acts as a wrapper
 * for our device/kernel stack!
 */
#include <fcntl.h>
#include <unistd.h>

#include "net_structures.h"

int socket(int domain, int type, int protocol)
{
	int fd = open("/dev/net/socket", O_RDWR);
	int rv = 0;
	struct socket_args sa;

	if (fd < 0)
		return fd;

	sa.dom = domain;
	sa.type = type;
	sa.prot = protocol;

	rv = ioctl(fd, NET_SOCKET_CREATE, &sa, sizeof(sa));
	if (rv < 0)
		return rv;

	return fd;
}

int closesocket(int sock)
{
	return close(sock);
}

