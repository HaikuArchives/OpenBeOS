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
	void		*readBuffer,*writeBuffer;
	int32		numReadBuffers,numWriteBuffers;

	sem_id		commandSemaphore;
	net_command *commands;
	int32		commandIndex,numCommands;
} net_stack_cookie;


//*****************************************************/
// Prototypes
//*****************************************************/

static status_t execute_command(net_stack_cookie *cookie, int32 op, void *data, uint32 length);

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
// The Command Pipeline
//*****************************************************/


static status_t
execute_command(net_stack_cookie *cookie,int32 op,void *data,uint32 length)
{
	uint32 commandIndex = atomic_add(&cookie->commandIndex,1) % cookie->numCommands;
	net_command *command = cookie->commands + commandIndex;
	status_t status;
	ssize_t bytes;

	if (command->op != 0) {
		FATAL(("execute: command queue is full\n"));
		return B_ERROR;
	}

	command->op = op;
	command->result = 0;

	bytes = write_port(cookie->port,commandIndex,data,length);
	if (bytes < B_OK) {
		FATAL(("execute %ld: couldn't contact stack (id = %ld): %s\n",op,cookie->port,strerror(bytes)));
		free(cookie);
		return bytes;
	}
	if (op == NET_STACK_CLOSE)
		return B_OK;

	while (true) {
		// wait until we get the results back from our command
		if ((status = acquire_sem(cookie->commandSemaphore)) == B_OK) {
			if (command->op != 0) {
				release_sem(cookie->commandSemaphore);
				continue;
			}
			return command->result;
		}
		FATAL(("command couldn't be executed: %s\n",strerror(status)));
		return status;
	}
}


//	#pragma mark -
//*****************************************************/
// Device hooks 
//*****************************************************/


static status_t
net_stack_open(const char *name,uint32 flags,void **_cookie)
{
	net_connection connection;
	status_t status;
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
	cookie->area = clone_area("net connection buffer",(void **)&cookie->commands,B_ANY_ADDRESS,
			B_READ_AREA | B_WRITE_AREA,connection.area);
	if (cookie->area < B_OK) {
		FATAL(("could clone command queue: %s\n",strerror(cookie->area)));
		delete_port(cookie->localPort);
		free(cookie);
		return cookie->area;
	}
	cookie->numCommands = connection.numCommands;
	cookie->commandIndex = 0;

	cookie->readBuffer = (uint8 *)cookie->commands + NET_BUFFER_SIZE;
	cookie->writeBuffer = (uint8 *)cookie->readBuffer + connection.numReadBuffers * NET_BUFFER_SIZE;
	cookie->numReadBuffers = connection.numReadBuffers;
	cookie->numWriteBuffers = connection.numWriteBuffers;

	PRINT(("Connection established!\n"));

	*_cookie = cookie;
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

	delete_area(cookie->area);
	delete_port(cookie->localPort);

	free(cookie);
	return B_OK;
}


static status_t
net_stack_control(void *_cookie, uint32 op, void * data, size_t length)
{
	net_stack_cookie *cookie = (net_stack_cookie *)_cookie;

	FUNCTION_START(("cookie = %p, op = %ld, data = %p, length = %ld\n",cookie,op,data,length));

	if (cookie == NULL)
		return B_BAD_VALUE;

	// ToDo: handle read/write stuff correctly
	// most commands should work directly

	return execute_command(cookie,op,data,length);
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

