/* net_userstack_driver - This file implements a very simple socket driver
**		that is intended to act as an interface to the networking stack when
**		it is loaded in userland.
**		The communication is slow, and could probably be much better, but it's
**		working, and that should be enough for now.
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


//#include <OS.h>
#include <KernelExport.h>
#include <Drivers.h>

#include <string.h>
#include <malloc.h>

#include "net_stack_driver.h"
#include "userland_ipc.h"
#include "lock.h"
#include "sys/sockio.h"
#include "sys/socket.h"
#include "net/if.h"
#include "sys/select.h"


/* these are missing from KernelExport.h ... */
#define  B_SELECT_READ       1 
#define  B_SELECT_WRITE      2 
#define  B_SELECT_EXCEPTION  3 

extern void notify_select_event(selectsync *sync, uint32 ref); 

//*****************************************************/
// Debug output
//*****************************************************/

#define DEBUG_PREFIX "net_driver: "
#define __out dprintf

#define DEBUG
#ifdef DEBUG
	#define PRINT(x) { __out(DEBUG_PREFIX); __out x; }
	#define REPORT_ERROR(status) __out(DEBUG_PREFIX "%s:%ld: %s\n",__FUNCTION__,__LINE__,strerror(status));
	#define RETURN_ERROR(err) { status_t _status = err; if (_status < B_OK) REPORT_ERROR(_status); return _status;}
	#define FATAL(x) { __out(DEBUG_PREFIX); __out x; }
	#define INFORM(x) { __out(DEBUG_PREFIX); __out x; }
//	#define FUNCTION() __out(DEBUG_PREFIX "%s()\n",__FUNCTION__);
	#define FUNCTION_START(x) { __out(DEBUG_PREFIX "%s() ",__FUNCTION__); __out x; }
	#define FUNCTION() ;
//	#define FUNCTION_START(x) ;
	#define D(x) {x;};
#else
	#define PRINT(x) ;
	#define REPORT_ERROR(status) ;
	#define RETURN_ERROR(status) return status;
	#define FATAL(x) { __out(DEBUG_PREFIX); __out x; }
	#define INFORM(x) { __out(DEBUG_PREFIX); __out x; }
	#define FUNCTION() ;
	#define FUNCTION_START(x) ;
	#define D(x) ;
#endif


//*****************************************************/
// Structure definitions
//*****************************************************/

#ifndef DRIVER_NAME
#	define DRIVER_NAME 		"net_stack_driver"
#endif

/* wait one second when waiting on the stack */
#define STACK_TIMEOUT 1000000LL


typedef struct {
	port_id		localPort,port;
	area_id		area;

	sem_id		commandSemaphore;
	net_command *commands;
	int32		commandIndex,numCommands;
} net_stack_cookie;


//*****************************************************/
// Prototypes
//*****************************************************/

/* command queue */
static net_command *get_command(net_stack_cookie *cookie,int32 *_index);
static status_t execute_command(net_stack_cookie *cookie, int32 op, void *data, uint32 length);
static status_t init_connection(void **_cookie);
static void shutdown_connection(net_stack_cookie *cookie);

/* R5 compatibility / select */
static void r5_notify_select_event(selectsync * sync, uint32 ref);

/* device hooks */
static status_t net_stack_open(const char * name, uint32 flags, void ** cookie);
static status_t net_stack_close(void * cookie);
static status_t net_stack_free_cookie(void * cookie);
static status_t net_stack_control(void * cookie, uint32 msg,void * data, size_t datalen);
static status_t net_stack_read(void * cookie, off_t pos, void * data, size_t * datalen);
static status_t net_stack_write(void * cookie, off_t pos, const void * data, size_t * datalen);
static status_t net_stack_select(void *cookie, uint8 event, uint32 ref, selectsync *sync);
static status_t net_stack_deselect(void *cookie, uint8 event, selectsync *sync);


//*****************************************************/
// Global variables and kernel interface
//*****************************************************/

_EXPORT int32 api_version = B_CUR_DRIVER_API_VERSION;

const char *gDeviceNames[] = {
	NET_STACK_DRIVER_PATH,
	NULL
};

device_hooks gDriverHooks =
{
	net_stack_open,
	net_stack_close,
	net_stack_free_cookie,
	net_stack_control,
	net_stack_read,
	net_stack_write,
	net_stack_select,
	net_stack_deselect,
	NULL,               /* -> readv entry pint */
	NULL                /* writev entry point */
};

