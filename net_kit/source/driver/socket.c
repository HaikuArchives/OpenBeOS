/* socket.c
 *
 * This file implements a very simple socket driver that is intended to
 * act as an interface to the new networking stack.
 */

#ifndef _KERNEL_MODE

#error "This module MUST be built as a kernel driver!"

#else

#include <OS.h>
#include <Drivers.h>
#include <KernelExport.h>

// these are missing from KernelExport.h ...
#define  B_SELECT_READ       1 
#define  B_SELECT_WRITE      2 
#define  B_SELECT_EXCEPTION  3 

extern void notify_select_event(selectsync * sync, uint32 ref); 

#include <driver_settings.h>

#include "netinet/in_var.h"
#include "sys/protosw.h"
#include "net_server/core_module.h"
#include "net_structures.h"
#include "sys/select.h"

struct be_sockaddr_in {
	uint8  sin_family;
	uint16 sin_port;
	uint32 sin_addr;
	char sin_zero[4];
};

/*
 * Local definitions
 * -----------------
 */
 
/* the cookie we attach to each file descriptor opened on our driver entry */
typedef struct {
	void *		socket;		/* NULL before ioctl(fd, NET_SOCKET_SOCKET/ACCEPT) */
	int         r5;         /* is it an R5 app ? */
	uint32		open_flags;	/* the open() flags (mostly for storing O_NONBLOCK mode) */
	struct {
		selectsync *	sync;
		uint32			ref;
	} selectinfo[3];
} net_socket_cookie;

#define SHOW_INSANE_DEBUGGING     0
#define SERIAL_DEBUGGING          1

/* Prototypes of device hooks functions */
static status_t net_socket_open(const char * name, uint32 flags, void ** cookie);
static status_t net_socket_close(void * cookie);
static status_t net_socket_free(void * cookie);
static status_t net_socket_control(void * cookie, uint32 msg,void * buf, size_t len);
static status_t net_socket_read(void * cookie, off_t pos, void * buf, size_t * len);
// static status_t net_socket_readv(void * cookie, off_t pos, const iovec * vec, size_t count, size_t * len);
static status_t net_socket_write(void * cookie, off_t pos, const void * buf, size_t * len);
// static status_t net_socket_writev(void * cookie, off_t pos, const iovec * vec, size_t count, size_t * len);
static status_t net_socket_select(void *cookie, uint8 event, uint32 ref, selectsync *sync);
static status_t net_socket_deselect(void *cookie, uint8 event, selectsync *sync);

/* Privates prototypes */
static void on_socket_event(void * socket, uint32 event, void * cookie);

/*
 * Global variables
 * ----------------
 */
 
const char * device_name_list[] = {
        "net/socket",
        NULL
};

device_hooks net_socket_driver_hooks =
{
        net_socket_open,
        net_socket_close,
        net_socket_free,
        net_socket_control,
        net_socket_read,
        net_socket_write,
		net_socket_select,
		net_socket_deselect,
        NULL, 
        NULL
};

struct core_module_info * core = NULL;

#ifdef CODEWARRIOR
	#pragma mark [Driver API calls]
#endif

/*
 * Driver API calls
 * ----------------
 */
 
_EXPORT int32 api_version = B_CUR_DRIVER_API_VERSION;


/* Do we init ourselves? If we're in safe mode we'll decline so if things
 * screw up we can boot and delete the broken driver!
 * After my experiences earlier - a good idea!
 *
 * Also we'll turn on the serial debugger to aid in our debugging
 * experience.
 */
_EXPORT status_t init_hardware (void)
{
	bool safemode = false;
	void *sfmptr;

#if SERIAL_DEBUGGING	
	int rv;

	/* XXX - switch on/off at top of file... */
	set_dprintf_enabled(true);
	rv = load_driver_symbols("socket");
#endif

	/* get a pointer to the driver settings... */
	sfmptr = load_driver_settings(B_SAFEMODE_DRIVER_SETTINGS);

	/* only use the pointer if it's valid */
	if (sfmptr != NULL) {
		/* we got a pointer, now get setting... */
		safemode = get_driver_boolean_parameter(sfmptr,
		                                        B_SAFEMODE_SAFE_MODE, 
		                                        false, false);
		/* now get rid of settings */
		unload_driver_settings(sfmptr);
	}
	if (safemode) {
		dprintf("net_srv: init_hardware: declining offer to join the party.\n");
		return B_ERROR;
	}

	dprintf("socket_driver: init_hardware - returning B_OK\n");

	return B_OK;
}


