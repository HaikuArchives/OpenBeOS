/* socket.c */

/* This is a very simple little shared library that acts as a wrapper
 * for our device/kernel stack!
 */

#include <fcntl.h>
#include <unistd.h>
#include <kernel/OS.h>
#include <iovec.h>
#include <stdio.h>

#include "sys/socket.h"
#include "sys/sockio.h"
#include "netinet/in.h"
#include "net_structures.h"

// global variables
const char * g_socket_driver_path = "/dev/net/socket";
// global libraries data are per team
bool g_beos_r5_compatibility = false;

// local definition
struct beosr5_sockaddr_in {
	uint16 sin_family;
	uint16 sin_port;
	uint32 sin_addr;
	char sin_zero[4];
};

// static prototypes
static void convert_from_beos_r5_sockaddr(struct sockaddr *to, const struct sockaddr *from);
static void convert_to_beos_r5_sockaddr(struct sockaddr *to, const struct sockaddr *from);
static void	convert_from_beos_r5_sockopt(int * level, int * optnum);


#ifdef CODEWARRIOR
	#pragma mark [BSD sockets API]
#endif

int socket(int domain, int type, int protocol)
{
	int fd;
	int rv;
	struct socket_args sa;

	fd = open(g_socket_driver_path, O_RDWR);
	if (fd < 0)
		return fd;

	/* work around the old Be header values... */
	if (type < SOCK_DGRAM) {
		// if caller use type value between 0 and 2, it was linked
		// against old R5 net socket-like API.
		g_beos_r5_compatibility = true;
		
		/* we have old be types... convert... */
		if (type == 1)
			type = SOCK_DGRAM;
		else if (type == 2)
			type = SOCK_STREAM;

		if (protocol == 1)
			protocol = IPPROTO_UDP;
		else if (protocol == 2)
			protocol = IPPROTO_TCP;
		else if (protocol == 3)
			protocol = IPPROTO_ICMP;
	};
	
	sa.dom = domain;
	sa.type = type;
	sa.prot = protocol;

	sa.rv = B_ERROR;
	rv = ioctl(fd, NET_SOCKET_CREATE, &sa, sizeof(sa));
	if (rv == 0)
		rv = sa.rv;

	if (rv < 0) {	// socket creation failure...
		close(fd);
		fd = rv;
	};

	return fd;
}

int closesocket(int sock)
{
	return close(sock);
}

int bind(int sock, const struct sockaddr * sa, int len)
{
	struct sockaddr temp;
	struct bind_args ba;
	int rv;
	
	if (g_beos_r5_compatibility) {
		convert_from_beos_r5_sockaddr(&temp, sa);
		sa = &temp;
		len = sizeof(struct sockaddr_in);
	}
	
	ba.data = (caddr_t)sa;
	ba.dlen = len;
	
	ba.rv = B_ERROR;
	rv = ioctl(sock, NET_SOCKET_BIND, &ba, sizeof(ba));
	return (rv < 0) ? rv : ba.rv;
}

int recvfrom(int sock, void * buffer, size_t buflen, int flags,
             struct sockaddr *addr, size_t *addrlen)
{
	struct sockaddr temp;
	struct msghdr mh;
	struct iovec iov;
	int rv;

	/* XXX - would this be better done as scatter gather? */	
	mh.msg_name = g_beos_r5_compatibility ?  (caddr_t)&temp : (caddr_t)addr;
	mh.msg_namelen = g_beos_r5_compatibility ? sizeof(temp) : addrlen ? *addrlen : 0;
	mh.msg_flags = flags;
	mh.msg_control = NULL;
	mh.msg_controllen = 0;
	iov.iov_base = buffer;
	iov.iov_len = buflen;
	mh.msg_iov = &iov;
	mh.msg_iovlen = 1;

	rv = ioctl(sock, NET_SOCKET_RECVFROM, &mh, sizeof(mh));
	if (rv < 0)
		return rv;
		
	if (g_beos_r5_compatibility && addr)
		convert_to_beos_r5_sockaddr(addr, &temp);
	
	if (addrlen) {
		if (g_beos_r5_compatibility)
			*addrlen = sizeof(struct beosr5_sockaddr_in);
		else
			*addrlen = mh.msg_namelen;
	}
		
	return rv;
}

