/* udp.h
 */

#include "ipv4/ipv4.h"
#include "netinet/in.h"


#ifndef OBOS_UDP_H
#define OBOS_UDP_H

typedef struct udp_header {
	uint16 src_port;
	uint16 dst_port;
	uint16 length;
	uint16 cksum;
} _PACKED udp_header;

/* we calculate the UDP checksum oiver
 *	- a pseudo header
 *	- the udp header
 *	- the data
 *
 * Here we define a structure we overlay over the ip header to fill
 * in and use.
 */
typedef struct pudp_header {
	uint32		lead;  /* these 2 are set to zero... */
	uint32		lead2;
        uint8           zero;
        uint8           prot;
        uint16          length;
	struct in_addr	src;
	struct in_addr	dest;
} _PACKED pudp_header;

struct udpiphdr {
	struct pudp_header ip;
	struct udp_header  udp;
};

#endif /* OBOS_UDP_H */

