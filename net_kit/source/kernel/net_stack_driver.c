
// BeOS network stack driver

#include <stdlib.h>
#include <stdio.h>

#include <OS.h>
#include <Drivers.h>
#include <KernelExport.h>

// these are missing from KernelExport.h ...
#define  B_SELECT_READ       1 
#define  B_SELECT_WRITE      2 
#define  B_SELECT_EXCEPTION  3 
extern void notify_select_event(selectsync *sync, uint32 ref); 

#include "net/stack_driver.h"
#include "net/net_endpoint.h"
// #include "net/net_data.h"

#define DEBUG 1
#include <Debug.h>

#define LOAD_SYMBOLS		1
#define STAY_LOADED			0	// Force the driver to stay loaded in memory

#ifdef CODEWARRIOR
	#pragma mark [Local definitions]
#endif

// Local definitions
// -----------------

#ifndef DRIVER_NAME
	#define DRIVER_NAME 		"net_stack"
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

// #define DEBUGGER_COMMAND	 	DRIVER_NAME

#define DEVICE_NAME 			DRIVER_NAME
#define DEVICE_NAME_LENGTH		(64)			
#define DEVICE_PATH 			NET_STACK_DRIVER

#if STAY_LOADED
	// we publish a "unload" entry, sa that we could ask the driver to unload itself...
	// a simple 'echo now /dev/scanner/usb/unload' command for example would unload the driver...
	#define UNLOAD_DEVICE_PATH	"net/unload_stack"
	#define UNLOAD_COOKIE		(void *) 4
#endif

// Prototypes of device hooks functions
static status_t device_open(const char *name, uint32 flags, void ** cookie);
static status_t device_close(void * cookie);
static status_t device_free(void * cookie);
static status_t device_control(void * cookie, uint32 msg,void * buf, size_t len);
static status_t device_read(void * cookie, off_t pos, void * buf, size_t * len);
static status_t device_readv(void * cookie, off_t pos, const iovec * vec, size_t count, size_t * len);
static status_t device_write(void * cookie, off_t pos, const void * buf, size_t * len);
static status_t device_writev(void * cookie, off_t pos, const iovec * vec, size_t count, size_t * len);
static status_t device_select(void *cookie, uint8 event, uint32 ref, selectsync *sync);
static status_t device_deselect(void *cookie, uint8 event, selectsync *sync);

// Globals variables
// -----------------

// globals for driver instances
static char *	g_driver_name = DRIVER_NAME;

#if STAY_LOADED
	static int	g_stay_loaded_fd = -1;
#endif

static char *	g_device_name_list[] = { DEVICE_PATH, NULL };

/* And, so, the device hooks list */
static device_hooks g_device_hooks = {
	device_open, 		/* -> open entry point */
	device_close,         	/* -> close entry point */
	device_free,          	/* -> free entry point */
	device_control,       	/* -> control entry point */
	device_read,       	/* -> read entry point */
	device_write,        	/* -> write entry point */
	device_select,             /* -> select entry point */
	device_deselect,          /* -> deselect entry point */
	device_readv,              /* -> readv */
	device_writev              /* -> writev */
};

#ifdef CODEWARRIOR
	#pragma mark [Local prototypes]
#endif

// Privates prototypes
// -------------------

#if STAY_LOADED
	static status_t	stay_loaded();
	static status_t	unload_driver();
#endif

#ifdef DEBUGGER_COMMAND
	static int 		device_debugger_command(int argc, char ** argv);
#endif

#define ROUND_UP(x, y) (((x) + (y) - 1) & ~((y) - 1))


// OKAY, NOW IMPLEMENTATION PLEASE!
// --------------------------------

#ifdef CODEWARRIOR
	#pragma mark [Driver API calls]
#endif

// Driver API calls
// ----------------


_EXPORT int32 api_version = B_CUR_DRIVER_API_VERSION;


// -------------------------------------------------------
_EXPORT status_t init_hardware (void)
{
	DEBUG_ONLY( dprintf(LOGID "--- init_hardware\n") );
	return B_OK;
}