int sendto(int sock, const void * buffer, size_t buflen, int flags,
           const struct sockaddr *addr, size_t addrlen)
{
	struct sockaddr temp;
	struct msghdr mh;
	struct iovec iov;

	if (g_beos_r5_compatibility) {
		convert_from_beos_r5_sockaddr(&temp, addr);
		addr = &temp;
		addrlen = sizeof(struct sockaddr_in);
	}

	/* XXX - would this be better done as scatter gather? */	
	mh.msg_name = (caddr_t)addr;
	mh.msg_namelen = addrlen;
	mh.msg_flags = flags;
	mh.msg_control = NULL;
	mh.msg_controllen = 0;
	iov.iov_base = (caddr_t)buffer;
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
	
	sa.rv = B_ERROR;
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
	
	sa.rv = B_ERROR;
	rv = ioctl(s, NET_SOCKET_SYSCTL, &sa, sizeof(sa));

	close(s);
	
	return (rv < 0) ? rv : sa.rv;
}
	
int connect(int sock, const struct sockaddr * addr, int addrlen)
{
	struct sockaddr temp;
	struct connect_args ca;
	int rv;
	
	if (g_beos_r5_compatibility) {
		convert_from_beos_r5_sockaddr(&temp, addr);
		addr = &temp;
		addrlen = sizeof(struct sockaddr_in);
	}

	ca.name = (caddr_t)addr;
	ca.namelen = addrlen;
	
	ca.rv = B_ERROR;
	rv = ioctl(sock, NET_SOCKET_CONNECT, &ca, sizeof(ca));
	return (rv < 0) ? rv : ca.rv;
}

/* These need to be adjusted to take account of the MSG_PEEK
 * flag, but old R5 doesn't use it...
 */
int recv(int sock, void * data, int datalen, int flags)
{
	/* flags gets ignored here... */
	return read(sock, data, datalen);
}

int send(int sock, const void * data, int datalen, int flags)
{
	return write(sock, data, datalen);
}

int getsockopt(int sock, int level, int optnum, void * val, size_t *valsize)
{
	struct getopt_args ga;
	int rv;

	if (g_beos_r5_compatibility && optnum == 5) { // BeOS R5 SO_FIONREAD
		status_t rv;
		int temp;
		rv = ioctl(sock,FIONREAD,&temp);
		*(int*)val = temp;
		return rv;
	}

	if (g_beos_r5_compatibility)
		convert_from_beos_r5_sockopt(&level, &optnum);

	ga.level = level;
	ga.optnum = optnum;
	ga.val = val;
	ga.valsize = valsize;
	
	ga.rv = B_ERROR;	
	rv = ioctl(sock, NET_SOCKET_GETSOCKOPT, &ga, sizeof(ga));
	return (rv < 0) ? rv : ga.rv;
}

int setsockopt(int sock, int level, int optnum, const void *val, size_t valsize)
{
	struct setopt_args sa;
	int rv;
	
	if (g_beos_r5_compatibility && optnum == 3) { // BeOS R5 SO_NONBLOCK
		int temp;
		temp = (*(int*)val) ? 1 : 0;
		return ioctl(sock,FIONBIO,&temp);
	}

	if (g_beos_r5_compatibility)
		convert_from_beos_r5_sockopt(&level, &optnum);

	sa.level = level;
	sa.optnum = optnum;
	sa.val = val;
	sa.valsize = valsize;
	
	sa.rv = B_ERROR;	
	rv = ioctl(sock, NET_SOCKET_SETSOCKOPT, &sa, sizeof(sa));
	return (rv < 0) ? rv : sa.rv;
}

int getpeername(int sock, struct sockaddr * addr, int * addrlen)
{
	struct sockaddr temp;
	struct getname_args ga;
	int rv;
	
	if (g_beos_r5_compatibility)
		*addrlen = sizeof(temp);
	
	ga.name = g_beos_r5_compatibility ? &temp : addr;
	ga.namelen = addrlen;
	
	ga.rv = B_ERROR;
	rv = ioctl(sock, NET_SOCKET_GETPEERNAME, &ga, sizeof(ga));
	if (rv < 0)
		return rv;
		
	if (g_beos_r5_compatibility) {
		convert_to_beos_r5_sockaddr(addr, &temp);
		*addrlen = sizeof(struct beosr5_sockaddr_in);
	}

	return ga.rv;
}

