/* libnet.c */

/* This is a very simple little shared library that acts as a wrapper
 * for our device/kernel stack!
 */
#include <fcntl.h>
#include <unistd.h>
#include <kernel/OS.h>
#include <iovec.h>
#include <stdio.h>

#include "sys/socket.h"
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

int bind(int sock, const struct sockaddr *sa, int len)
{
	struct bind_args ba;
	
	ba.data = (caddr_t)sa;
	ba.dlen = len;
	
	return ioctl(sock, NET_SOCKET_BIND, &ba, sizeof(ba));
}

int recvfrom(int sock, caddr_t buffer, size_t buflen, int flags,
             struct sockaddr *addr, size_t addrlen)
{
	struct msghdr mh;
	struct iovec iov;

	/* XXX - would this be better done as scatter gather? */	
	mh.msg_name = (caddr_t)addr;
	mh.msg_namelen = addrlen;
	mh.msg_flags = flags;
	mh.msg_control = NULL;
	mh.msg_controllen = 0;
	iov.iov_base = buffer;
	iov.iov_len = buflen;
	mh.msg_iov = &iov;
	mh.msg_iovlen = 1;

	return ioctl(sock, NET_SOCKET_RECVFROM, &mh, sizeof(mh));
}

int sendto(int sock, caddr_t buffer, size_t buflen, int flags,
           const struct sockaddr *addr, size_t addrlen)
{
	struct msghdr mh;
	struct iovec iov;

	/* XXX - would this be better done as scatter gather? */	
	mh.msg_name = (caddr_t)addr;
	mh.msg_namelen = addrlen;
	mh.msg_flags = flags;
	mh.msg_control = NULL;
	mh.msg_controllen = 0;
	iov.iov_base = buffer;
	iov.iov_len = buflen;
	mh.msg_iov = &iov;
	mh.msg_iovlen = 1;

	return ioctl(sock, NET_SOCKET_SENDTO, &mh, sizeof(mh));
}

int net_select(int nbits, struct fd_set *rbits, 
                      struct fd_set *wbits, 
                      struct fd_set *ebits, 
                      struct timeval *timeout)
{
	int tmpfd = open("/dev/net/socket", O_RDWR);
	struct select_args sa;
	int error;
	int i;
	
	sa.mfd = nbits;
	sa.rbits = rbits;
	sa.wbits = wbits;
	sa.ebits = ebits;
	sa.tv = timeout;
	
	for (i=2; i <10;i++) {
		if (rbits)
			if (FD_ISSET(i, rbits))
				printf("socket %d set in read bits...\n", i);
	}
	
	error = ioctl(tmpfd, NET_SOCKET_SELECT, &sa, sizeof(sa));
printf("error = %d\n", error);
	close(tmpfd);
	return (error);
}
