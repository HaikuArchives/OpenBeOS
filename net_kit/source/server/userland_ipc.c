/* userland_ipc - Communication between the network driver
**		and the userland stack.
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include "userland_ipc.h"

#include "sys/socket.h"
#include "net_misc.h"
#include "core_module.h"
#include "net_module.h"
#include "sys/sockio.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


extern struct core_module_info *core;

// installs a main()
//#define COMMUNICATION_TEST

#define NUM_COMMANDS 32
#define CONNECTION_BUFFER_SIZE (65536 + 4096 - CONNECTION_COMMAND_SIZE)

#define ROUND_TO_PAGE_SIZE(x) (((x) + (B_PAGE_SIZE) - 1) & ~((B_PAGE_SIZE) - 1))

typedef struct {
	port_id		localPort,port;
	area_id		area;
	void		*socket;

	uint8		*buffer;
	net_command *commands;
	sem_id		commandSemaphore;

	int32		openFlags;

	thread_id	runner;
} connection_cookie;


port_id gStackPort = -1;
thread_id gConnectionOpener = -1;

// prototypes
static int32 connection_runner(void *_cookie);
static status_t init_connection(net_connection *connection, connection_cookie **_cookie);
static void shutdown_connection(connection_cookie *cookie);


static void
delete_cloned_areas(net_area_info *area)
{
	int32 i;
	for (i = 0;i < MAX_NET_AREAS;i++) {
		if (area[i].id == 0)
			continue;

		delete_area(area[i].id);
	}
}


static status_t
clone_command_areas(net_area_info *localArea,net_command *command)
{
	int32 i;

	memset(localArea,0,sizeof(net_area_info) * MAX_NET_AREAS);

	for (i = 0;i < MAX_NET_AREAS;i++) {
		if (command->area[i].id <= 0)
			continue;

		localArea[i].id = clone_area("net connection",(void **)&localArea[i].offset,B_ANY_ADDRESS,
				B_READ_AREA | B_WRITE_AREA,command->area[i].id);
		if (localArea[i].id < B_OK)
			return localArea[i].id;
	}
	return B_OK;
}


static uint8 *
convert_address(net_area_info *fromArea,net_area_info *toArea,uint8 *data)
{
	if (data == NULL)
		return NULL;

	if (data < fromArea->offset) {
		printf("could not translate address: %p\n",data);
		return data;
	}

	return data - fromArea->offset + toArea->offset;
}


static inline void *
convert_to_local(net_area_info *foreignArea,net_area_info *localArea,void *data)
{
	return convert_address(foreignArea,localArea,data);
}


static void *
convert_to_foreign(net_area_info *foreignArea,net_area_info *localArea,void *data)
{
	return convert_address(localArea,foreignArea,data);
}


static int32
connection_runner(void *_cookie)
{
	connection_cookie *cookie = (connection_cookie *)_cookie;
	bool run = true;

	while (run) {
		net_area_info area[MAX_NET_AREAS];
		net_command *command;
		status_t status = B_OK;
		uint8 *data;
		int32 index;
		ssize_t bytes = read_port(cookie->localPort,&index,NULL,0);
		if (bytes < B_OK)
			break;

		if (index >= NUM_COMMANDS || index < 0) {
			printf("got bad command index: %lx\n",index);
			continue;
		}
		command = cookie->commands + index;
		if (clone_command_areas(area,command) < B_OK) {
			printf("could not clone command areas!\n");
			continue;
		}

		data = convert_to_local(&command->area[0],&area[0],command->data);
		printf("command %lx (index = %ld), buffer = %p, length = %ld, result = %ld\n",command->op,index,data,command->length,command->result);

		switch (command->op) {
			case NET_STACK_OPEN:
			{
				struct int_args *args = (struct int_args *)data;
				cookie->openFlags = args->value;
				printf("opening socket, mode = %lx!\n",cookie->openFlags);
				break;
			}
			case NET_STACK_CLOSE:
				printf("closing socket...\n");
				run = false;
				break;

			case NET_STACK_SOCKET:
			{
				struct socket_args *args = (struct socket_args *)data;

				printf("open a socket... family = %d, type = %d, proto = %d\n",args->family,args->type,args->proto);
				status = core->initsocket(&cookie->socket);
				if (status == 0)
					status = core->socreate(args->family,cookie->socket,args->type,args->proto);
				break;
			}
			case NET_STACK_GETSOCKOPT:
			case NET_STACK_SETSOCKOPT:
			{
				struct sockopt_args *sockopt = (struct sockopt_args *)data;

				if (command->op == NET_STACK_GETSOCKOPT) {
					status = core->sogetopt(cookie->socket,sockopt->level,sockopt->option,
						convert_to_local(&command->area[1],&area[1],sockopt->optval),
						(size_t *)&sockopt->optlen);
				} else {
					status = core->sosetopt(cookie->socket,sockopt->level,sockopt->option,
						(const void *)convert_to_local(&command->area[1],&area[1],sockopt->optval),
						sockopt->optlen);
				}
				break;
			}
			case NET_STACK_CONNECT:
			case NET_STACK_BIND:
			case NET_STACK_GETSOCKNAME:
			case NET_STACK_GETPEERNAME:
			{
				struct sockaddr_args *args = (struct sockaddr_args *)data;
				caddr_t addr = (caddr_t)convert_to_local(&command->area[1],&area[1],args->addr);

				switch (command->op) {
					case NET_STACK_CONNECT:
						status = core->soconnect(cookie->socket,addr,args->addrlen);
						break;
					case NET_STACK_BIND:
						status = core->sobind(cookie->socket,addr,args->addrlen);
						break;
					case NET_STACK_GETSOCKNAME:
						status = core->sogetsockname(cookie->socket,(struct sockaddr *)addr,&args->addrlen);
						break;
					case NET_STACK_GETPEERNAME:
						status = core->sogetpeername(cookie->socket,(struct sockaddr *)addr,&args->addrlen);
						break;
				}
				break;
			}
			case NET_STACK_LISTEN:
				status = core->solisten(cookie->socket,((struct int_args *)data)->value);
				break;

			case NET_STACK_GET_COOKIE:
				/* this is needed by accept() call, to be able to pass back
				 * in NET_STACK_ACCEPT opcode the cookie of the filedescriptor to 
				 * use for the new accepted socket
				 */
				*((void **)data) = cookie;
				break;
			case NET_STACK_ACCEPT:
			{
				struct accept_args *args = (struct accept_args *)data;
				connection_cookie *otherCookie = (connection_cookie *)args->cookie;
				status = core->soaccept(cookie->socket,&otherCookie->socket,
					convert_to_local(&command->area[1],&area[1],args->addr),
					&args->addrlen);
			}
			case NET_STACK_SEND:
			{
				struct data_xfer_args *args = (struct data_xfer_args *)data;
				struct iovec iov;
				int flags = 0;

				iov.iov_base = convert_to_local(&command->area[1],&area[1],args->data);
				iov.iov_len = args->datalen;

				status = core->writeit(cookie->socket,&iov,flags);
				break;
			}
			case NET_STACK_RECV:
			{
				struct data_xfer_args *args = (struct data_xfer_args *)data;
				struct iovec iov;
				int flags = 0;

				iov.iov_base = convert_to_local(&command->area[1],&area[1],args->data);
				iov.iov_len = args->datalen;

				/* flags gets ignored here... */
				status = core->readit(cookie->socket,&iov,&flags);
				break;
			}
			case NET_STACK_RECVFROM:
			{
				struct msghdr *msg = (struct msghdr *)data;
				int received;

				msg->msg_name = convert_to_local(&command->area[1],&area[1],msg->msg_name);
				msg->msg_iov = convert_to_local(&command->area[2],&area[2],msg->msg_iov);
				msg->msg_control = convert_to_local(&command->area[3],&area[3],msg->msg_control);

				status = core->recvit(cookie->socket, msg, (caddr_t)&msg->msg_namelen,&received);
				if (status == 0)
					status = received;

				msg->msg_name = convert_to_foreign(&command->area[1],&area[1],msg->msg_name);
				msg->msg_iov = convert_to_foreign(&command->area[2],&area[2],msg->msg_iov);
				msg->msg_control = convert_to_foreign(&command->area[3],&area[3],msg->msg_control);
				break;
			}
			case NET_STACK_SENDTO:
			{
				struct msghdr *msg = (struct msghdr *)data;
				int sent;

				msg->msg_name = convert_to_local(&command->area[1],&area[1],msg->msg_name);
				msg->msg_iov = convert_to_local(&command->area[2],&area[2],msg->msg_iov);
				msg->msg_control = convert_to_local(&command->area[3],&area[3],msg->msg_control);
	
				status = core->sendit(cookie->socket,msg,msg->msg_flags,&sent);
				if (status == 0)
					status = sent;

				msg->msg_name = convert_to_foreign(&command->area[1],&area[1],msg->msg_name);
				msg->msg_iov = convert_to_foreign(&command->area[2],&area[2],msg->msg_iov);
				msg->msg_control = convert_to_foreign(&command->area[3],&area[3],msg->msg_control);
				break;
			}

			case NET_STACK_SYSCTL:
			{
				struct sysctl_args *args = (struct sysctl_args *)data;
				
				status = core->net_sysctl(convert_to_local(&command->area[1],&area[1],args->name),
					args->namelen,convert_to_local(&command->area[2],&area[2],args->oldp),
					convert_to_local(&command->area[3],&area[3],args->oldlenp),
					convert_to_local(&command->area[4],&area[4],args->newp),
					args->newlen);
				break;
			}

			case NET_STACK_STOP:
				core->stop();
				break;

			case B_SET_BLOCKING_IO:
				cookie->openFlags &= ~O_NONBLOCK;
				return B_OK;
	
			case B_SET_NONBLOCKING_IO:
				cookie->openFlags |= O_NONBLOCK;
				return B_OK;

			case NET_STACK_SELECT:
			{
				//struct select_args *args = (struct select_args *)data;

				//status = select(args->nbits, args->rbits, args->wbits, args->ebits, args->timeout);
			}

			case OSIOCGIFCONF:
			case SIOCGIFCONF:
			{
				struct ifconf *ifc = (struct ifconf *)data;
				ifc->ifc_buf = convert_to_local(&command->area[1],&area[1],ifc->ifc_buf);

				status = core->soo_ioctl(cookie->socket,command->op,data);

				ifc->ifc_buf = convert_to_foreign(&command->area[1],&area[1],ifc->ifc_buf);
				break;
			}

			default:
				status = core->soo_ioctl(cookie->socket,command->op,data);
				break;
		}
		// mark the command as done
		command->result = status;
		command->op = 0;
		delete_cloned_areas(area);

		// notify the command pipeline that we're done with the command
		release_sem(cookie->commandSemaphore);
	}

	cookie->runner = -1;
	shutdown_connection(cookie);

	return 0;
}


