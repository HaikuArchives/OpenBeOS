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
#include "net/if.h"
//#include "netinet/in_var.h"
//#include "sys/protosw.h"
//#include "sys/select.h"


/* these are missing from KernelExport.h ... */
//#define  B_SELECT_READ       1 
//#define  B_SELECT_WRITE      2 
//#define  B_SELECT_EXCEPTION  3 

//extern void notify_select_event(selectsync * sync, uint32 ref); 

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

	benaphore	bufferLock;
	uint8		*buffer;
	int32		bufferOffset,bufferSize;

	sem_id		commandSemaphore;
	net_command *commands;
	int32		commandIndex,numCommands;
} net_stack_cookie;


//*****************************************************/
// Prototypes
//*****************************************************/

static net_command *get_command(net_stack_cookie *cookie,int32 *_index);
static char *get_buffer(net_stack_cookie *cookie,uint32 size,int32 *_offset);
static status_t execute_command(net_stack_cookie *cookie, int32 op, void *data, uint32 length);
static status_t init_connection(void **_cookie);
static void shutdown_connection(net_stack_cookie *cookie);

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

port_id gStackPort = -1;


//*****************************************************/
// The Command Queue
//*****************************************************/


static int32
get_length(int32 op,uint8 *buffer)
{
	// this is *so* ugly, but BeOS' ioctl() doesn't know the size
	// of the data - the length parameter is just set to zero, so
	// we actually have to know here what we are doing later, which
	// is really unfortunate.
	switch (op) {
		case NET_STACK_LISTEN:
		case NET_STACK_SHUTDOWN:
			return sizeof(struct int_args);
		case NET_STACK_SOCKET:
			return sizeof(struct socket_args);
		case NET_STACK_CONNECT:
		case NET_STACK_BIND:
		case NET_STACK_GETSOCKNAME:
		case NET_STACK_GETPEERNAME:
			return sizeof(struct sockaddr_args) + ((struct sockaddr_args *)buffer)->addrlen;
		case NET_STACK_SETSOCKOPT:
		case NET_STACK_GETSOCKOPT:
			return sizeof(struct sockopt_args) + ((struct sockopt_args *)buffer)->optlen;
		case NET_STACK_ACCEPT:
			return sizeof(struct accept_args) + ((struct accept_args *)buffer)->addrlen;
		case NET_STACK_SEND:
		case NET_STACK_SENDTO:
		case NET_STACK_RECV:
		case NET_STACK_RECVFROM:
			return sizeof(struct data_xfer_args) + ((struct data_xfer_args *)buffer)->datalen
				+ ((struct data_xfer_args *)buffer)->addrlen;
		case NET_STACK_SELECT:
			return sizeof(struct select_args) + 3*sizeof(struct fd_set) + sizeof(struct timeval);
//		case NET_STACK_SYSCTL:
//			return sizeof(struct sysctl_args) + ((struct sysctl_args *)buffer)->namelen
//				+ ((struct sysctl_args *)buffer)->oldlenp + ((struct sysctl_args *)buffer)->newlen;

		case OSIOCGIFCONF:
		case SIOCGIFCONF:
			return sizeof(struct ifconf) + ((struct ifconf *)buffer)->ifc_len;

		default:
			if (IOCGROUP(op) == 'i' || IOCGROUP(op) == 'f')
				return IOCPARM_MASK & (op >> 16);
			PRINT(("unhandled ioctl op: %lx\n",op));
			return 0;
	}
}


static void
copy_to_stack(int32 op,uint8 *from,uint8 *to,int32 length)
{
	switch (op) {
		case NET_STACK_GETSOCKOPT:
		case NET_STACK_SETSOCKOPT:
		{
			struct sockopt_args *sockopt = (struct sockopt_args *)from;
			void *opt = to + sizeof(struct sockopt_args);

			memcpy(to,from,sizeof(struct sockopt_args));

			if (sockopt->optlen > 0) {
				memcpy(opt,sockopt->optval,sockopt->optlen);
				((struct sockopt_args *)to)->optval = opt;
			}
			break;
		}
		
		case NET_STACK_SYSCTL:
		{
			struct sysctl_args *fromSys = (struct sysctl_args *)from;
			struct sysctl_args *toSys = (struct sysctl_args *)to;
			void *name = to + sizeof(struct sysctl_args);
			void *oldp = (uint8 *)name + fromSys->namelen;
			size_t *oldlenp = (size_t *)((uint8 *)oldp + *fromSys->oldlenp);
			void *newp = (uint8 *)oldlenp + sizeof(size_t);

			memset(toSys,0,sizeof(struct sysctl_args));

			toSys->namelen = fromSys->namelen;
			if (toSys->namelen > 0) {
				memcpy(name,fromSys->name,fromSys->namelen);
				toSys->name = name;
			}
			
			toSys->oldlenp = oldlenp;
			*oldlenp = *fromSys->oldlenp;
			if (*oldlenp > 0 && fromSys->oldp) {
				memcpy(oldp,fromSys->oldp,*oldlenp);
				toSys->oldp = oldp;
			}
			
			toSys->newlen = fromSys->newlen;
			if (toSys->namelen > 0 && fromSys->newp) {
				memcpy(newp,fromSys->newp,fromSys->newlen);
				toSys->newp = newp;
			}
			break;
		}

		case OSIOCGIFCONF:
		case SIOCGIFCONF:
		{
			struct ifconf *ifc = (struct ifconf *)from;
			struct ifconf *ifc_to = (struct ifconf *)to;
			void *buffer = to + sizeof(struct ifconf);
			
			ifc_to->ifc_len = ifc->ifc_len;

			if (ifc->ifc_len > 0) {
				memcpy(buffer,ifc->ifc_buf,ifc->ifc_len);
				ifc_to->ifc_buf = buffer;
			}
			break;
		}

		case NET_STACK_LISTEN:
		case NET_STACK_SHUTDOWN:
		case NET_STACK_SOCKET:
		default:
			memcpy(to,from,length);
	}
}


