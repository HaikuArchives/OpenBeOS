/* net_server.c */

/* this is just a hack to get us an application that
 * we can develop while we continue to develop the net stack
 */

#include <stdio.h>
#include <kernel/OS.h>
#include <unistd.h>
#include <dirent.h>
#include <Drivers.h>
#include <limits.h>
#include <string.h>
#include <image.h>
#include <stdlib.h>


#include "sys/socket.h"
#include "net_misc.h"
#include "core_module.h"
#include "net_module.h"
#include "sys/sockio.h"

#include "core_funcs.h"

#define CORE_MODULE_PATH       "modules/core"

static struct core_module_info *core = NULL;
static image_id core_id;

static void load_core(void)
{
	char path[PATH_MAX];
	status_t status = -1;
	
	getcwd(path, PATH_MAX);
	sprintf(path, "%s/%s", path, CORE_MODULE_PATH);
	
	printf("opening %s\n", path);
	core_id = load_add_on(path);
	if (core_id > 0) {
		status = get_image_symbol(core_id, "core_info",
		                          B_SYMBOL_TYPE_ANY, 
		                          (void**)&core);
		if (status == 0)
			return;
		unload_add_on(core_id);
	}
	printf("Unable to load the core module! Can't continue.\n");
	if (core_id < 0)
		printf("core_id %ld, %s\n", core_id, strerror(core_id));
	else
		printf("status = %ld, %s\n", status, strerror(status));
	exit(-1);
}

int32 create_sockets(void *data)
{
	int howmany = *(int*)data;
	int intval = howmany / 10;
	void *sp = NULL;
	int rv, cnt = 0;
	bigtime_t tnow;

	printf("\n*** socket creation test ***\n\n");
	
#define stats(x)	printf("Total sockets created: %d\n", x);

	tnow = real_time_clock_usecs();
	while (cnt < howmany) {
		rv = initsocket(&sp);
		if (rv != 0) {
			printf("failed to create a new empty socket!\n");
			stats(cnt);
			return -1;
		} else {
			rv = socreate(AF_INET, sp, SOCK_DGRAM, 0);
			if (rv != 0) {
				printf("socreate failed! %d [%s]\n", rv, strerror(rv));
				stats(cnt);
				return -1;
			}
		}
		rv = soclose(sp);
		if (rv != 0) {
			printf("Error closing socket! %d [%s]\n", rv, strerror(rv));
			stats(cnt);
			return -1;
		}
		cnt++;
		if ((cnt % intval) == 0)
			printf("socket test: %3d %%\n", (cnt / intval) * 10);
	}
	printf("%d sockets in %lld usecs...\n", howmany, real_time_clock_usecs() - tnow);	
	printf("I have created %d sockets...\n", howmany);
	
	return 0;
}

