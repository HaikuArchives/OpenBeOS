#ifndef NET_STACK_DRIVER_H
#define NET_STACK_DRIVER_H

#define NET_STACK_DRIVER  "net/stack"

enum {
  NET_IOCTL_BASE = 0xbe240000,	// no BONE conflict, please :-)
  NET_STACK_IOCTL_BASE = NET_IOCTL_BASE + 0x200,
};

enum {
  NET_STACK_IOCTL_MIN = NET_STACK_IOCTL_BASE,
  NET_STACK_ACCEPT,
  NET_STACK_BIND,
  NET_STACK_CONNECT,
  NET_STACK_GETPEER,
  NET_STACK_GETSOCK,
  NET_STACK_GETOPT,
  NET_STACK_LISTEN,
  NET_STACK_RECV,
  NET_STACK_RECVFROM,
  NET_STACK_SEND,
  NET_STACK_SENDTO,
  NET_STACK_SETOPT,
  NET_STACK_SHUTDOWN,
  NET_STACK_SOCKET,
  NET_STACK_GET_COOKIE,
  NET_STACK_SELECT,
  NET_STACK_IOCTL_MAX
} net_stack_ioctls;

typedef struct int_ioctl
{
  int     rc;
  int     value;
} int_ioctl;

typedef struct sockaddr_ioctl
{
  int       rc;
  int       len;
  struct sockaddr * addr;
} sockaddr_ioctl;

typedef struct sockopt_ioctl
{
  int   rc;
  int   level;
  int   option;
  void  *optval;
  int   optlen;
} sockopt_ioctl;

typedef struct data_xfer_ioctl
{
  int         rc;
  void        *data;
  size_t      datalen;
  int         flags;
  struct sockaddr   *addr;
  int         addrlen;
} data_xfer_ioctl;

typedef struct socket_ioctl
{
  int rc;
  int family;
  int type;
  int proto;
} socket_ioctl;

typedef struct accept_ioctl {
  int       rc;
  void      *cookie;
  int       len;
  struct sockaddr *addr;
} accept_ioctl;

typedef struct select_ioctl {
  int 				rc;
  int				nbits;
  struct fd_set * 	rbits;
  struct fd_set * 	wbits;
  struct fd_set * 	ebits;
  struct timeval * 	timeout;
} select_ioctl;

#endif /* NET_STACK_DRIVER_H */

