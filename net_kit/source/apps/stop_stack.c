#include <stdio.h>
#include <kernel/OS.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>

#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "sys/select.h"
#include "../libnet/net_stack_driver.h"

#include "ufunc.h"

int main(int argc, char **argv)
{
	int s;
	int rv;
	
	test_banner("Stopping Server");
			
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		err(s, "Socket creation failed");

	rv = ioctl(s, NET_STACK_STOP, NULL, 0);
	printf("ioctl to stop stack gave %d\n", rv);
	rv = close(s);
	printf("close gave %d\n", rv);
	
	printf("Operation complete.\n");

	return (0);
}