typedef void (*notify_select_event_function)(selectsync * sync, uint32 ref);
notify_select_event_function gNotifySelectEvent = notify_select_event;

port_id gStackPort = -1;


//*****************************************************/
// The Command Queue
//*****************************************************/


static net_command *
get_command(net_stack_cookie *cookie,int32 *_index)
{
	int32 index,count = 0;
	net_command *command;

	while (count < cookie->numCommands*2) {
		index = atomic_add(&cookie->commandIndex,1) & (cookie->numCommands - 1);
		command = cookie->commands + index;

		if (command->op == 0) {
			// command is free to use
			*_index = index;
			return command;
		}
		count++;
	}
	return NULL;
}


static status_t
get_area_from_address(net_area_info *info,void *data)
{
	area_info areaInfo;

	if (data == NULL)
		return B_OK;

	info->id = area_for(data);
	if (info->id < B_OK)
		return info->id;

	if (get_area_info(info->id,&areaInfo) != B_OK)
		return B_BAD_VALUE;

	info->offset = areaInfo.address;
	return B_OK;
}


static void
set_command_areas(net_command *command)
{
	void *data = command->data;
	if (data == NULL)
		return;

	if (get_area_from_address(&command->area[0],data) < B_OK)
		return;

	switch (command->op) {
		case NET_STACK_GETSOCKOPT:
		case NET_STACK_SETSOCKOPT:
		{
			struct sockopt_args *sockopt = (struct sockopt_args *)data;

			get_area_from_address(&command->area[1],sockopt->optval);
			break;
		}

		case NET_STACK_CONNECT:
		case NET_STACK_BIND:
		case NET_STACK_GETSOCKNAME:
		case NET_STACK_GETPEERNAME:
		{
			struct sockaddr_args *args = (struct sockaddr_args *)data;
			
			get_area_from_address(&command->area[1],args->addr);
			break;
		}

		case NET_STACK_RECV:
		case NET_STACK_SEND:
		{
			struct data_xfer_args *args = (struct data_xfer_args *)data;
			get_area_from_address(&command->area[1],args->data);
			get_area_from_address(&command->area[2],args->addr);
			break;
		}
		case NET_STACK_RECVFROM:
		case NET_STACK_SENDTO:
		{
			struct msghdr *mh = (struct msghdr *)data;
			get_area_from_address(&command->area[1],mh->msg_name);
			get_area_from_address(&command->area[2],mh->msg_iov);
			get_area_from_address(&command->area[3],mh->msg_control);
			break;
		}

		case NET_STACK_ACCEPT:
		{
			struct accept_args *args = (struct accept_args *)data;
			/* accept_args.cookie is always in the address space of the server */
			get_area_from_address(&command->area[1],args->addr);
			break;
		}

		case NET_STACK_SELECT:
		{
			struct select_args *args = (struct select_args *)data;
			/* shouldn't be even needed at all... */
			get_area_from_address(&command->area[1],args->sync);
			break;
		}

		case NET_STACK_SYSCTL:
		{
			struct sysctl_args *sysctl = (struct sysctl_args *)data;

			get_area_from_address(&command->area[1],sysctl->name);
			get_area_from_address(&command->area[2],sysctl->oldp);
			get_area_from_address(&command->area[3],sysctl->oldlenp);
			get_area_from_address(&command->area[4],sysctl->newp);
			break;
		}

		case OSIOCGIFCONF:
		case SIOCGIFCONF:
		{
			struct ifconf *ifc = (struct ifconf *)data;

			get_area_from_address(&command->area[1],ifc->ifc_buf);
			break;
		}
	}	
}


