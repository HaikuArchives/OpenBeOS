/* net_stack_driver.c
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
#include <string.h>

// these are missing from KernelExport.h ...
#define  B_SELECT_READ       1 
#define  B_SELECT_WRITE      2 
#define  B_SELECT_EXCEPTION  3 

extern void notify_select_event(selectsync * sync, uint32 ref); 

#include <driver_settings.h>

#include "netinet/in_var.h"
#include "sys/protosw.h"
#include "core_module.h"
#include "net_stack_driver.h"
#include "sys/select.h"

#define SHOW_INSANE_DEBUGGING	(1)
#define SERIAL_DEBUGGING		(0)
#define STAY_LOADED				(1)	// Force the driver to stay loaded in memory

#ifdef CODEWARRIOR
	#pragma mark [Local definitions]
#endif

/*
 * Local definitions
 * -----------------
 */

#ifndef DRIVER_NAME
	#define DRIVER_NAME 		"net_stack_driver"
#endif

#ifndef LOGID
	#define LOGID DRIVER_NAME ": "
#endif
#ifndef WARN
	#define WARN "Warning: "
#endif
#ifndef ERR
	#define ERR "ERROR: "
#endif

/* the cookie we attach to each file descriptor opened on our driver entry */
typedef struct {
	void *		socket;		/* NULL before ioctl(fd, NET_STACK_SOCKET/_ACCEPT) */
	uint32		open_flags;	/* the open() flags (mostly for storing O_NONBLOCK mode) */
	struct {
		selectsync *	sync;
		uint32			ref;
	} selectinfo[3];
} net_stack_cookie;

#ifdef CODEWARRIOR
	#pragma mark [Local prototypes]
#endif

#if STAY_LOADED
	// to unload the driver, simply write UNLOAD_CMD to him:
	// $ echo stop > /dev/net/stack
	// As soon as last app, via libnet.so, stop using it, it will unload,
	// and in turn stop the stack and unload all kernel modules...
	#define UNLOAD_CMD	"stop"
#endif

/* Prototypes of device hooks functions */
static status_t net_stack_open(const char * name, uint32 flags, void ** cookie);
static status_t net_stack_close(void * cookie);
static status_t net_stack_free(void * cookie);
static status_t net_stack_control(void * cookie, uint32 msg,void * data, size_t datalen);
static status_t net_stack_read(void * cookie, off_t pos, void * data, size_t * datalen);
// static status_t net_stack_readv(void * cookie, off_t pos, const iovec * vec, size_t count, size_t * len);
static status_t net_stack_write(void * cookie, off_t pos, const void * data, size_t * datalen);
// static status_t net_stack_writev(void * cookie, off_t pos, const iovec * vec, size_t count, size_t * len);
static status_t net_stack_select(void *cookie, uint8 event, uint32 ref, selectsync *sync);
static status_t net_stack_deselect(void *cookie, uint8 event, selectsync *sync);

/* Privates prototypes */
static void on_socket_event(void * socket, uint32 event, void * cookie);

#if STAY_LOADED
	static status_t	keep_driver_loaded();
	static status_t	unload_driver();
#endif

#ifdef CODEWARRIOR
	#pragma mark [Global variables]
#endif

/*
 * Global variables
 * ----------------
 */

#if STAY_LOADED
	int	g_stay_loaded_fd = -1;
#endif

const char * g_device_name_list[] = {
        NET_STACK_DRIVER_PATH,
        NULL
};

device_hooks g_net_stack_driver_hooks =
{
	net_stack_open,		/* -> open entry point */
	net_stack_close,	/* -> close entry point */
	net_stack_free,		/* -> free entry point */
	net_stack_control,	/* -> control entry point */
	net_stack_read,		/* -> read entry point */
	net_stack_write,	/* -> write entry point */
	net_stack_select,	/* -> select entry point */
	net_stack_deselect,	/* -> deselect entry point */
	NULL,
	NULL
	// net_stack_readv,	/* -> readv */
	// net_stack_writev	/* -> writev */
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
	void * sfmptr;

#if SERIAL_DEBUGGING	
	int rv;

	/* XXX - switch on/off at top of file... */
	set_dprintf_enabled(true);
	rv = load_driver_symbols(DRIVER_NAME);
#endif

	/* get a pointer to the driver settings... */
	sfmptr = load_driver_settings(B_SAFEMODE_DRIVER_SETTINGS);

	/* only use the pointer if it's valid */
	if (sfmptr != NULL) {
		/* we got a pointer, now get setting... */
		safemode = get_driver_boolean_parameter(sfmptr, B_SAFEMODE_SAFE_MODE, false, false);
		/* now get rid of settings */
		unload_driver_settings(sfmptr);
	}
	if (safemode) {
		dprintf(LOGID WARN "init_hardware: declining offer to join the party.\n");
		return B_ERROR;
	}

	dprintf(LOGID "init_hardware done.\n");

	return B_OK;
}


