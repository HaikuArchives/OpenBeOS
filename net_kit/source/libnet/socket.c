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
#include "netinet/in.h"
#include "net_structures.h"

const char * g_socket_driver_path = "/dev/net/socket";

int socket(int domain, int type, int protocol)
{
	int fd;
	int rv;
	struct socket_args sa;

/* Not sure what you'll do with this now as binary compat is
 * broken until you do something about it guys.
 * Your call.
 * Fix it - and fix it soon.
 */
 
	/* work around the old Be header values... */
	if (type < 10) {
		/* we have old be types... convert... */
		if (type == 1)
			type = SOCK_DGRAM;
		if (type == 2)
			type = SOCK_STREAM;
		if (protocol == 1)
			protocol = IPPROTO_UDP;
		if (protocol == 2)
			protocol = IPPROTO_TCP;
		if (protocol == 3)
			protocol = IPPROTO_ICMP;
	}
	
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
	mh.msg_namelen = *addrlen;
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
	struct connect_args ca;
	int rv;
	
	ca.rv = 0;
	ca.name = (caddr_t)name;
	ca.namelen = namelen;
	
	ca.rv = B_OK;
	rv = ioctl(sock, NET_SOCKET_CONNECT, &ca, sizeof(ca));
	return (rv < 0) ? rv : ca.rv;
}

/* These need to be adjusted to take account of the MSG_PEEK
 * flag, but old R5 doesn't use it...
 */
int recv(int sock, caddr_t data, int buflen, int flags)
{
	/* flags gets ignored here... */
	return read(sock, data, buflen);
}

int send(int sock, const caddr_t data, int buflen, int flags)
{
	return write(sock, data, buflen);
}

int getsockopt(int sock, int level, int optnum, void * val, size_t *valsize)
{
	struct getopt_args ga;

	ga.rv = 0;	
	ga.level = level;
	ga.optnum = optnum;
	ga.val = val;
	ga.valsize = valsize;
	
	ioctl(sock, NET_SOCKET_GETSOCKOPT, &ga, sizeof(ga));
	return ga.rv;
}

int setsockopt(int sock, int level, int optnum, const void *val, size_t valsize)
{
	struct setopt_args sa;

	sa.rv = 0;	
	sa.level = level;
	sa.optnum = optnum;
	sa.val = val;
	sa.valsize = valsize;
	
	ioctl(sock, NET_SOCKET_SETSOCKOPT, &sa, sizeof(sa));
	return sa.rv;
}

int getpeername(int sock, struct sockaddr *name, uint32 *namelen)
{
	struct getname_args ga;
	
	ga.rv = 0;
	ga.name = name;
	ga.namelen = namelen;
	
	ioctl(sock, NET_SOCKET_GETPEERNAME, &ga, sizeof(ga));
	return ga.rv;
}

int getsockname(int sock, struct sockaddr *name, uint32 *namelen)
{
	struct getname_args ga;
	
	ga.rv = 0;
	ga.name = name;
	ga.namelen = namelen;
	
	ioctl(sock, NET_SOCKET_GETSOCKNAME, &ga, sizeof(ga));
	return ga.rv;
}

int listen(int sock, int backlog)
{
	struct listen_args la;
	
	la.rv = 0;
	la.backlog = backlog;
	
	ioctl(sock, NET_SOCKET_LISTEN, &la, sizeof(la));
	return la.rv;
}

int accept(int sock, struct sockaddr *name, uint32 *namelen)
{
	struct accept_args aa;
	
	aa.rv = 0;
	aa.name = name;
	aa.namelen = namelen;
	
	ioctl(sock, NET_SOCKET_ACCEPT, &aa, sizeof(aa));
	return aa.rv;
}

/* these are for compatibility with BeOS R5... */

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