/* init_driver()
 * called every time we're loaded.
 */
_EXPORT status_t init_driver (void)
{
	int rv = 0;

#if SHOW_INSANE_DEBUGGING
	dprintf("socket_driver: init_driver\n");
#endif

	/* do we need to do anything here? */
	
	rv = get_module(CORE_MODULE_PATH, (module_info **)&core);
	if (rv < 0) {
		dprintf("net_socket_open: core module not found! %d\n", rv);
		return B_ERROR;
	}
	
#if SHOW_INSANE_DEBUGGING
	dprintf("socket_driver: init_driver: core = %p\n", core);
#endif
	core->start();

	return B_OK;
}

/* uninit_driver()
 * called every time the driver is unloaded
 */
_EXPORT void uninit_driver (void)
{
#if SHOW_INSANE_DEBUGGING
	dprintf("socket_driver: uninit_driver\n");
#endif

	/* shutdown core server */
	put_module(CORE_MODULE_PATH);
}

/* publish_devices()
 * called to publish our device.
 */
_EXPORT const char ** publish_devices()
{
        return device_name_list;
}


_EXPORT device_hooks * find_device (const char* DeviceName)
{
        return &net_socket_driver_hooks;
}


#ifdef CODEWARRIOR
	#pragma mark [Device hooks]
#endif

/*
 * Device hooks
 * ------------
 */

/* the network stack functions - mainly just pass throughs... */

/* we try to get a handle to the core module here. If we're already in memory
 * i.e. someone has a socket open, then we can skip this step. The first time
 * get_module is called the core module will do it's init and will then stay in
 * memory, so repeated calls to get_module just change the reference count, which
 * shouldn't be a big drain on resources/performance.
 */
static status_t net_socket_open(const char * name,
                                uint32 flags,
                                void ** cookie)
{
	net_socket_cookie *	nsc;

	nsc = (net_socket_cookie *) malloc(sizeof(*nsc));
	if (!nsc)
		return B_NO_MEMORY;
	
	memset(nsc, 0, sizeof(*nsc));
	nsc->socket = NULL; /* the socket will be allocated in NET_SOCKET_SOCKET ioctl */
	nsc->open_flags = flags;

  	/* attach this new net_socket_cookie to file descriptor */
	*cookie = nsc; 

#if SHOW_INSANE_DEBUGGING
	dprintf("socket_driver: net_socket_open(%s, %s%s) return this cookie: %p\n", name,
						( ((flags & O_RWMASK) == O_RDONLY) ? "O_RDONLY" :
						  ((flags & O_RWMASK) == O_WRONLY) ? "O_WRONLY" : "O_RDWR"),
						(flags & O_NONBLOCK) ? " O_NONBLOCK" : "",
						*cookie);
#endif

	return B_OK;
}


static status_t net_socket_close(void *cookie)
{
	net_socket_cookie * nsc = (net_socket_cookie *) cookie;

#if SHOW_INSANE_DEBUGGING
	dprintf("socket_driver: net_socket_close(%p)\n", nsc);
#endif
	if (nsc->socket)
		return core->soclose(nsc->socket);
	return B_OK;
}


/* Resources are freed on the stack when we close... */
static status_t net_socket_free(void *cookie)
{
#if SHOW_INSANE_DEBUGGING
	dprintf("socket_driver: net_socket_free\n");
#endif
	return B_OK;
}