// -------------------------------------------------------
_EXPORT status_t init_driver(void)
{
	DEBUG_ONLY( dprintf(LOGID "--- init_driver, built %s %s\n", __DATE__, __TIME__) );

#if LOAD_SYMBOLS
	if ( load_driver_symbols(DRIVER_NAME) != B_OK )
		dprintf(LOGID WARN "load_driver_symbols(\"" DRIVER_NAME "\") FAILED !\n");
	else
		dprintf(LOGID "load_driver_symbols(\"" DRIVER_NAME "\") done.\n");
#endif
	
#ifdef DEBUGGER_COMMAND
	add_debugger_command(DEBUGGER_COMMAND, device_debugger_command, DRIVER_NAME " driver info");
#endif	

	return B_OK;
}


// -------------------------------------------------------
_EXPORT void uninit_driver(void)
{
	DEBUG_ONLY( dprintf(LOGID "--- uninit_driver\n") );

#ifdef DEBUGGER_COMMAND
	remove_debugger_command(DEBUGGER_COMMAND, device_debugger_command);
#endif
}


// -------------------------------------------------------
_EXPORT const char ** publish_devices(void)
{
	DEBUG_ONLY( dprintf(LOGID "--- publish_devices\n") );
	return (const char **) g_device_name_list;
}


// -------------------------------------------------------
_EXPORT device_hooks * find_device
	(
	const char *	name
	)
{
	DEBUG_ONLY( dprintf(LOGID "--- find_device %s\n", name) );
	return &g_device_hooks;
}


#ifdef CODEWARRIOR
	#pragma mark [Device hooks]
#endif

// Device hooks
// ------------

// -------------------------------------------------------
static status_t device_open
	(
	const char *	name, 
	uint32 			flags, 
	void **			cookie
	)
{
	net_endpoint *		ep;
	status_t			status;

#if STAY_LOADED
	if ( strcmp(name, UNLOAD_DEVICE_PATH) == 0) {
		*cookie = UNLOAD_COOKIE;
		return B_OK;
	};
#endif

	// TODO: move endpoint creation/init code into a dedicated module...
	ep = (net_endpoint *) malloc(sizeof(*ep));
	if ( NULL == ep ) {
		DEBUG_ONLY( dprintf(LOGID ERR "--- device_open: Can't alloc() a new net_endpoint, ARGH !\n") );
		return EINVAL;
	};

	ep->addr		= &ep->builtin_addr;
	ep->addrlen	= sizeof(ep->builtin_addr);
	ep->peer		= &ep->builtin_peer;
	ep->peerlen	= sizeof(ep->builtin_peer);
	ep->family 		= -1;
	ep->type		= -1;
	ep->proto		= -1;
		
	ep->open_flags = flags;
		
	DEBUG_ONLY( dprintf(LOGID "--- device_open %s (%s%s)\n", name,
						( ((flags & O_RWMASK) == O_RDONLY) ? "O_RDONLY" :
						  ((flags & O_RWMASK) == O_WRONLY) ? "O_WRONLY" : "O_RDWR"),
						(flags & O_NONBLOCK) ? " O_NONBLOCK" : "") );
		
	*cookie = ep;

	return B_OK;
}


// -------------------------------------------------------
static status_t device_close
	(
	void *	cookie
	)
{
	net_endpoint *	ep;
	status_t		status;

#if STAY_LOADED
	if (cookie == UNLOAD_COOKIE)
		return unload_driver();
#endif

	ep = (net_endpoint *) cookie;
	if ( NULL == ep ) {
		DEBUG_ONLY( dprintf(LOGID ERR "--- device_close: COOKIE BURNED !\n") );
		return B_ERROR;
	};
	
	DEBUG_ONLY( dprintf(LOGID "--- device_close %p\n", ep) );

	return B_OK;
}


// -------------------------------------------------------
static status_t device_free
	(
	void *	cookie
	)
{
	net_endpoint *	ep;
	status_t		status;

#if STAY_LOADED
	if (cookie == UNLOAD_COOKIE)
		return B_OK;
#endif

	ep = (net_endpoint *) cookie;
	if ( NULL == ep ) {
		DEBUG_ONLY( dprintf(LOGID ERR "--- device_free: COOKIE BURNED !\n") );
		return B_ERROR;
	};
	
	DEBUG_ONLY( dprintf(LOGID "--- device_free %p\n", ep) );
	
	free(ep);

	return B_OK;
}