/* init_driver()
 * called every time we're loaded.
 */
_EXPORT status_t init_driver (void)
{
	int rv = 0;

#if SHOW_INSANE_DEBUGGING
	dprintf(LOGID "init_driver, built %s %s\n", __DATE__, __TIME__);
#endif

	/* do we need to do anything here? */
	
	rv = get_module(CORE_MODULE_PATH, (module_info **) &core);
	if (rv < 0) {
		dprintf(LOGID ERR "Argh, can't load " CORE_MODULE_PATH " module: %d\n", rv);
		return rv;
	}
	
#if SHOW_INSANE_DEBUGGING
	dprintf(LOGID "init_driver: core = %p\n", core);
#endif

	// start the network stack
	core->start();

	return B_OK;
}

/* uninit_driver()
 * called every time the driver is unloaded
 */
_EXPORT void uninit_driver (void)
{
#if SHOW_INSANE_DEBUGGING
	dprintf(LOGID "uninit_driver\n");
#endif

	if (core) {
		// shutdown the network stack
		// core->stop();
		put_module(CORE_MODULE_PATH);
	};
}

/* publish_devices()
 * called to publish our device.
 */
_EXPORT const char ** publish_devices()
{
        return g_device_name_list;
}


_EXPORT device_hooks * find_device (const char* DeviceName)
{
        return &g_net_stack_driver_hooks;
}


#ifdef CODEWARRIOR
	#pragma mark [Device hooks]
#endif

/*
 * Device hooks
 * ------------
 */

/* the network stack functions - mainly just pass throughs... */
static status_t net_stack_open(const char * name,
                                uint32 flags,
                                void ** cookie)
{
	net_stack_cookie *	nsc;

	nsc = (net_stack_cookie *) malloc(sizeof(*nsc));
	if (!nsc)
		return B_NO_MEMORY;
	
	memset(nsc, 0, sizeof(*nsc));
	nsc->socket = NULL; /* the socket will be allocated in NET_STACK_SOCKET ioctl */
	nsc->open_flags = flags;

  	/* attach this new net_socket_cookie to file descriptor */
	*cookie = nsc; 

#if SHOW_INSANE_DEBUGGING
	dprintf(LOGID "net_stack_open(%s, %s%s) return this cookie: %p\n", name,
						( ((flags & O_RWMASK) == O_RDONLY) ? "O_RDONLY" :
						  ((flags & O_RWMASK) == O_WRONLY) ? "O_WRONLY" : "O_RDWR"),
						(flags & O_NONBLOCK) ? " O_NONBLOCK" : "",
						*cookie);
#endif

	return B_OK;
}


static status_t net_stack_close(void *cookie)
{
	net_stack_cookie * nsc = (net_stack_cookie *) cookie;

#if SHOW_INSANE_DEBUGGING
	dprintf(LOGID "net_stack_close(%p)\n", nsc);
#endif

	if (nsc->socket) {
		core->soclose(nsc->socket);
		nsc->socket = NULL;
	};

	return B_OK;
}


/* Resources are freed on the stack when we close... */
static status_t net_stack_free(void *cookie)
{
#if SHOW_INSANE_DEBUGGING
	dprintf(LOGID "net_stack_free(%p)\n", cookie);
#endif

	free(cookie);
	return B_OK;
}