static status_t
execute_command(net_stack_cookie *cookie,int32 op,void *data,uint32 length)
{
	uint32 commandIndex;
	net_command *command = get_command(cookie,&commandIndex);
	int32 maxTries = 200;
	status_t status;
	ssize_t bytes;

	if (command == NULL) {
		FATAL(("execute: command queue is full\n"));
		return B_ERROR;
	}

	memset(command,0,sizeof(net_command));

	command->op = op;
	command->data = data;
	set_command_areas(command);

	bytes = write_port(cookie->port,commandIndex,NULL,0);
	if (bytes < B_OK) {
		FATAL(("execute %ld: couldn't contact stack (id = %ld): %s\n",op,cookie->port,strerror(bytes)));
		free(cookie);
		return bytes;
	}

	// if we're closing the connection, there is no need to
	// wait for a result
	if (op == NET_STACK_CLOSE)
		return B_OK;

	while (true) {
		// wait until we get the results back from our command
		if ((status = acquire_sem(cookie->commandSemaphore)) == B_OK) {
			if (command->op != 0) {
				if (--maxTries <= 0) {
					FATAL(("command is not freed after 200 tries!\n"));
					return B_ERROR;
				}
				release_sem(cookie->commandSemaphore);
				continue;
			}
			return command->result;
		}
		FATAL(("command couldn't be executed: %s\n",strerror(status)));
		return status;
	}
}


static status_t
init_connection(void **_cookie)
{
	net_connection connection;
	ssize_t bytes;
	int32 msg;

	net_stack_cookie *cookie = (net_stack_cookie *)malloc(sizeof(net_stack_cookie));
	if (cookie == NULL)
		return B_NO_MEMORY;

	// create a new port and get a connection from the stack

	cookie->localPort = create_port(CONNECTION_QUEUE_LENGTH,"net_driver connection");
	if (cookie->localPort < B_OK) {
		FATAL(("open: couldn't create port: %s\n",strerror(cookie->localPort)));
		free(cookie);
		return bytes;
	}

	bytes = write_port(gStackPort,NET_STACK_NEW_CONNECTION,&cookie->localPort,sizeof(port_id));
	if (bytes == B_BAD_PORT_ID) {
		gStackPort = find_port(NET_STACK_PORTNAME);
		PRINT(("try to get net_server's port id: %ld\n",gStackPort));
		bytes = write_port(gStackPort,NET_STACK_NEW_CONNECTION,&cookie->localPort,sizeof(port_id));
	}
	if (bytes < B_OK) {
		FATAL(("open: couldn't contact stack: %s\n",strerror(bytes)));
		delete_port(cookie->localPort);
		free(cookie);
		return bytes;
	}

	bytes = read_port_etc(cookie->localPort,&msg,&connection,sizeof(net_connection),B_TIMEOUT,STACK_TIMEOUT);
	if (bytes < B_OK) {
		FATAL(("open: didn't hear back from stack: %s\n",strerror(bytes)));
		delete_port(cookie->localPort);
		free(cookie);
		return bytes;
	}
	if (msg != NET_STACK_NEW_CONNECTION) {
		FATAL(("open: received wrong answer: %ld\n",msg));
		delete_port(cookie->localPort);
		free(cookie);
		return B_ERROR;
	}

	// set up communication channels
	
	// ToDo: close server connection if anything fails
	// -> could also be done by the server, if it doesn't receive NET_STACK_OPEN

	cookie->port = connection.port;
	cookie->commandSemaphore = connection.commandSemaphore;
	cookie->area = clone_area("net connection buffer",(void **)&cookie->commands,B_CLONE_ADDRESS,
			B_READ_AREA | B_WRITE_AREA,connection.area);
	if (cookie->area < B_OK) {
		FATAL(("could clone command queue: %s\n",strerror(cookie->area)));
		delete_port(cookie->localPort);
		free(cookie);
		return cookie->area;
	}
	cookie->numCommands = connection.numCommands;
	cookie->commandIndex = 0;

	*_cookie = cookie;
	return B_OK;
}


static void
shutdown_connection(net_stack_cookie *cookie)
{
	delete_area(cookie->area);
	delete_port(cookie->localPort);

	free(cookie);
}


//	#pragma mark -
//*****************************************************/
// R5 select compatibility
//*****************************************************/


static void r5_notify_select_event(selectsync *sync, uint32 ref)
{
	struct r5_selectsync *rss = (struct r5_selectsync *)sync;

	if (acquire_sem(rss->lock) != B_OK) {
		/* if we can't acquire the lock, it's that select() is done,
		   or whatever it can be, we can do anything more about it here...
		*/
		PRINT(("r5_notify_select_event(%p, %ld) done\n", sync, ref));
		return;
	};

	switch (ref & 0x0F) {
		case 1:
			if (rss->rbits) { 
				FD_SET((ref >> 8), rss->rbits);
				rss->nfd++;
			} 
			break;
		case 2: 
			if (rss->wbits) {
				FD_SET((ref >> 8), rss->wbits);
				rss->nfd++;
			} 
			break;
		case 3:
			if (rss->ebits) {
				FD_SET((ref >> 8), rss->ebits);
				rss->nfd++;
			}
			break;
	};

	release_sem(rss->wait); /* wakeup r5_select() */
	release_sem(rss->lock);
}