int getsockname(int sock, struct sockaddr * addr, int * addrlen)
{
	struct sockaddr temp;
	struct getname_args ga;
	int rv;
	
	if (g_beos_r5_compatibility)
		*addrlen = sizeof(temp);
	
	ga.name = g_beos_r5_compatibility ? &temp : addr;
	ga.namelen = addrlen;
	
	ga.rv = B_ERROR;
	rv = ioctl(sock, NET_SOCKET_GETSOCKNAME, &ga, sizeof(ga));
	if (rv < 0)
		return rv;
		
	if (g_beos_r5_compatibility) {
		convert_to_beos_r5_sockaddr(addr, &temp);
		*addrlen = sizeof(struct beosr5_sockaddr_in);
	}

	return ga.rv;
}

int listen(int sock, int backlog)
{
	struct listen_args la;
	int rv;
	
	la.backlog = backlog;
	
	la.rv = B_ERROR;
	rv = ioctl(sock, NET_SOCKET_LISTEN, &la, sizeof(la));
	return (rv < 0) ? rv : la.rv;
}

int accept(int sock, struct sockaddr * addr, int * addrlen)
{
	struct sockaddr temp;
	struct accept_args aa;
	int	rv;
	int	new_sock;
	void * cookie;

	new_sock = open(g_socket_driver_path, O_RDWR);
	if (new_sock < 0)
		return new_sock;
	
	// The network stack driver will need to know to which net_socket_cookie to
	// *bind* with the new accepted socket. He can't know himself find out 
	// the net_socket_cookie of our new_sock file descriptor, the just open() one...
	// So, here, we ask him the net_socket_cookie value for our fd... :-)
	rv = ioctl(new_sock, NET_SOCKET_GET_COOKIE, &cookie);
	if (rv < 0) {
		close(new_sock);
		return rv;
	}; 

	aa.cookie	= cookie; // this way driver can use the right fd/cookie for the new_sock!

	aa.name		= g_beos_r5_compatibility ? &temp : addr;
	aa.namelen	= g_beos_r5_compatibility ? sizeof(temp) : *addrlen;
	
	aa.rv = B_ERROR;
	rv = ioctl(sock, NET_SOCKET_ACCEPT, &aa, sizeof(aa));
	if (rv == 0)
		rv = aa.rv;
	
	if (rv < 0) {
		close(new_sock);
		return rv;
	};

	if (g_beos_r5_compatibility) {
		convert_to_beos_r5_sockaddr(addr, &temp);
		*addrlen = sizeof(struct beosr5_sockaddr_in);
	} else
		*addrlen = aa.namelen;

	return new_sock;
}

#ifdef CODEWARRIOR
	#pragma mark [Private routines]
#endif

/* 
 * Private routines
 * ----------------
 */

static void convert_from_beos_r5_sockaddr(struct sockaddr *_to, const struct sockaddr *_from)
{
	const struct beosr5_sockaddr_in *from = (struct beosr5_sockaddr_in *)_from;
	struct sockaddr_in *to = (struct sockaddr_in *)_to;
	memset(to, 0, sizeof(*to));
	to->sin_len = sizeof(*to);
	to->sin_family = from->sin_family;
	to->sin_port = from->sin_port;
	to->sin_addr.s_addr = from->sin_addr;
}

static void convert_to_beos_r5_sockaddr(struct sockaddr *_to, const struct sockaddr *_from)
{
	const struct sockaddr_in *from = (struct sockaddr_in *)_from;
	struct beosr5_sockaddr_in *to = (struct beosr5_sockaddr_in *)_to;
	memset(to, 0, sizeof(*to));
	to->sin_family = from->sin_family;
	to->sin_port = from->sin_port;
	to->sin_addr = from->sin_addr.s_addr;
}

static void	convert_from_beos_r5_sockopt(int * level, int * optnum)
{
	if (*level == 1)
		*level = SOL_SOCKET;
		
	switch (*optnum) {
	case 1: *optnum = SO_DEBUG; break;
	case 2: *optnum = SO_REUSEADDR; break;
	// case 3: *optnum = SO_NONBLOCK; break;
	case 4: *optnum = SO_REUSEPORT; break;
	// case 5: *optnum = SO_FIONREAD; break;
	};
}
