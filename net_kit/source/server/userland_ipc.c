/* userland_ipc - Communication between the network driver
**		and the userland stack.
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include "userland_ipc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// installs a main()
#define COMMUNICATION_TEST

// maximum 2048 / sizeof(net_command) == 256 commands
#define NUM_COMMANDS 32
#define NUM_READ_BUFFERS 8
#define NUM_WRITE_BUFFERS 8

#define ROUND_TO_PAGE_SIZE(x) (((x) + (B_PAGE_SIZE) - 1) & ~((B_PAGE_SIZE) - 1))

typedef struct {
	port_id		localPort,port;
	area_id		area;

	void		*readBuffer,*writeBuffer;
	sem_id		commandSemaphore;
	net_command *commands;

	int32		openFlags;

	thread_id	runner;
} connection_cookie;


port_id gStackPort = -1;
thread_id gConnectionOpener = -1;

// prototypes
static int32 connection_runner(void *_cookie);
static status_t init_connection(net_connection *connection, connection_cookie **_cookie);
static void shutdown_connection(connection_cookie *cookie);


static int32
connection_runner(void *_cookie)
{
	connection_cookie *cookie = (connection_cookie *)_cookie;
	bool run = true;

	while (run) {
		net_command *command;
		int32 index;
		char buffer[256];
		ssize_t bytes = read_port(cookie->localPort,&index,buffer,sizeof(buffer));
		if (bytes < B_OK)
			break;

		if (index >= NUM_COMMANDS || index < 0) {
			printf("got bad command index: %lx\n",index);
			continue;
		}
		command = cookie->commands + index;

		switch (command->op) {
			case NET_STACK_OPEN:
				cookie->openFlags = *(int32 *)&buffer;
				printf("opening socket, mode = %ld!\n",cookie->openFlags);
				break;
			case NET_STACK_CLOSE:
				printf("closing socket...\n");
				run = false;
				break;
			default:
				printf("received command: %lx (%ld bytes read)\n",command->op,bytes);
				// ToDo: call the core stack here
				break;
		}
		// mark the command as done
		command->op = 0;
		command->result = B_OK;

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
			(NUM_READ_BUFFERS + NUM_WRITE_BUFFERS + 2) * NET_BUFFER_SIZE,
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
	connection->numReadBuffers = NUM_READ_BUFFERS;
	connection->numWriteBuffers = NUM_WRITE_BUFFERS;

	// setup connection cookie
	cookie->area = connection->area;
	cookie->commands = commands;
	cookie->readBuffer = (uint8 *)commands + NET_BUFFER_SIZE;
	cookie->writeBuffer = (uint8 *)cookie->readBuffer + NUM_READ_BUFFERS * NET_BUFFER_SIZE;
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
			fprintf(stderr,"connection_opener: received unknown command: %ld\n",msg);
	}
	return 0;
}


status_t
init_userland_ipc()
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
shutdown_userland_ipc()
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
