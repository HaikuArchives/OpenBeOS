/* socket.c
 *
 * This file implements a very simple socket driver that is intended to
 * act as an interface to the new networking stack.
 */

#ifndef _KERNEL_MODE

#error "This module MUST be built as a kernel driver!"

#else

#include <KernelExport.h>
#include <Drivers.h>
#include <driver_settings.h>

#include "netinet/in_var.h"
#include "sys/protosw.h"
#include "net_server/core_module.h"
#include "net_structures.h"
#include "sys/select.h"

#define SHOW_INSANE_DEBUGGING	0
#define SERIAL_DEBUGGING	1

_EXPORT int32 api_version = B_CUR_DRIVER_API_VERSION;

/* static variables... */
struct core_module_info *core = NULL;

const char *PublishedDeviceNames [] =
{
        "net/socket",
        NULL
};

/* Do we init ourselves? If we're in safe mode we'll decline so if things
 * screw up we can boot and delete the broken driver!
 * After my experiences earlier - a good idea!
 *
 * Also we'll turn on the serial debugger to aid in our debugging
 * experience.
 */
status_t init_hardware (void)
{
        bool safemode = false;
        void *sfmptr;
		int rv;

        /* get a pointer to the driver settings... */
        sfmptr = load_driver_settings(B_SAFEMODE_DRIVER_SETTINGS);

        /* only use the pointer if it's valid */
        if (sfmptr != NULL) {
                /* we got a pointer, now get setting... */
                safemode = get_driver_boolean_parameter(sfmptr,
                        B_SAFEMODE_SAFE_MODE, false, false);
                /* now get rid of settings */
                unload_driver_settings(sfmptr);
        }
        if (safemode) {
                dprintf("net_srv: init_hardware: declining offer to join the party.\n");
                return B_ERROR;
        }

#if SERIAL_DEBUGGING	
	/* XXX - switch on/off at top of file... */
        set_dprintf_enabled(true);
		rv = load_driver_symbols("net/socket");
		dprintf("load-driver_symbols gave %d\n", rv);
#endif

        return B_OK;
}

/* init_driver()
 * called every time we're loaded.
 */
status_t init_driver (void)
{
	int rv = 0;

#if SHOW_INSANE_DEBUGGING
	dprintf("socket_driver: init_driver\n");
#endif

	/* do we need to do anything here? */
#if SERIAL_DEBUGGING
	/* switch on/off at top of file */
	/* XXX ??? - do we need this here? */
	set_dprintf_enabled(true);
	rv = load_driver_symbols("net/socket");
	dprintf("load-driver_symbols gave %d\n", rv);
#endif

	rv = get_module(CORE_MODULE_PATH, (module_info **)&core);
	if (rv < 0) {
		dprintf("net_socket_open: core module not found! %d\n", rv);
		return B_ERROR;
	}

	/* now we've got it opened and installed in memory, ask it
	 * to start the rest of the stack...
	 */
	core->start();

	return B_OK;
}

/* uninit_driver()
 * called every time the driver is unloaded
 */
void uninit_driver (void)
{
#if SHOW_INSANE_DEBUGGING
	dprintf("socket_driver: uninit_driver\n");
#endif
	/* shutdown core server */
}

/* publish_devices()
 * called to publish our device.
 */
const char** publish_devices()
{
        return PublishedDeviceNames;
}

/* the network stack functions - mainly just pass throughs... */

/* we try to get a handle to the core module here. If we're already in memory
 * i.e. someone has a socket open, then we can skip this step. The first time
 * get_module is called the core module will do it's init and will then stay in
 * memory, so repeated calls to get_module just change the reference count, which
 * shouldn't be a big drain on resources/performance.
 */
static status_t net_socket_open(const char *devName,
                                uint32 flags,
                                void **cpp)
{
	int rv;

#if SHOW_INSANE_DEBUGGING
	dprintf("socket_driver: net_socket_open!\n");
#endif

	if (!core) {
		rv = get_module(CORE_MODULE_PATH, (module_info **)&core);	
		if (rv < 0) {
			dprintf("net_socket_open: core module not found! %d\n", rv);
			return B_ERROR;
		}
	}

	rv = core->initsocket(cpp);
	if (rv != 0)
		return rv;

	return B_OK;
}

