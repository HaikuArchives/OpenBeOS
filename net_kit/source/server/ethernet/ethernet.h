/* ethernet.h
 * structures for ethernet packets
 */

#ifndef OBOS_ETHERNET_H
#define OBOS_ETHERNET_H

#include "net_misc.h"
#include "mbuf.h"

typedef struct ethernet_header {
	ether_addr dest;
	ether_addr src;
	uint16 	type;  /* MUST be above 1500 */
} ethernet_header;

typedef struct eth802_header {
        ether_addr dest;
        ether_addr src;
		uint16 	length;	/* max value is 1500 */
		uint8 	dsap;
		uint8 	ssap;
		uint8 	cntl;
		uint8 	org1;
		uint16 	org2;
		uint16	type;
} eth802_header;

int ethernet_input(struct mbuf *buf);

#endif /* OBOS_ETHERNET_H */