static status_t
init_connection(net_connection *connection,connection_cookie **_cookie)
{
	connection_cookie *cookie;
	net_command *commands;

	cookie = (connection_cookie *)malloc(sizeof(connection_cookie));
	if (cookie == NULL) {
		fprintf(stderr,"couldn't allocate memory for cookie.\n");
		return B_NO_MEMORY;
	}

	connection->area = create_area("net connection",(void *)&commands,B_ANY_ADDRESS,
			CONNECTION_BUFFER_SIZE + CONNECTION_COMMAND_SIZE,
			B_NO_LOCK,B_READ_AREA | B_WRITE_AREA);
	if (connection->area < B_OK) {
		fprintf(stderr,"couldn't create area: %s.\n",strerror(connection->area));
		free(cookie);
		return connection->area;
	}
	memset(commands,0,NUM_COMMANDS * sizeof(net_command));

	connection->port = create_port(CONNECTION_QUEUE_LENGTH,"net stack connection");
	if (connection->port < B_OK) {
		fprintf(stderr,"couldn't create port: %s.\n",strerror(connection->port));
		delete_area(connection->area);
		free(cookie);
		return connection->port;
	}
	
	connection->commandSemaphore = create_sem(0,"net command queue");
	if (connection->commandSemaphore < B_OK) {
		fprintf(stderr,"couldn't create semaphore: %s.\n",strerror(connection->commandSemaphore));
		delete_area(connection->area);
		delete_port(connection->port);
		free(cookie);
		return connection->commandSemaphore;
	}

	cookie->runner = spawn_thread(connection_runner,"connection runner",B_NORMAL_PRIORITY,cookie);
	if (cookie->runner < B_OK) {
		fprintf(stderr,"couldn't create thread: %s.\n",strerror(cookie->runner));
		delete_sem(connection->commandSemaphore);
		delete_area(connection->area);
		delete_port(connection->port);
		free(cookie);
		return B_ERROR;
	}

	connection->numCommands = NUM_COMMANDS;
	connection->bufferSize = CONNECTION_BUFFER_SIZE;

	// setup connection cookie
	cookie->area = connection->area;
	cookie->commands = commands;
	cookie->buffer = (uint8 *)commands + CONNECTION_COMMAND_SIZE;
	cookie->commandSemaphore = connection->commandSemaphore;
	cookie->localPort = connection->port;
	cookie->openFlags = 0;

	resume_thread(cookie->runner);

	*_cookie = cookie;
	return B_OK;
}