static void
copy_from_stack(int32 op,uint8 *from,uint8 *to,int32 length)
{
	switch (op) {
		case NET_STACK_SETSOCKOPT:
		case NET_STACK_GETSOCKOPT:
		{
			struct sockopt_args *sockopt = (struct sockopt_args *)from;
			void *opt = from + sizeof(struct sockopt_args);
			void *oldOptval = ((struct sockopt_args *)to)->optval;

			memcpy(to,from,sizeof(struct sockopt_args));
			((struct sockopt_args *)to)->optval = oldOptval;

			if (sockopt->optlen > 0)
				memcpy(oldOptval,opt,sockopt->optlen);
			break;
		}
		case NET_STACK_SEND:
		case NET_STACK_SENDTO:
			break;

		case OSIOCGIFCONF:
		case SIOCGIFCONF:
		{
			struct ifconf *ifc = (struct ifconf *)from;
			struct ifconf *ifc_to = (struct ifconf *)to;

			PRINT(("first name %p = %s\n",ifc->ifc_req,ifc->ifc_req->ifr_name));
			ifc_to->ifc_len = ifc->ifc_len;
			if (ifc->ifc_len > 0)
				memcpy(ifc_to->ifc_buf,ifc->ifc_buf,ifc->ifc_len);
			break;
		}

		default:
			if ((IOCGROUP(op) == 'i' || IOCGROUP(op) == 'f') && (op & IOC_INOUT) == IOC_IN)
				break;
			memcpy(to,from,length);
	}
}


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


static char *
get_buffer(net_stack_cookie *cookie,uint32 size,int32 *_offset)
{
	// ToDo: this can currently overwrite memory which is still used

	uint8 *buffer;

	if (size > cookie->bufferSize)
		return NULL;

	if (ACQUIRE_BENAPHORE(cookie->bufferLock) < B_OK)
		return NULL;

PRINT(("buffer = %p,bufferOffset = %ld, bufferSize = %ld, size = %ld\n",cookie->buffer,cookie->bufferOffset,cookie->bufferSize,size));
	if (cookie->bufferOffset + size > cookie->bufferSize) {
		PRINT(("buffer overflow, start from 0\n"));
		buffer = cookie->buffer;
		*_offset = 0;
		cookie->bufferOffset = size;
	} else {
		buffer = cookie->buffer + cookie->bufferOffset;
		*_offset = cookie->bufferOffset;
		cookie->bufferOffset += size;
	}
PRINT((" -> offset = %ld, targetBuffer = %p, newOffset = %ld\n",*_offset,buffer,cookie->bufferOffset));

	RELEASE_BENAPHORE(cookie->bufferLock);
	return buffer;
}


static status_t
execute_command(net_stack_cookie *cookie,int32 op,void *data,uint32 length)
{
	uint32 commandIndex;
	net_command *command = get_command(cookie,&commandIndex);
	uint8 *buffer;
	status_t status;
	ssize_t bytes;

	if (command == NULL) {
		FATAL(("execute: command queue is full\n"));
		return B_ERROR;
	}

	if (length == -1) {
		length = get_length(op,data);
		PRINT(("****** got length: %ld\n",length));
	}

	command->op = op;
	command->buffer = 0;
	command->length = length;
	command->result = 0;

	if (length > 0) {
		buffer = get_buffer(cookie,length,&command->buffer);
		if (buffer == NULL)
			return B_NO_MEMORY;

		copy_to_stack(op,data,buffer,length);	
	}
	PRINT(("send command (%ld): op = %lx, buffer = %ld, length = %ld\n",commandIndex,command->op,command->buffer,command->length));

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
				release_sem(cookie->commandSemaphore);
				continue;
			}
			// copy the commands data back
			if (length)
				copy_from_stack(op,buffer,data,length);

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

	INIT_BENAPHORE(cookie->bufferLock,"net connection buffer");
	if (CHECK_BENAPHORE(cookie->bufferLock) < B_OK) {
		FATAL(("open: couldn't create semaphore: %s\n",strerror(cookie->bufferLock.sem)));
		free(cookie);
		return CHECK_BENAPHORE(cookie->bufferLock);
	}

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

	cookie->buffer = (uint8 *)cookie->commands + CONNECTION_COMMAND_SIZE;
	cookie->bufferSize = connection.bufferSize;
	cookie->bufferOffset = 0;

	*_cookie = cookie;
	return B_OK;
}


static void
shutdown_connection(net_stack_cookie *cookie)
{
	UNINIT_BENAPHORE(cookie->bufferLock);
	delete_area(cookie->area);
	delete_port(cookie->localPort);

	free(cookie);
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
	PRINT(("Connection established!\n"));

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

	FUNCTION_START(("cookie = %p, op = %lx, data = %p, length = %ld\n",cookie,op,data,length));

	if (cookie == NULL)
		return B_BAD_VALUE;

	return execute_command(cookie,op,data,-1);
}


static status_t
net_stack_read(void *_cookie,off_t pos,void *buffer,size_t *length)
{
	return B_ERROR;
}


static status_t
net_stack_write(void *_cookie,off_t pos,const void *buffer,size_t *length)
{
	return B_ERROR;
}


static status_t
net_stack_select(void *_cookie,uint8 event,uint32 ref,selectsync *sync)
{
	return B_ERROR;
}


static status_t
net_stack_deselect(void *_cookie,uint8 event,selectsync *sync)
{
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

