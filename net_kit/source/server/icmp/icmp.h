/* icmp.h
 * icmp for IPv4
 */

#include "mbuf.h"
#include "sys/socket.h"

#ifndef OBOS_ICMP_H
#define OBOS_ICMP_H

enum {
	ICMP_ECHO_RPLY 		= 0,
	ICMP_DEST_UNRCH		= 3,
	ICMP_SRC_QUENCH		= 4,
	ICMP_REDIRECT		= 5,
	ICMP_ECHO_RQST		= 8,
	ICMP_RTR_ADVERT		= 9,
	ICMP_RTR_SOL		= 10,
	ICMP_TIME_UP		= 11,
	ICMP_PARAM		= 12,
	/* XXX - add the remaining ones... */
};

typedef struct icmp_header {
	uint8	type;
	uint8	code;
	uint16	cksum;
} _PACKED icmp_header;

typedef struct icmp_echo {
	icmp_header hdr;
	uint16	id;
	uint16	seq;
} icmp_echo;

#endif /* OBOS_ICMP_H */