static void
shutdown_connection(connection_cookie *cookie)
{
	printf("free cookie: %p\n",cookie);
	kill_thread(cookie->runner);

	delete_port(cookie->localPort);
	delete_sem(cookie->commandSemaphore);
	delete_area(cookie->area);

	free(cookie);
}


static int32
connection_opener(void *_unused)
{
	while(true) {
		port_id port;
		int32 msg;
		ssize_t bytes = read_port(gStackPort,&msg,&port,sizeof(port_id));
		if (bytes < B_OK)
			return bytes;

		if (msg == NET_STACK_NEW_CONNECTION) {
			net_connection connection;
			connection_cookie *cookie;

			printf("incoming connection...\n");
			if (init_connection(&connection,&cookie) == B_OK)
				write_port(port,NET_STACK_NEW_CONNECTION,&connection,sizeof(net_connection));
		} else
			fprintf(stderr,"connection_opener: received unknown command: %lx (expected = %lx)\n",msg,(int32)NET_STACK_NEW_CONNECTION);
	}
	return 0;
}


status_t
init_userland_ipc(void)
{
	gStackPort = create_port(CONNECTION_QUEUE_LENGTH,NET_STACK_PORTNAME);
	if (gStackPort < B_OK)
		return gStackPort;

	gConnectionOpener = spawn_thread(connection_opener,"connection opener",B_NORMAL_PRIORITY,NULL);
	if (resume_thread(gConnectionOpener) < B_OK) {
		delete_port(gStackPort);
		if (gConnectionOpener >= B_OK) {
			kill_thread(gConnectionOpener);
			return B_BAD_THREAD_STATE;
		}
		return gConnectionOpener;
	}

	return B_OK;
}


void
shutdown_userland_ipc(void)
{
	delete_port(gStackPort);
	kill_thread(gConnectionOpener);
}


#ifdef COMMUNICATION_TEST
int
main(void)
{
	char buffer[8];

	if (init_userland_ipc() < B_OK)
		return -1;

	puts("Userland_ipc - test is running. Press <Return> to quit.");
	fgets(buffer,sizeof(buffer),stdin);

	shutdown_userland_ipc();

	return 0;
}
#endif	/* COMMUNICATION_TEST */