// -------------------------------------------------------
static status_t device_control
	(
	void *	cookie, 
	uint32 	op_code, 
	void *	data, 
	size_t 	len
	)
{
	net_endpoint *	ep;
	status_t		status;

#if STAY_LOADED
	if (cookie == UNLOAD_COOKIE)
		return B_ERROR;
#endif

	ep = (net_endpoint *) cookie;
	if (NULL == ep) {
		DEBUG_ONLY( dprintf(LOGID ERR "--- device_control: COOKIE BURNED !\n") );
		return B_ERROR;
	};

#if STAY_LOADED
	stay_loaded();
#endif

	switch ( op_code ) {
	case NET_STACK_SOCKET:
		{
		socket_ioctl * ctl = (socket_ioctl *) data;

		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: socket(%d, %d, %d).\n",
					ep, ctl->family, ctl->type, ctl->proto) );
	
		ep->family 	= ctl->family;
		ep->type	= ctl->type;
		ep->proto	= ctl->proto;
		
		// TODO: try to load the stack (protocol(s) & layer(s) chain) for this 
		// family / type / proto socket...
		return B_OK;
		};
		
	case NET_STACK_SHUTDOWN:
		{
		int_ioctl * ctl = (int_ioctl *) data;

		// ctl->value = direction
		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: shutdown(%d).\n",
					ep, ctl->value) );

		return B_UNSUPPORTED;
		};
		
	case NET_STACK_CONNECT:
		{
		sockaddr_ioctl * ctl = (sockaddr_ioctl *) data;

		// ctl->addr = sockaddr to connect to...
		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: connect().\n",
					ep) );

		return B_UNSUPPORTED;
		};

	case NET_STACK_BIND:
		{
		sockaddr_ioctl * ctl = (sockaddr_ioctl *) data;

		// ctl->addr = sockaddr to bind
		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: bind().\n",
					ep) );

		return B_UNSUPPORTED;
		};
		
	case NET_STACK_LISTEN:
		{
		int_ioctl * ctl = (int_ioctl *) data;

		// ctl->value = backlog
		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: listen(%d).\n",
					ep, ctl->value) );

		return B_UNSUPPORTED;
		};
		
	case NET_STACK_ACCEPT:
		{
		accept_ioctl * ctl = (accept_ioctl *) data;

		// ctl->cookie = net_endpoint to use for the new, accepted, endpoint
		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: accept(%p).\n",
					ep, ctl->cookie) );

		return B_UNSUPPORTED;
		};
			
	case NET_STACK_SEND:
		{
		data_xfer_ioctl * ctl = (data_xfer_ioctl *) data;

		// TODO: store somewhere with ep the send() flags...
		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: send(%p, %d).\n",
					ep, ctl->data, ctl->datalen) );

		return device_write(ep, 0, ctl->data, &ctl->datalen);
		};
		
	case NET_STACK_RECV:
		{
		data_xfer_ioctl * ctl = (data_xfer_ioctl *) data;

		// TODO: store somewhere with ep the recv() flags...
		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: recv(%p, %d).\n",
					ep, ctl->data, ctl->datalen) );

		return device_read(ep, 0, ctl->data, &ctl->datalen);
		};

	case NET_STACK_SENDTO:
		{
		data_xfer_ioctl * ctl = (data_xfer_ioctl *) data;

		// ctl->addr = sockaddr to send to...
		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: sendto(%p, %d).\n",
					ep, ctl->data, ctl->datalen) );

		return B_UNSUPPORTED;
		};

	case NET_STACK_RECVFROM:
		{
		data_xfer_ioctl * ctl = (data_xfer_ioctl *) data;

		// ctl->addr = sockaddr to receive from...
		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: recvfrom(%p, %d).\n",
					ep, ctl->data, ctl->datalen) );

		return B_UNSUPPORTED;
		};
		
	case NET_STACK_GETPEER:
		{
		sockaddr_ioctl * 	ctl = (sockaddr_ioctl *) data;
			
		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: getpeername().\n",
					ep) );

		if (!ep->peer)
			return B_ERROR;
		memcpy(ctl->addr, ep->peer, min(ctl->len, ep->peerlen));
		return B_OK;
		};
			
	case NET_STACK_GETSOCK:
		{
		sockaddr_ioctl * 	ctl = (sockaddr_ioctl *) data;
			
		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: getsockname().\n",
					ep) );

		if (!ep->addr)
			return B_ERROR;
		memcpy(ctl->addr, ep->addr, min(ctl->len, ep->peerlen));
		return B_OK;
		};
			
	case NET_STACK_GETOPT:
		{
		sockopt_ioctl * ctl = (sockopt_ioctl *) data;
			
		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: getsockopt(%d, %d).\n",
					ep, ctl->level, ctl->option) );

		return B_UNSUPPORTED;
		};
			
	case NET_STACK_SETOPT:
		{
		sockopt_ioctl * ctl = (sockopt_ioctl *) data;

		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: setsockopt(%d, %d).\n",
					ep, ctl->level, ctl->option) );

		return B_UNSUPPORTED;
		};
  		
	case NET_STACK_GET_COOKIE:
		{
		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: NET_STACK_GET_COOKIE.\n",
					ep) );

		*((void **) data) = ep;
		return B_OK;
		};

	case NET_STACK_SELECT:
		{
		select_ioctl * ctl = (select_ioctl *) data;

		DEBUG_ONLY( dprintf(LOGID "--- device_control %p: select().\n") );		
		return select(ctl->nbits, ctl->rbits, ctl->wbits, ctl->ebits, ctl->timeout);
		};
		
	default:
		{
		DEBUG_ONLY( dprintf(LOGID WARN "--- device_control %p: op_code %d (0x%x) unhandled, , data = %p, len=%d.\n",
					ep, op_code, op_code, data, len) );
		return EINVAL;
		};
	}

	return B_ERROR;
}


