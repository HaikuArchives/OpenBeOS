/* some misc functions... */

#include <stdio.h>

#include "net_misc.h"

/* Basically use dump_ to see the address plus message on a line,
 * print_ to simply have the address printed with nothing else...
 */
 
void dump_ipv4_addr(char *msg, ipv4_addr *ad)
{
        uint8 *b = (uint8*)ad;
        printf("%s %d.%d.%d.%d\n", msg, b[0], b[1], b[2], b[3]);
}

void print_ipv4_addr(ipv4_addr *ad)
{
        uint8 *b = (uint8*)ad;
        printf("%d.%d.%d.%d", b[0], b[1], b[2], b[3]);
}

void dump_ether_addr(char *msg, ether_addr *ea)
{
        printf("%s %02x:%02x:%02x:%02x:%02x:%02x\n", msg,
                ea->addr[0], ea->addr[1], ea->addr[2],
                ea->addr[3], ea->addr[4], ea->addr[5]);
}

void print_ether_addr(ether_addr *ea)
{
	printf("%02x:%02x:%02x:%02x:%02x:%02x",
		ea->addr[0], ea->addr[1], ea->addr[2],
                ea->addr[3], ea->addr[4], ea->addr[5]);
}

void dump_buffer(char *buffer, int len) 
{
	uint8 *b = (uint8 *)buffer;
	int i;

	printf ("  ");
	for (i=0;i<len;i++) {
		if (i%16 == 0)
			printf("\n  ");
		if (i%2 == 0)
			printf("%02x ", b[i]);
		else
			printf(" %02x", b[i]);
	}
	printf("\n\n");
}