static status_t net_stack_control(void *cookie, uint32 op, void * data, size_t len)
{
	net_stack_cookie *	nsc = (net_stack_cookie *) cookie;

#if SHOW_INSANE_DEBUGGING
	dprintf(LOGID "net_stack_control(%p, %ld, %p, %ld)\n", cookie, op, data, len);
#endif

#if STAY_LOADED
	keep_driver_loaded();
#endif

	switch (op) {
		case NET_STACK_SOCKET: {
			struct socket_args * args = (struct socket_args *) data;
			
			args->rv = core->initsocket(&nsc->socket);
			if (args->rv == 0)
				args->rv = core->socreate(args->family, nsc->socket, args->type, args->proto);
			/* This is where the open flags need to be addressed */
			return B_OK;
		}
		case NET_STACK_CONNECT: {
			struct sockaddr_args * args = (struct sockaddr_args *) data;
			// args->addr == sockaddr to connect to...
			args->rv = core->soconnect(nsc->socket, (caddr_t) args->addr, args->addrlen);
			/* restore original settings... */
			return B_OK;
		}
		case NET_STACK_BIND: {
			struct sockaddr_args * args = (struct sockaddr_args *) data;
			// args->addr == sockaddr to bind
			args->rv = core->sobind(nsc->socket, (caddr_t) args->addr, args->addrlen);
			return B_OK;
		}
		case NET_STACK_LISTEN: {
			struct int_args * args = (struct int_args *) data;
			// args->value == backlog
			args->rv = core->solisten(nsc->socket, args->value);
			return B_OK;
		}
		case NET_STACK_GET_COOKIE: {
			// this is needed by accept() call, to be able to pass back
			// in NET_STACK_ACCEPT opcode the cookie of the filedescriptor to 
			// use for the new accepted socket
			*((void **) data) = cookie;
			return B_OK;
		}
		case NET_STACK_ACCEPT: {
			struct accept_args * args = (struct accept_args *) data;
			net_stack_cookie * ansc = (net_stack_cookie *) args->cookie;
			// args->cookie == net_stack_cookie * of the already opened fd to use to the 
			// newly accepted socket
			args->rv = core->soaccept(nsc->socket, &ansc->socket, args->addr, &args->addrlen);
			return B_OK;
		}
		case NET_STACK_SEND: {
			struct data_xfer_args * args = (struct data_xfer_args *) data;
			/* flags gets ignored here... */
			args->rv = net_stack_write(cookie, 0, args->data, &args->datalen);
			return B_OK;
		}
		case NET_STACK_RECV: {
			struct data_xfer_args * args = (struct data_xfer_args *) data;
			/* flags gets ignored here... */
			args->rv = net_stack_read(cookie, 0, args->data, &args->datalen);
			return B_OK;;
		}
		case NET_STACK_RECVFROM: {
			struct msghdr * mh = (struct msghdr *) data;
			int retsize, error;

			error = core->recvit(nsc->socket, mh, (caddr_t)&mh->msg_namelen, 
			                     &retsize);
			if (error == 0)
				return retsize;
			return error;
		}
		case NET_STACK_SENDTO: {
			struct msghdr * mh = (struct msghdr *) data;
			int retsize, error;

			error = core->sendit(nsc->socket, mh, mh->msg_flags, 
			                     &retsize);
			if (error == 0)
				return retsize;
			return error;
		}
		case NET_STACK_SYSCTL: {
			struct sysctl_args * args = (struct sysctl_args *) data;
			args->rv = core->net_sysctl(args->name, args->namelen,
			                          args->oldp, args->oldlenp,
			                          args->newp, args->newlen);
			return B_OK;
		}
		case NET_STACK_GETSOCKOPT: {
			struct sockopt_args * args = (struct sockopt_args *) data;
			
			args->rv = core->sogetopt(nsc->socket, args->level, args->option,
			                        args->optval, (size_t *) &args->optlen);
			return B_OK;
		}
		case NET_STACK_SETSOCKOPT: {
			struct sockopt_args * args = (struct sockopt_args *)data;
			
			args->rv = core->sosetopt(nsc->socket, args->level, args->option,
			                        (const void *) args->optval, args->optlen);
			return B_OK;
		}		

		case NET_STACK_GETSOCKNAME: {
			struct sockaddr_args * args = (struct sockaddr_args *) data;
			// args->addr == sockaddr to copy into sockname
			args->rv = core->sogetsockname(nsc->socket, args->addr, &args->addrlen);
			return B_OK;
		}
		case NET_STACK_GETPEERNAME: {
			struct sockaddr_args * args = (struct sockaddr_args *) data;
			// args->addr == sockaddr to copy into peername
			args->rv = core->sogetpeername(nsc->socket, args->addr, &args->addrlen);
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

		case NET_STACK_SELECT:
			{
			struct select_args * args = (struct select_args *) data;
			args->rv = select(args->nbits, args->rbits, args->wbits, args->ebits, args->timeout);
			dprintf("kernel select returned %d\n", args->rv);
			return B_OK;
			};
			
		default:
			return core->soo_ioctl(nsc->socket, op, data);
	}
}


static status_t net_stack_read(void *cookie,
                                off_t position,
                                void *buffer,
                                size_t *readlen)
{
	net_stack_cookie *	nsc = (net_stack_cookie *) cookie;
	struct iovec iov;
	int error;
	int flags;

#if SHOW_INSANE_DEBUGGING
	dprintf(LOGID "net_stack_read(%p, %Ld, %p, %ld)\n", cookie, position, buffer, *readlen);
#endif

#if STAY_LOADED
	dprintf(LOGID "Calling keep_driver_loaded()...\n");
	keep_driver_loaded();
#endif

	if (! nsc->socket)
		return B_BAD_VALUE;
	
	iov.iov_base = buffer;
	iov.iov_len = *readlen;
	
	error = core->readit(nsc->socket, &iov, &flags);
	*readlen = error;
    return error;
}


static status_t net_stack_write(void *cookie,
                                 off_t position,
                                 const void *buffer,
                                 size_t *writelen)
{
	net_stack_cookie *	nsc = (net_stack_cookie *) cookie;
	struct iovec iov;
	int error;

#if SHOW_INSANE_DEBUGGING
	dprintf(LOGID "net_stack_write(%p, %Ld, %p, %ld)\n", cookie, position, buffer, *writelen);
#endif

#if STAY_LOADED
	keep_driver_loaded();
#endif

	if (! nsc->socket) {
#if STAY_LOADED
		if (*writelen >= strlen(UNLOAD_CMD) &&
		    strncmp(buffer, UNLOAD_CMD, strlen(UNLOAD_CMD)) == 0)
			return unload_driver();
#endif
		return B_BAD_VALUE;
	};

	iov.iov_base = (void*)buffer;
	iov.iov_len = *writelen;
	
	error = core->writeit(nsc->socket, &iov, 0);
	dprintf("writeit gave %d\n", error);
	*writelen = error;
	return error;	
}


static status_t net_stack_select(void * cookie, uint8 event, uint32 ref, selectsync * sync)
{
	net_stack_cookie *	nsc = (net_stack_cookie *) cookie;

#if SHOW_INSANE_DEBUGGING
	dprintf(LOGID "net_stack_select(%p, %d, %ld, %p)\n", cookie, event, ref, sync);
#endif
	
	if (! nsc->socket)
		return B_BAD_VALUE;
	
	nsc->selectinfo[event - 1].sync = sync;
	nsc->selectinfo[event - 1].ref = ref;

	/* start (or continue) to monitor for socket event */
	return core->set_socket_event_callback(nsc->socket, on_socket_event, nsc);
}


static status_t net_stack_deselect(void * cookie, uint8 event, selectsync * sync)
{
	net_stack_cookie *	nsc = (net_stack_cookie *) cookie;
	int i;

#if SHOW_INSANE_DEBUGGING
	dprintf(LOGID "net_stack_deselect(%p, %d, %p)\n", cookie, event, sync);
#endif
	
	if (! nsc->socket)
		return B_BAD_VALUE;

	nsc->selectinfo[event - 1].sync = NULL;
	nsc->selectinfo[event - 1].ref = 0;
	
	for (i = 0; i < 3; i++) {
		if (nsc->selectinfo[i].sync)
			return B_OK;	/* still one (or more) socket's event to monitor */
	};

	/* no need to monitor socket events anymore */
	return core->set_socket_event_callback(nsc->socket, NULL, NULL);
}

#ifdef CODEWARRIOR
	#pragma mark [Private routines]
#endif

/* 
 * Private routines
 * ----------------
 */

static void on_socket_event(void *socket, uint32 event, void *cookie)
{
	net_stack_cookie * nsc = (net_stack_cookie *) cookie;
	if (! nsc)
		return;

#if SHOW_INSANE_DEBUGGING
	dprintf(LOGID "on_socket_event(%p, %ld, %p)\n", socket, event, cookie);
#endif
	
	/* BEWARE: We assert there that socket 'event' values are
	 * in fact 'B_SELECT_XXXX' ones [1..3]
	 */
	event--;
	if (nsc->selectinfo[event].sync)
		notify_select_event(nsc->selectinfo[event].sync, nsc->selectinfo[event].ref);
}

#if STAY_LOADED
static status_t	keep_driver_loaded()
{
	if ( g_stay_loaded_fd != -1 )
		return B_OK;	// already did
		
	// force the driver to stay loaded by opening himself
#if SHOW_INSANE_DEBUGGING
	dprintf(LOGID "keep_driver_loaded: internaly opening /dev/" NET_STACK_DRIVER_PATH " to stay loaded in memory...\n");
#endif

	g_stay_loaded_fd = open("/dev/" NET_STACK_DRIVER_PATH, 0);

#if SHOW_INSANE_DEBUGGING
	if (g_stay_loaded_fd < 0)
		dprintf(LOGID ERR "keep_driver_loaded: couldn't open(/dev/" NET_STACK_DRIVER_PATH")!\n");
#endif

	return B_OK;
}


static status_t unload_driver()
{
	if ( g_stay_loaded_fd >= 0 )
		{
		int tmp_fd;
			
		// we need to set g_stay_loaded_fd to < 0 if we don't want
		// the next close enter again in this case, and so on...
#if SHOW_INSANE_DEBUGGING
		dprintf(LOGID "unload_driver: unload requested.\n");
#endif
		tmp_fd = g_stay_loaded_fd;
		g_stay_loaded_fd = -1;

		close(tmp_fd);
		};
			
	return B_OK;
}
#endif // #if STAY_LOADED


#endif /* _KERNEL_MODE */