// -------------------------------------------------------
static status_t device_read
	(
	void *	cookie, 
	off_t 		pos, 
	void *	data, 
	size_t *	len
	)
{
	iovec	vec;

	vec.iov_base 	= data;
	vec.iov_len		= *len;
	return device_readv(cookie, pos, &vec, 1, len);
}


// -------------------------------------------------------
static status_t device_readv
	(
	void * 		cookie, 
	off_t 			pos, 
	const iovec * 	vec, 
	size_t 		count, 
	size_t * 		len
	)
{
	net_endpoint *	ep;

#if STAY_LOADED
	if (cookie == UNLOAD_COOKIE) {
		*len = 0;
		return B_ERROR;
	};
#endif

	ep = (net_endpoint *) cookie;
	if ( NULL == ep ) {
		DEBUG_ONLY( dprintf(LOGID ERR "--- device_readv: COOKIE BURNED !\n") );
		return B_ERROR;
	};

	DEBUG_ONLY( dprintf(LOGID "--- device_readv %p: %p, %d\n",
				ep, vec, count ) );

#if STAY_LOADED
	stay_loaded();
#endif

	*len = 0;
		
	return B_OK; 
}


// -------------------------------------------------------
static status_t device_write
	(
	void *			cookie, 
	off_t 			pos, 
	const void *	data, 
	size_t *		len
	)
{
	iovec	vec;

	vec.iov_base 	= (void *) data;
	vec.iov_len		= *len;
	return device_writev(cookie, pos, &vec, 1, len);
}


// -------------------------------------------------------
static status_t device_writev
	(
	void * 		cookie, 
	off_t 			pos, 
	const iovec * 	vec, 
	size_t 		count, 
	size_t * 		len
	)
{
	net_endpoint *	ep;
	status_t		status;

#if STAY_LOADED
	if (cookie == UNLOAD_COOKIE) {
		*len = 0;
		return B_ERROR;
	};
#endif

	ep = (net_endpoint *) cookie;
	if ( NULL == ep ) {
		DEBUG_ONLY( dprintf(LOGID ERR "--- device_writev: COOKIE BURNED !\n") );
		return B_ERROR;
	};
	
	DEBUG_ONLY( dprintf(LOGID "--- device_writev %p: %p, %d\n",
				ep, vec, count ) );

#if STAY_LOADED
	stay_loaded();
#endif

	*len = 0;
	return B_OK;
}


