/* udp.h
 */

#ifndef OBOS_UDP_H
#define OBOS_UDP_H

#include "mbuf.h"

typedef struct udp_header {
	uint16 src_port;
	uint16 dst_port;
	uint16 length;
	uint16 cksum;
} _PACKED udp_header;

int udp_input(struct mbuf *buf);

#endif /* OBOS_UDP_H */

