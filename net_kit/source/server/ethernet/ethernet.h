/* ethernet.h
 * structures for ethernet packets
 */

#include "netinet/in.h"
 
#ifndef OBOS_ETHERNET_H
#define OBOS_ETHERNET_H

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

/* This structure keeps ethernet specific details in addition to the generic ifnet stuff...
 *
 * NB NB the ifnet structure MUST be the first element as we'll use it for casting
 *       from an ifnet* to a struct ether_device*
 */
struct ether_device {
	struct ifnet	ifn;		/* our ifnet structure... */
	struct ether_device *next;      /* next ether_device */
	int		devid;		/* io handle */
	ether_addr 	e_addr;
	struct in_addr	i_addr; /* ipv4 only :( */
	ether_addr	*e_multi;	/* we can have a list of multicast ethernet
					 * addresses we respond to, so we keep a list here
					 * which is e_multilen long (starts at 0)
					 */
	int		e_multilen;
};

#define ed_name		ifn.name
#define ed_type		ifn.if_type
#define ed_unit		ifn.unit
#define ed_if_addrlist	ifn.if_addrlist
#define ed_rx_thread	ifn.rx_thread
#define ed_tx_thread	ifn.tx_thread
#define ed_txq		ifn.txq
#define ed_devid	ifn.devid
#define ed_hdrlen	ifn.if_hdrlen
#define ed_addrlen	ifn.if_addrlen

#define ETHERMTU 1500

#endif /* OBOS_ETHERNET_H */

