#include <unistd.h>
#include <fcntl.h>

#include "net/stack_driver.h"

const char * g_stack_driver_path = "/dev/" NET_STACK_DRIVER;

// ------------------------------------
int socket(int family, int type, int proto)
{
	int				sock;
	int				tmp;
	socket_ioctl	ctl;
	
	sock = open(g_stack_driver_path, O_RDWR);
	if (sock < 0)
		return sock;
	
	ctl.family 	= family;
	ctl.type	= type;
	ctl.proto	= proto;
	
	tmp = ioctl(sock, NET_STACK_SOCKET, &ctl);
	if (tmp < 0) {
		close(sock);
		sock = tmp;
	};
		
	return sock;
}


// ------------------------------------
int shutdown(int sock, int direction)
{
	int_ioctl	ctl;
	
	ctl.value 	= direction;

	return ioctl(sock,  NET_STACK_SHUTDOWN, &ctl);
}


/* This one is there only for BeOS R5.0.x and previous backward compatibility... 
*/
// ------------------------------------
int closesocket(int sock)
{
	return close(sock);
}


// ------------------------------------
int bind(int sock, const struct sockaddr * addr, size_t addrlen)
{
	sockaddr_ioctl	ctl;
	
	ctl.len 	= addrlen;
	ctl.addr	= (struct sockaddr *) addr;
	
	return ioctl(sock,  NET_STACK_BIND, &ctl);
}


// ------------------------------------
int connect(int sock, const struct sockaddr * addr, int addrlen)
{
	sockaddr_ioctl	ctl;
	
	ctl.len 	= addrlen;
	ctl.addr	= (struct sockaddr *) addr;
	
	return ioctl(sock,  NET_STACK_CONNECT, &ctl);
}


// ------------------------------------
int listen(int sock, int backlog)
{
	int_ioctl	ctl;
	
	ctl.value 	= backlog;

	return ioctl(sock,  NET_STACK_LISTEN, &ctl);
}


// ------------------------------------
int accept(int sock, struct sockaddr * addr, int * addrlen)
{
	accept_ioctl	ctl;
	int				rc;
	int				new_sock;
	void *			cookie;

	new_sock = open(g_stack_driver_path, O_RDWR);
	if (new_sock < 0)
		return new_sock;
	
	// We help net stack driver accept ioctl() attach the
	// new accepted endpoint to a file descriptor, the just open() one...
	rc = ioctl(new_sock, NET_STACK_GET_COOKIE, &cookie);
	if (rc < 0) {
		close(new_sock);
		return rc;
	}; 
	
	ctl.cookie	= cookie;	// this way driver can use the right fd for the new socket!
	ctl.addr	= addr;
	ctl.len 	= *addrlen;
	rc = ioctl(sock,  NET_STACK_ACCEPT, &ctl);
	if (rc < 0) {
		close(new_sock);
		return rc;
	};
	
	return new_sock;
}


// ------------------------------------
ssize_t send(int sock, const void * data, size_t datalen, int flags)
{
	data_xfer_ioctl	ctl;
	
	ctl.data 	= (void *) data;
	ctl.datalen	= datalen;
	ctl.flags 	= flags;
	ctl.addr	= NULL;
	ctl.addrlen	= 0;
	
	return ioctl(sock, NET_STACK_SEND, &ctl);
}


// ------------------------------------
ssize_t sendto(int sock, const void * data, size_t datalen, int flags, const struct sockaddr * addr, int addrlen)
{
	data_xfer_ioctl	ctl;
	
	ctl.data 	= (void *) data;
	ctl.datalen	= datalen;
	ctl.flags 	= flags;
	ctl.addr	= (struct sockaddr *) addr;
	ctl.addrlen	= addrlen;
	
	return ioctl(sock, NET_STACK_SENDTO, &ctl);
}


// ------------------------------------
ssize_t recv(int sock, void * data, size_t datalen, int flags)
{
	data_xfer_ioctl	ctl;
	
	ctl.data 	= data;
	ctl.datalen	= datalen;
	ctl.flags 	= flags;
	ctl.addr	= NULL;
	ctl.addrlen	= 0;
	
	return ioctl(sock, NET_STACK_RECV, &ctl);
}


// ------------------------------------
ssize_t recvfrom(int sock, void * data, size_t datalen, int flags, struct sockaddr * addr, int * addrlen)
{
	data_xfer_ioctl	ctl;
	
	ctl.data 	= data;
	ctl.datalen	= datalen;
	ctl.flags 	= flags;
	ctl.addr	= addr;
	ctl.addrlen	= *addrlen;
	
	return ioctl(sock, NET_STACK_RECVFROM, &ctl);
}


// ------------------------------------
int   getpeername(int sock, struct sockaddr * addr, int * addrlen)
{
	sockaddr_ioctl	ctl;
	
	ctl.addr	= addr;
	ctl.len 	= *addrlen;
	
	return ioctl(sock,  NET_STACK_GETPEER, &ctl);
}


// ------------------------------------
int   getsockname(int sock, struct sockaddr * addr, int * addrlen)
{
	sockaddr_ioctl	ctl;
	
	ctl.addr	= addr;
	ctl.len 	= *addrlen;
	
	return ioctl(sock,  NET_STACK_GETSOCK, &ctl);
}


// ------------------------------------
int   getsockopt(int sock, int level, int option, void * optval, int * optlen)
{
	sockopt_ioctl	ctl;
	
	ctl.level	= level;
	ctl.option	= option;
	ctl.optval	= (void *) optval;
	ctl.optlen	= *optlen;
	
	return ioctl(sock,  NET_STACK_GETOPT, &ctl);
}


// ------------------------------------
int   setsockopt(int sock, int level, int option, const void * optval, int optlen)
{
	sockopt_ioctl	ctl;
	
	ctl.level	= level;
	ctl.option	= option;
	ctl.optval	= (void *) optval;
	ctl.optlen	= optlen;
	
	return ioctl(sock,  NET_STACK_SETOPT, &ctl);
}


