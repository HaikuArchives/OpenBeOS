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

#include "net_server/core_module.h"
#include "net_structures.h"

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

        /* XXX - remove me when debugging is no longer required! */
        set_dprintf_enabled(true);

        return B_OK;
}

/* init_driver()
 * called every time we're loaded.
 */
status_t init_driver (void)
{
	int rv = 0;
        /* do we need to do anything here? */
	set_dprintf_enabled(true);
        dprintf("socket: socket device driver - init called\n");
	rv = get_module(CORE_MODULE_PATH, (module_info **)&core);
	if (rv < 0) {
		dprintf("net_socket_open: core module not found! %d\n", rv);
		return B_ERROR;
	}

        return B_OK;
}

/* uninit_driver()
 * called every time the driver is unloaded
 */
void uninit_driver (void)
{
        dprintf("net_srv: uninit_driver: uninit_driver\n");
	put_module(CORE_MODULE_PATH);
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
	struct sock_ptr *sp;
	dprintf("net_socket_open\n");
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
	dprintf("net_socket_close\n");
	
	put_module(CORE_MODULE_PATH);
	return rv;
}

/* Resources are freed on the stack when we close... */
static status_t net_socket_free(void *cookie)
{
        dprintf("net_socket_free\n");
        return B_OK;
}

static status_t net_socket_control(void *cookie,
                                uint32 op,
                                void *data,
                                size_t len)
{
	dprintf("net_socket_control: \n");

	if (op == NET_SOCKET_CREATE) {
		struct socket_args *sa = (struct socket_args*)data;
		return core->socreate(sa->dom, cookie, sa->type, sa->prot);
	}

	return core->soo_ioctl(cookie, op, data);
}

static status_t net_socket_read(void *cookie,
                                off_t position,
                                void *buffer,
                                size_t *readlen)
{
        dprintf("net_socket_read\n");
        return B_OK;
}

static status_t net_socket_write(void *cookie,
                                 off_t position,
                                 const void *buffer,
                                 size_t *writelen)
{
        dprintf("net_socket_write\n");
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
        NULL, /* We don't implement the select hooks. */
        NULL,
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

