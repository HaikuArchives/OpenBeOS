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

static char * g_socket_driver_path = "/dev/net/socket";

int socket(int domain, int type, int protocol)
{
	int fd;
	int rv;
	struct socket_args sa;

	fd = open(g_socket_driver_path, O_RDWR);
	if (fd < 0)
		return fd;

	sa.dom = domain;
	sa.type = type;
	sa.prot = protocol;

	sa.rv = B_OK;
	rv = ioctl(fd, NET_SOCKET_CREATE, &sa, sizeof(sa));
	if (rv == 0)
		rv = sa.rv;

	if (rv < 0) {
		close(fd);
		fd = rv;
	};

	return fd;
}

int closesocket(int sock)
{
	return close(sock);
}

int bind(int sock, const struct sockaddr *sa, int len)
{
	struct bind_args ba;
	int rv;
	
	ba.data = (caddr_t)sa;
	ba.dlen = len;
	
	ba.rv = B_OK;
	rv = ioctl(sock, NET_SOCKET_BIND, &ba, sizeof(ba));
	return (rv < 0) ? rv : ba.rv;
}

int recvfrom(int sock, caddr_t buffer, size_t buflen, int flags,
             struct sockaddr *addr, size_t *addrlen)
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

int select(int nbits, struct fd_set *rbits, 
                      struct fd_set *wbits, 
                      struct fd_set *ebits, 
                      struct timeval *timeout)
{
	int tmpfd;
	struct select_args sa;
	int rv;
	int i;
	
	tmpfd = open(g_socket_driver_path, O_RDWR);
	if (tmpfd < 0)
		return tmpfd;

	sa.mfd = nbits;
	sa.rbits = rbits;
	sa.wbits = wbits;
	sa.ebits = ebits;
	sa.tv = timeout;
	
	for (i=3; i < sa.mfd;i++) {
		printf("socket %d: ", i);
		if (rbits)
			if (FD_ISSET(i, rbits))
				printf(" read ");
		if (wbits)
			if (FD_ISSET(i, wbits))
				printf(" write ");
		if (ebits)
			if (FD_ISSET(i, ebits))
				printf(" except ");
		printf("\n");
	}
	
	sa.rv = B_OK;
	rv = ioctl(tmpfd, NET_SOCKET_SELECT, &sa, sizeof(sa));
	printf("error = %d\n", rv);

	close(tmpfd);
	
	return (rv < 0) ? rv : sa.rv;
}

int shutdown(int sock, int how)
{
	struct shutdown_args sa;
	int rv;
	
	sa.how = how;
	sa.rv = B_OK;
	rv = ioctl(sock, NET_SOCKET_SHUTDOWN, &sa, sizeof(sa));
	return (rv < 0) ? rv : sa.rv;
}

int sysctl (int *name, uint namelen, void *oldp, size_t *oldlenp, 
            void *newp, size_t newlen)
{
	int s;
	struct sysctl_args sa;
	int rv;
	
	s = socket(PF_ROUTE, SOCK_RAW, 0);
	if (s < 0)
		return s;
		
	sa.name = name;
	sa.namelen = namelen;
	sa.oldp = oldp;
	sa.oldlenp = oldlenp;
	sa.newp = newp;
	sa.newlen = newlen;
	
	sa.rv = B_OK;
	rv = ioctl(s, NET_SOCKET_SYSCTL, &sa, sizeof(sa));

	close(s);
	
	return (rv < 0) ? rv : sa.rv;
}
	
int connect(int sock, const struct sockaddr *name, int namelen)
{
	return -1;
}

int send(int sock, const caddr_t data, int buflen, int flags)
{
	return -1;
}


