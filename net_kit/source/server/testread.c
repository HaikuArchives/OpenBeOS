#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <OS.h>
#include <unistd.h>

#include "if.h"
#include "net_misc.h"
#include "mbuf.h"
#include "ethernet/ethernet.h"

/* Not currently being used! */
/*
static void dump_buffer(char *buffer) {
	uint8 *b = (uint8 *)buffer;
	int i;
	
	printf ("  ");
	for (i=0;i<128;i+=2) {
		if (i%16 == 0)
			printf("\n  ");
		printf(" %02x", b[i]);
		printf("%02x ", b[i+1]);
	}
	printf("\n\n");
}
*/

/* Very much hardcoded device address! Change to point at your
 * card's device driver.
 */ 
int main(int argc, char **argv)
{
	int dev = open("/dev/net/tulip/0", O_RDONLY);
	char *buffer = malloc(sizeof(char) * 1500);
	size_t len = 1500;
	status_t status;
	int on = 1;
	ether_addr ea;

	/* This shouldn't really be here, but it needs to be here
	 * or we'll not have any pools setup to work with. This should
	 * eventually be called in the net_module init.
	 */
	mbinit();
	
	printf("Network Card Read Test\n");
	printf("======================\n\n");
	printf("Have you set the device path to point at your card?\n");
	printf("If not you probably won't see anything!\n\n");

	if (dev < B_OK) {
		printf("Couldn't open the device! %s\n", strerror(dev));
		exit (-1);
	}
	printf("Opened the device.\n");

	status = ioctl(dev, IF_GETADDR, &ea, 6);
	if (status < B_OK)
		printf("Unable to get MAC address\n");
	else
		printf("MAC address %02x:%02x:%02x:%02x:%02x:%02x\n",
			ea.addr[0], ea.addr[1], ea.addr[2],
			ea.addr[3], ea.addr[4], ea.addr[5]);

	/* not sure why this doesn't currently work... */	
	status = ioctl(dev, IF_SETPROMISC, &on);
	if (status < B_OK)
		printf("Unable to set promiscuous. %s\n", strerror(status));
		
	while ((status = read(dev, buffer, len)) >= B_OK) {
		struct mbuf *mb = m_devget(buffer, len, 0, NULL);
		ethernet_input(mb);
	}
	
	close (dev);
	
	printf("Device closed!\n");
	return 0;
}