static status_t net_socket_control(void *cookie,
                                uint32 op,
                                void *data,
                                size_t len)
{
	net_socket_cookie *	nsc = (net_socket_cookie *) cookie;

#if SHOW_INSANE_DEBUGGING
	dprintf("socket_driver: net_socket_control: \n");
#endif

	switch (op) {
		case NET_SOCKET_CREATE: {
			struct socket_args * sa = (struct socket_args*)data;
			nsc->r5 = sa->r5;
			
			sa->rv = core->initsocket(&nsc->socket);
			if (sa->rv != 0)
				return B_OK;

			sa->rv = core->socreate(sa->dom, nsc->socket, sa->type, sa->prot);
			/* This is where the open flags need to be addressed */
			return B_OK;
		}
		case NET_SOCKET_CONNECT: {
			struct connect_args * ca = (struct connect_args *) data;
			struct be_sockaddr_in * bsa;
			struct sockaddr_in sin;
			
			if (nsc->r5) {
				dprintf("BEOS R5 App!\n");
				bsa = (struct be_sockaddr_in*)ca->name;
				memset(&sin, 0, sizeof(sin));
				sin.sin_family = bsa->sin_family;
				sin.sin_port = bsa->sin_port;
				sin.sin_addr.s_addr = bsa->sin_addr;
				sin.sin_len = sizeof(sin);
dprintf("connect: sin: [%d] %08lx:%d\n", sin.sin_family, sin.sin_addr.s_addr, sin.sin_port);
				ca->name = (caddr_t)&sin;
				ca->namelen = sizeof(sin);
			}
			ca->rv = core->soconnect(nsc->socket, ca->name, ca->namelen);
			/* restore original settings... */
			if (nsc->r5) {
				ca->name = (caddr_t) bsa;
				ca->namelen = sizeof(struct be_sockaddr_in);
			}			
			return B_OK;
		}
		case NET_SOCKET_BIND: {
			struct bind_args * ba = (struct bind_args *) data;
			
			ba->rv = core->sobind(nsc->socket, ba->data, ba->dlen);
			return B_OK;
		}
		case NET_SOCKET_LISTEN: {
			struct listen_args * la = (struct listen_args *) data;
			la->rv = core->solisten(nsc->socket, la->backlog);
			return B_OK;
		}
		case NET_SOCKET_GET_COOKIE: {
			// this is needed by libnet.so accept() call, to be able to pass 
			// in NET_STACK_ACCEPT opcode the cookie of the filedescriptor to 
			// use for the new accepted socket
dprintf("net_stack: net_socket_control %p: NET_SOCKET_GET_COOKIE.\n", nsc);
			*((void **) data) = cookie;
			return B_OK;
		}
		case NET_SOCKET_ACCEPT: {
			struct accept_args * aa = (struct accept_args *) data;
			net_socket_cookie *	ansc = (net_socket_cookie *) aa->cookie;
			// aa->cookie == net_socket_cookie * of the already opened fd to use for the 
			// newly accepted socket
			// aa->rv = core->soaccept(nsc->socket, &ansc->socket, aa->name, &aa->namelen);
			return B_OK;
		}	
		case NET_SOCKET_RECVFROM:
		{
			struct msghdr *mh = (struct msghdr *)data;
			int retsize, error;

			error = core->recvit(nsc->socket, mh, (caddr_t)&mh->msg_namelen, 
			                     &retsize);
			if (error == 0)
				return retsize;
			return error;
		}
		case NET_SOCKET_SENDTO: 
		{
			struct msghdr *mh = (struct msghdr *)data;
			int retsize, error;

			error = core->sendit(nsc->socket, mh, mh->msg_flags, 
			                     &retsize);
			if (error == 0)
				return retsize;
			return error;
		}
		case NET_SOCKET_SYSCTL:
		{
			struct sysctl_args *sa = (struct sysctl_args*)data;

			sa->rv = core->net_sysctl(sa->name, sa->namelen,
			                          sa->oldp, sa->oldlenp,
			                          sa->newp, sa->newlen);
			return B_OK;
		}
		case NET_SOCKET_GETSOCKOPT: {
			struct getopt_args *ga = (struct getopt_args*)data;
			
			ga->rv = core->sogetopt(nsc->socket, ga->level, ga->optnum,
			                        ga->val, ga->valsize);
			return B_OK;
		}
		case NET_SOCKET_SETSOCKOPT: {
			struct setopt_args *sa = (struct setopt_args*)data;
			
			sa->rv = core->sosetopt(nsc->socket, sa->level, sa->optnum,
			                        sa->val, sa->valsize);
			return B_OK;
		}		

		case NET_SOCKET_GETSOCKNAME: {
			struct getname_args *ga = (struct getname_args*)data;

			ga->rv = core->sogetsockname(nsc->socket, ga->name, ga->namelen);
			return B_OK;
		}
		case NET_SOCKET_GETPEERNAME: {
			struct getname_args *ga = (struct getname_args*)data;

			ga->rv = core->sogetpeername(nsc->socket, ga->name, ga->namelen);
			return B_OK;
		}

/*
		case NET_STACK_RESTART:
			core->stop();
			return B_OK;
*/
		case B_SET_BLOCKING_IO:
			nsc->open_flags &= ~O_NONBLOCK;
			return B_OK;

		case B_SET_NONBLOCKING_IO:
			nsc->open_flags |= O_NONBLOCK;
			return B_OK;

		case NET_SOCKET_SELECT:
			{
			struct select_args *sa = (struct select_args *)data;
		
			sa->rv = select(sa->mfd, sa->rbits, sa->wbits, sa->ebits, sa->tv);
			dprintf("kernel select returned %d\n", sa->rv);
			return B_OK;
			};
			
		default:
			return core->soo_ioctl(nsc->socket, op, data);
	}
}