void assign_addresses(void)
{
	void *sp; /* socket pointer... */
	int rv;
	struct ifreq ifr;
	
	printf("*************** assign socket addresses **************\n"
	       "Have you changed these to match your card and network?\n"
	       "******************************************************\n");

	rv = initsocket(&sp);
	if (rv < 0) {
		printf("Couldn't get a socket!\n");
		return;
	}
	
	rv = socreate(AF_INET, sp, SOCK_DGRAM, 0);
	if (rv < 0) {
		printf("Failed to create a socket to use...\n");
		return;
	}
	
	strcpy(ifr.ifr_name, "tulip0");
	memset(&ifr.ifr_addr, 0, sizeof(ifr.ifr_addr));
	
	
	rv = soo_ioctl(sp, SIOCGIFFLAGS, (caddr_t)&ifr);
	if (rv < 0) {
		printf("failed to configure %s\n", ifr.ifr_name);
		printf("soo_ioctl gave %d\n", rv);
		goto loop;
	}

	((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr = htonl(0xc0a80085);
	
	rv = soo_ioctl(sp, SIOCSIFADDR, (caddr_t)&ifr);
	if (rv < 0) {
		printf("failed to configure %s\n", ifr.ifr_name);
		printf("error %d [%s]\n", rv, strerror(rv));
	} else {
		printf("%s configured\n", ifr.ifr_name);
	}


loop:
	
	strcpy(ifr.ifr_name,"loop0");
	((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr = htonl(0x7f000001);
	((struct sockaddr_in*)&ifr.ifr_addr)->sin_family = AF_INET;
	((struct sockaddr_in*)&ifr.ifr_addr)->sin_len = sizeof(struct sockaddr_in);
	rv = soo_ioctl(sp, SIOCSIFADDR, (caddr_t)&ifr);
	if (rv < 0) {
		printf("failed to configure %s\n", ifr.ifr_name);
		printf("error %d [%s]\n", rv, strerror(rv));
	} else {
		printf("%s configured\n", ifr.ifr_name);
	}
	
	soclose(sp);

}

#define TEST_DATA "Hello World"

static void err(int code, char *msg)
{
	printf("Error: %s: %d [%s]\n", msg, code, strerror(code));
}
		
static void sysctl_test(void)
{
	int mib[5];
	size_t needed;
	char *buf;
	caddr_t lim;
	
	printf ("sysctl test\n"
	        "===========\n");
	
	mib[0] = PF_ROUTE;
	mib[1] = 0;
	mib[2] = 0;
	mib[3] = NET_RT_DUMP;
	mib[4] = 0;
	if (net_sysctl(mib, 5, NULL, &needed, NULL, 0) < 0)	{
		perror("route-sysctl-estimate");
		exit(1);
	}

	printf("estimated to need %ld bytes\n", needed);

	if (needed > 0) {
		if ((buf = malloc(needed)) == 0) {
			printf("out of space\n");
			exit(1);
		}
		if (net_sysctl(mib, 5, buf, &needed, NULL, 0) < 0) {
			perror("sysctl of routing table");
			exit(1);
		}
		lim  = buf + needed;
	}
	printf("got data...\n");
}

#define HELLO "Hello from net_srv"
#define RESPONSE "Knock, knock"

static int32 accept_test_server(void *data)
{
	void *sp, *nsp;
	struct sockaddr_in sin;
	int rv;
	int salen = sizeof(sin);
	char buff[50];
	struct iovec iov;
	int flags = 0;
		
	rv = initsocket(&sp);
	if (rv < 0) {
		err(rv, "initsocket");
		return -1;
	}	

	rv = socreate(AF_INET, sp, SOCK_STREAM, 0);
	if (rv < 0) {
		err(rv, "Failed to create a socket to use...");
		return -1;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(7777);
	sin.sin_addr.s_addr = INADDR_LOOPBACK;
		
	rv = sobind(sp, (caddr_t)&sin, salen);
	if (rv < 0) {
		err(rv, "bind");
		return -1;
	}	
	
	rv = solisten(sp, 5);
	if (rv < 0) {
		err(rv, "bind");
		return -1;
	}	
	printf("server listening...\n");

	rv = soaccept(sp, &nsp, (struct sockaddr *)&sin, &salen);
	if (rv < 0) {
		err(rv, "soaccept");
		return -1;
	}	
	printf("\n *** WooHoo - accepted a socket! ***\n\n");

	memset(&buff, 0, 50);
	memcpy(&buff, HELLO, strlen(HELLO));
	iov.iov_base = &buff;
	iov.iov_len = strlen(HELLO);
	
	rv = writeit(nsp, &iov, flags);
	if (rv < 0) {
		err(rv, "writeit");
		return -1;
	}
	printf("Sent hello to connected socket\n");

	memset(&buff, 0, 50);
	iov.iov_base = &buff;
	iov.iov_len = 50;
	rv = readit(nsp, &iov, &flags);
	if (rv < 0) {
		err(rv, "readit");
		return -1;
	}
	printf("Got reply from connected socket: %s\n", buff);
	
	soclose(nsp);
	soclose(sp);
}

static int32 accept_test_client(void *data)
{
	void *sp;
	struct sockaddr_in sin;
	int rv;
	int salen = sizeof(sin);
	char buffer[50];
	int flags = 0;
	struct iovec iov;
	
	rv = initsocket(&sp);
	if (rv < 0) {
		err(rv, "initsocket");
		return -1;
	}	

	rv = socreate(AF_INET, sp, SOCK_STREAM, 0);
	if (rv < 0) {
		err(rv, "Failed to create a socket to use...");
		return -1;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	sin.sin_addr.s_addr = INADDR_LOOPBACK;
		
	rv = sobind(sp, (caddr_t)&sin, salen);
	if (rv < 0) {
		err(rv, "bind");
		return -1;
	}	

	sin.sin_port = htons(7777);

	printf("client about to try and connect\n");	
	rv = soconnect(sp, (caddr_t)&sin, salen);	
	if (rv < 0) {
		err(rv, "soconnect");
		return -1;
	}
	printf("client has connected!!!\n");

	memset(&buffer, 0, 50);
	iov.iov_base = &buffer;
	iov.iov_len = 50;
	
	rv = readit(sp, &iov, &flags);	
	if (rv < 0) {
		err(rv, "readit");
		return -1;
	}
	printf("Got a greeting...%s\n", buffer);

	memset(&buffer, 0, 50);
	memcpy(&buffer, RESPONSE, strlen(RESPONSE)); 	
	iov.iov_base = &buffer;
	iov.iov_len = strlen(RESPONSE);
	flags = 0;
	rv = writeit(sp, &iov, flags);	
	if (rv < 0) {
		err(rv, "writeit");
		return -1;
	}

	soclose(sp);
	printf("Client completed OK\n");	
}
	
	
static void tcp_test(uint32 srv)
{
	void *sp;
	struct sockaddr_in sa;
	int rv;
	char buffer[200];
	struct iovec iov;
	int flags = 0;
	
	printf("\n*** TCP Test ***\n\n");	
	rv = initsocket(&sp);
	if (rv < 0) {
		err(rv, "Couldn't get a 2nd socket!");
		return;
	}
	
	rv = socreate(AF_INET, sp, SOCK_STREAM, 0);
	if (rv < 0) {
		err(rv, "Failed to create a socket to use...");
		return;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons((real_time_clock() & 0xffff));
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_len = sizeof(sa);
	
	rv = sobind(sp, (caddr_t)&sa, sizeof(sa));
	if (rv < 0) {
		err(rv, "Failed to bind!\n");
		return;
	}

	sa.sin_addr.s_addr = htonl(srv);
	sa.sin_port = htons(80);
	
	rv = soconnect(sp, (caddr_t)&sa, sizeof(sa));
	if (rv < 0) {
		err(rv, "Connect failed!!");
		return;
	}
	snooze(500000);

	memset(&buffer, 0, 200);
	strcpy(buffer, "GET / HTTP/1.0\n\n");
	iov.iov_base = &buffer;
	iov.iov_len = 16;

	
	rv = writeit(sp, &iov, flags);
	if (rv < 0) {
		err(rv, "writeit failed!!");
		return;
	}
	printf("writeit wrote %d bytes\n\n", rv);
	
	memset(&buffer, 0, 200);
	iov.iov_len = 200;
	iov.iov_base = &buffer;
	while ((rv = readit(sp, &iov, &flags)) >= 0) {
		if (rv < 0) { 
			err (rv, "readit");
			soclose(sp);
			return;
		} else
			printf("%s", buffer);
		if (rv == 0)
			break;
		/* PITA - have to keep resetting these... */
		iov.iov_len = 200;
		iov.iov_base = &buffer;
		memset(&buffer, 0, 200);
	}
	
	soclose(sp);
	printf("\nTCP socket test completed...\n");
}

int main(int argc, char **argv)
{
//	int qty = 1000;
	thread_id srv, clt;

	/* XXX - change this line to query a different web server! */
	uint32 web_server = 0xc0a80001;
	
	printf("Net Server Test App!\n"
	       "====================\n\n");

	load_core();
	core->start();
	
	assign_addresses();

	//create_sockets((void*)&qty);
	tcp_test(web_server);
	sysctl_test();

	snooze(1000000);
	
	srv = spawn_thread(accept_test_server, "accept_test_server", 
	                   B_NORMAL_PRIORITY, NULL);
	if (srv > 0)
		resume_thread(srv);

	clt = spawn_thread(accept_test_client, "accept_test_client", 
	                   B_NORMAL_PRIORITY, NULL);
	if (clt > 0)
		resume_thread(clt);
	
	
	printf("\n\n Tests completed!\n");
	printf("\nTo stop, use CTRL-C :( \n");
	
	/* infinite loop */
	while (1) {}
	
	return 0;
}

