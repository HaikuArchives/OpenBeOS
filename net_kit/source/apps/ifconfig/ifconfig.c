/* ifconfig.c */

#include <stdio.h>
#include <unistd.h>
#include <kernel/OS.h>
#include <stdlib.h>	/* for exit() */

#include "netinet/in.h"
#include "sys/socket.h"
#include "sys/sockio.h"
#include "arpa/inet.h"
#include "if.h"

#define version           "0.1 pre-alpha"

void usage(void)
{
	printf("ifconfig version %s\n", version);
	printf("OpenBeos Networking Team\n");
	printf("usage: ifconfig <ifname> <family> <address>\n");
	printf("\nNB: netmask is currently calculated on address supplied.\n");
	exit(0);
}
	
int main(int argc, char **argv)
{
	int sock;
	int rv;
	struct ifreq ifr;
	struct sockaddr_in *sin = (struct sockaddr_in*)&ifr.ifr_addr;
		
	if (argc < 3)
		usage();
	
	if (strcmp(argv[2], "inet") != 0) {
		printf("ifconfig so far only understands 'inet' as it's family.\n");
		exit(-1);
	}

	strcpy(ifr.ifr_name, argv[1]);
	sin->sin_family = AF_INET;
	sin->sin_len = sizeof(struct sockaddr_in);
	rv = inet_aton(argv[3], &sin->sin_addr);
	if (rv == 0) {
		printf("Unable to make the supplied address make sense %d!\n", rv);
		printf("%s != 0x%08lx\n", argv[3], sin->sin_addr.s_addr);
		exit(-1);
	}
	printf("address is 0x%08lx\n", sin->sin_addr.s_addr);
	
	printf("Ready!\n");
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		printf("Couldn't open a socket to send request on.\n");
		exit(-1);
	}
	
	rv = ioctl(sock, SIOCSIFADDR, &ifr);
	printf("ioctl gave %d\n", rv);
	
	closesocket(sock);
	
	return 0;
}
