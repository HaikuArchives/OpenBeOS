/* udp.c
 */

#include <stdio.h>

#include "mbuf.h"
#include "udp.h"
#include "net_misc.h"

int udp_input(struct mbuf*buf)
{
	udp_header *udp = mtod(buf, udp_header *);

	printf("udp_header  :\n");
	printf("            : src_port      : %d\n", ntohs(udp->src_port));
        printf("            : dst_port      : %d\n", ntohs(udp->dst_port));
	printf("            : udp length    : %d bytes\n", ntohs(udp->length));

	return 0;
} 
