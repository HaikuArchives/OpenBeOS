/* in_var.h */

#include "netinet/in.h"

#ifndef NETINET_IN_VAR_H
#define NETINET_IN_VAR_H

struct in_ifaddr {
	struct ifaddr 		ia_ifa;
	struct in_ifaddr 	*ia_next;

	uint32			ia_net;
	uint32			ia_netmask;
	uint32			ia_subnet;
	uint32			ia_subnetmask;
	struct in_addr 		ia_netbroadcast;
	struct sockaddr_in 	ia_addr;
	struct sockaddr_in 	ia_dstaddr;		/* broadcast address */
	struct sockaddr_in	ia_sockmask;
	/*XXX - milticast address list */
};
#define ia_ifp		ia_ifa.ifn
#define ia_flags	ia_ifa.ifa_flags
#define ia_broadaddr	ia_dstaddr

#define ifatoia(ifa)    ((struct in_ifaddr *)(ifa))
#define sintosa(sin)	((struct sockaddr *)(sin))

/* used to pass in additional information, such as aliases */
struct in_aliasreq {
	char ifa_name[IFNAMSIZ];
	struct sockaddr_in ifra_addr;
	struct sockaddr_in ifra_broadaddr;
#define ifra_dstaddr	ifra_broadaddr
	struct sockaddr_in ifra_mask;
};

/*
 * Given a pointer to an in_ifaddr (ifaddr),
 * return a pointer to the addr as a sockaddr_in.
 */
#define IA_SIN(ia) (&(((struct in_ifaddr *)(ia))->ia_addr))

int in_ifinit(struct ifnet *dev, struct in_ifaddr *ia, struct sockaddr_in *sin,
                int scrub);

#endif /* NETINET_IN_VAR_H */