// -------------------------------------------------------
static status_t device_select
	(
	void *		cookie, 
	uint8 		event, 
	uint32 		ref, 
	selectsync *	sync
	)
{
	DEBUG_ONLY( dprintf(LOGID "--- device_select %p: %d, %d, %p\n",
			cookie, event, ref, sync) );
	return B_UNSUPPORTED;
}


// -------------------------------------------------------
static status_t device_deselect
	(
	void *		cookie, 
	uint8 		event, 
	selectsync *	sync
	)
{
	DEBUG_ONLY( dprintf(LOGID "--- device_deselect %p: %d, %p\n",
			cookie, event, sync) );
	return B_UNSUPPORTED;
}


#ifdef CODEWARRIOR
	#pragma mark [Privates routines]
#endif

// Privates routines
// -----------------

#if STAY_LOADED
// -------------------------------------------------------
static status_t	stay_loaded()
{
	if ( g_stay_loaded_fd == -1 )
		{
		// force the driver to stay loaded by opening the "unload" device entry
		DEBUG_ONLY( dprintf(LOGID "--- stay_loaded: opening /dev/%s to stay loaded...\n", UNLOAD_DEVICE_PATH) );
		g_stay_loaded_fd = open("/dev/" UNLOAD_DEVICE_PATH, 0);
		if (g_stay_loaded_fd < 0)
			DEBUG_ONLY( dprintf(LOGID ERR "--- stay_loaded: couldn't open /dev/%s!\n", UNLOAD_DEVICE_PATH) );
		};

	return B_OK;
}


// -------------------------------------------------------
static status_t unload_driver()
{
	if ( g_stay_loaded_fd >= 0 )
		{
		int tmp_fd;
			
		// we need to set g_stay_loaded_fd to < 0 if we don't want
		// the next close enter again in this case, and so on...
		DEBUG_ONLY( dprintf(LOGID "--- unload_driver: unload requested.\n") );

		tmp_fd = g_stay_loaded_fd;
		g_stay_loaded_fd = -1;

		close(tmp_fd);
		};
			
	return B_OK;
}
#endif // #if STAY_LOADED

#ifdef CODEWARRIOR
	#pragma mark [Debugger Command]
#endif


#ifdef DEBUGGER_COMMAND

// -------------------------------------------------------
static int debugger_command
	(
	int 	argc,
	char **	argv
	)
{
	struct debugger_commands {
		int		id;
		char * 	name;
		char *	shortcut;
		// TO DO : command handling by sub-routine call
		// int 	(*cmd)(device * dev, int argc, char ** argv);
		char *	arguments;
	} * cmd, dbgr_cmds[] = {
			{0,  "help", "h", NULL},
			{-1,  NULL, NULL, NULL}
		};
	int 			cmd_id;
	int			return_value;
	int			i;
	
	return_value 	= 0;

	cmd_id 			= 0;	// help, by default

	if ( argc > 1 ) {
		cmd = dbgr_cmds;
		while ( cmd->name )	
			{
			if ( (strcmp(cmd->name, argv[1])== 0) ||
				 (strcmp(cmd->shortcut, argv[1]) == 0 ) )
				break;

			 cmd++;
			};
		cmd_id = cmd->id;
	};

	switch (cmd_id) {
	case 0:	// help
	default:
		{
		kprintf(DEBUGGER_COMMAND " usage:\n");
		cmd = dbgr_cmds;
		while ( cmd->name )	
			{
			kprintf("  %s", cmd->name);
			if ( cmd->shortcut )
				kprintf(" | %s", cmd->shortcut);
			if ( cmd->arguments )
				kprintf(" %s", cmd->arguments);
			kprintf("\n");

			cmd++;	// next
			};
			
		break;
		};
	};	// switch
	
	return return_value;
}

#endif // DEBUGGER_COMMAND