static status_t net_socket_close(void *cookie)
{
	int rv = core->soclose(cookie);
#if SHOW_INSANE_DEBUGGING
	dprintf("socket_driver: net_socket_close\n");
#endif

	return rv;
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
#if SHOW_INSANE_DEBUGGING
	dprintf("socket_driver: net_socket_control: \n");
#endif
dprintf("net_socket_control: op = %ld (%d)\n", op, NET_SOCKET_CREATE);
	switch (op) {
		case NET_SOCKET_CREATE: {
			struct socket_args *sa = (struct socket_args*)data;
			sa->rv = core->socreate(sa->dom, cookie, sa->type, sa->prot);
			return B_OK;
		}
		case NET_SOCKET_BIND: {
			struct bind_args *ba = (struct bind_args*)data;
			ba->rv = core->sobind(cookie, ba->data, ba->dlen);
			return B_OK;
		}
		case NET_SOCKET_LISTEN: {
			struct listen_args *la = (struct listen_args*)data;
			la->rv = core->solisten(cookie, la->backlog);
			return B_OK;
		}
		case NET_SOCKET_CONNECT: {
			struct connect_args *ca = (struct connect_args*)data;
			ca->rv = core->soconnect(cookie, ca->name, ca->namelen);
			return B_OK;
		}
		case NET_SOCKET_SELECT: {
			struct select_args *sa = (struct select_args *)data;
			int i;
			struct timeval tv;
			
		dprintf("NET_SOCKET_SELECT, mfd = %d\n", sa->mfd);	
			for (i=2; i < sa->mfd;i++) {
				dprintf("socket %d: ", i);
				if (sa->rbits && FD_ISSET(i, sa->rbits))
					dprintf(" read bit ");
				if (sa->wbits && FD_ISSET(i, sa->wbits))
					dprintf(" write bit ");
				if (sa->ebits && FD_ISSET(i, sa->ebits))
					dprintf(" except bit ");
					
				dprintf("\n");
			}
			
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			
			sa->rv = select(sa->mfd, sa->rbits, sa->wbits, sa->ebits, &tv);
			dprintf("kernel select returned %d\n", sa->rv);
			return B_OK;
		}
		case NET_SOCKET_RECVFROM:
		{
			struct msghdr *mh = (struct msghdr *)data;
			int retsize, error;

			error = core->recvit(cookie, mh, (caddr_t)&mh->msg_namelen, 
			                     &retsize);
			if (error == 0)
				return retsize;
			return error;
		}
		case NET_SOCKET_SENDTO: 
		{
			struct msghdr *mh = (struct msghdr *)data;
			int retsize, error;
			printf("NET_SOCKET_SENDTO\n");

			error = core->sendit(cookie, mh, mh->msg_flags, 
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
		default:
			return core->soo_ioctl(cookie, op, data);
	}
}

static status_t net_socket_read(void *cookie,
                                off_t position,
                                void *buffer,
                                size_t *readlen)
{
	struct iovec iov;
	int error;
	int flags;
	dprintf("net_socket_read\n");
	
	iov.iov_base = buffer;
	iov.iov_len = *readlen;
	
	error = core->readit(cookie, &iov, &flags);
	if (error < 0)
		return error;
	*readlen = error;

    return B_OK;
}

static status_t net_socket_write(void *cookie,
                                 off_t position,
                                 const void *buffer,
                                 size_t *writelen)
{
	struct iovec iov;
	int error;
	dprintf("net_socket_write\n");
	
	iov.iov_base = buffer;
	iov.iov_len = *writelen;
	
	error = core->writeit(cookie, &iov, 0);
	if (error < 0)
		return error;
	*writelen = error;
	return B_OK;	
}

static status_t net_socket_select(void *cookie, 
                                  uint8 event, 
                                  uint32 ref,
                                  selectsync *sync)
{
	dprintf("net_socket_select!\n");
	dprintf("\tevent = %d\n", event);
	dprintf("\tref = %ld\n", ref);;
	dprintf("\tsync = %p\n", sync);
	
	
	return B_OK;
}

static status_t net_socket_deselect(void *cookie, 
                                    uint8 event,
                                    selectsync *sync)
{
	dprintf("net_socket_deselect!\n");
	dprintf("event = %d\n", event);
	dprintf("sync = %p\n", sync);

	return B_OK;
}

device_hooks openbeos_net_server_hooks =
{
        net_socket_open,
        net_socket_close,
        net_socket_free,
        net_socket_control,
        net_socket_read,
        net_socket_write,
		net_socket_select,
		net_socket_deselect,
        NULL, /* Don't implement the scattered buffer read and write. */
        NULL
};


device_hooks* find_device (const char* DeviceName)
{
        int rv;

        if (DeviceName == NULL)
                return NULL;

        if ((rv = strcmp(DeviceName, PublishedDeviceNames[0])) != 0) {
                return NULL;
        }

        return &openbeos_net_server_hooks;
}

#endif /* _KERNEL_MODE */

