#include <stdio.h>
#include <kernel/OS.h>
#include <string.h>
#include <sys/time.h>

#include "sys/socket.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "sys/select.h"

#include "ufunc.h"

int main(int argc, char **argv)
{
	int s;
	int rv;
	struct fd_set fdr, fdw, fde;
	struct timeval tv;

	test_banner("Select test");
			
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		err(s, "Socket creation failed");

	FD_ZERO(&fdr);
	FD_SET(s, &fdr);
	FD_ZERO(&fdw);
	FD_SET(s, &fdw);
	FD_ZERO(&fde);
	FD_SET(s, &fde);
	
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	
	printf("Trying with timeval...\n");
	rv = net_select(s +1, &fdr, &fdw, &fde, &tv);
	printf("select gave %d\n", rv);
	rv = net_select(s +1, &fdr, &fdw, &fde, &tv);
	printf("select gave %d\n", rv);
	rv = net_select(s +1, &fdr, &fdw, &fde, &tv);
	printf("select gave %d\n", rv);
	
	printf("Trying without timeval (= NULL)\n");
	rv = net_select(s +1, &fdr, &fdw, &fde, NULL);
	printf("select gave %d\n", rv);
	
	closesocket(s);
	
	printf("Test complete.\n");

	return (0);
}

	