//	#pragma mark -
//*****************************************************/
// Device hooks 
//*****************************************************/


static status_t
net_stack_open(const char *name,uint32 flags,void **_cookie)
{
	net_stack_cookie *cookie;
	status_t status = init_connection(_cookie);
	if (status < B_OK)
		return status;

	cookie = *_cookie;

	status = execute_command(cookie,NET_STACK_OPEN,&flags,sizeof(uint32));
	if (status < B_OK)
		net_stack_free_cookie(cookie);

	return status;
}


static status_t
net_stack_close(void *_cookie)
{
	net_stack_cookie *cookie = (net_stack_cookie *)_cookie;
	if (cookie == NULL)
		return B_BAD_VALUE;

	// we don't care here if the stack isn't alive anymore -
	// from the kernel's point of view, the device can always
	// be closed
	execute_command(cookie,NET_STACK_CLOSE,NULL,0);
	return B_OK;
}


static status_t
net_stack_free_cookie(void *_cookie)
{
	net_stack_cookie *cookie = (net_stack_cookie *)_cookie;
	if (cookie == NULL)
		return B_BAD_VALUE;

	shutdown_connection(cookie);
	return B_OK;
}


static status_t
net_stack_control(void *_cookie, uint32 op, void *data, size_t length)
{
	net_stack_cookie *cookie = (net_stack_cookie *)_cookie;

	//FUNCTION_START(("cookie = %p, op = %lx, data = %p, length = %ld\n",cookie,op,data,length));

	if (cookie == NULL)
		return B_BAD_VALUE;

	// if we get this call via ioctl() we are obviously called from an
	// R5 compatible libnet
	if (op == NET_STACK_SELECT)
		gNotifySelectEvent = r5_notify_select_event;

	return execute_command(cookie,op,data,-1);
}


static status_t
net_stack_read(void *_cookie,off_t pos,void *buffer,size_t *length)
{
	struct data_xfer_args args;
	int status;

	memset(&args,0,sizeof(args));
	args.data = buffer;
	args.datalen = *length;

	status = execute_command(_cookie,NET_STACK_RECV,&args,sizeof(args));
	if (status > 0) {
		*length = status;
		return B_OK;
	}
	return status;
}


static status_t
net_stack_write(void *_cookie,off_t pos,const void *buffer,size_t *length)
{
	struct data_xfer_args args;
	int status;
	
	memset(&args,0,sizeof(args));
	args.data = (void *)buffer;
	args.datalen = *length;
	
	status = execute_command(_cookie,NET_STACK_SEND,&args,sizeof(args));
	if (status > 0) {
		*length = status;
		return B_OK;
	}
	return status;
}


static status_t
net_stack_select(void *_cookie,uint8 event,uint32 ref,selectsync *sync)
{
	struct select_args args;

	args.ref = ref;
	args.sync = sync;
	// event has to be added to the args

	//return execute_command(_cookie,NET_STACK_SELECT,&args,sizeof(args));
	return B_ERROR;
}


static status_t
net_stack_deselect(void *_cookie,uint8 event,selectsync *sync)
{
	//return execute_command(_cookie,NET_STACK_DESELECT,&args,sizeof(args));
	return B_ERROR;
}


//	#pragma mark -
//*****************************************************/
// Driver hooks
//*****************************************************/


_EXPORT status_t
init_hardware(void)
{
	return B_OK;
}


_EXPORT status_t
init_driver (void)
{
	FUNCTION();
	return B_OK;
}


_EXPORT void
uninit_driver (void)
{
	FUNCTION();
}


_EXPORT const char **
publish_devices()
{
	FUNCTION();
	return gDeviceNames;
}


_EXPORT device_hooks *
find_device(const char *deviceName)
{
	FUNCTION();
	return &gDriverHooks;
}