static status_t net_socket_read(void *cookie,
                                off_t position,
                                void *buffer,
                                size_t *readlen)
{
	net_socket_cookie *	nsc = (net_socket_cookie *) cookie;
	struct iovec iov;
	int error;
	int flags;

	if (! nsc->socket)
		return B_BAD_VALUE;
	
	iov.iov_base = buffer;
	iov.iov_len = *readlen;
	
	error = core->readit(nsc->socket, &iov, &flags);
	*readlen = error;
    return error;
}


static status_t net_socket_write(void *cookie,
                                 off_t position,
                                 const void *buffer,
                                 size_t *writelen)
{
	net_socket_cookie *	nsc = (net_socket_cookie *) cookie;
	struct iovec iov;
	int error;
	
	if (! nsc->socket)
		return B_BAD_VALUE;
	
	iov.iov_base = (void*)buffer;
	iov.iov_len = *writelen;
	
	error = core->writeit(nsc->socket, &iov, 0);
	dprintf("writeit gave %d\n", error);
	*writelen = error;
	return error;	
}


static status_t net_socket_select(void * cookie, 
                                  uint8 event, 
                                  uint32 ref,
                                  selectsync * sync)
{
	net_socket_cookie *	nsc = (net_socket_cookie *) cookie;

	dprintf("net_socket_select!\n");
	
	nsc->selectinfo[event - 1].sync = sync;
	nsc->selectinfo[event - 1].ref = ref;

	if (! nsc->socket)
		return B_BAD_VALUE;
	
	/* start (or continue) to monitor for socket event */
	return core->set_socket_event_callback(nsc->socket, on_socket_event, nsc);
}


static status_t net_socket_deselect(void * cookie, 
                                    uint8 event,
                                    selectsync * sync)
{
	net_socket_cookie *	nsc = (net_socket_cookie *) cookie;
	int i;

	dprintf("net_socket_deselect!\n");

	nsc->selectinfo[event - 1].sync = NULL;
	nsc->selectinfo[event - 1].ref = 0;
	
	if (! nsc->socket)
		return B_BAD_VALUE;
	
	for (i = 0; i < 3; i++) {
		if (nsc->selectinfo[i].sync)
			return B_OK;	/* still one (or more) socket's event to monitor */
	};

	/* no need to monitor socket events anymore */
	return core->set_socket_event_callback(nsc->socket, NULL, NULL);
}

#ifdef CODEWARRIOR
	#pragma mark [Privates routines]
#endif

/* 
 * Private routines
 * ----------------
 */
static void on_socket_event(void *socket, uint32 event, void *cookie)
{
	net_socket_cookie * nsc = (net_socket_cookie *) cookie;
	if (! nsc)
		return;

	/* BEWARE: We assert there that socket 'event' values are
	 * in fact 'B_SELECT_XXXX' ones
	 */
	if (nsc->selectinfo[event-1].sync)
		notify_select_event(nsc->selectinfo[event-1].sync, nsc->selectinfo[event-1].ref);
}

#endif /* _KERNEL_MODE */

