/* in.c */

#include <stdio.h>

#include "netinet/in.h"
#include "if.h"
#include "sys/socketvar.h"
#include "sys/sockio.h"
#include "netinet/in_var.h"
#include "net/route.h"

#define IFA_ROUTE	RTF_UP

struct in_ifaddr *get_primary_addr(void)
{
	return in_ifaddr;
}

int in_ifinit(struct ifnet *dev, struct in_ifaddr *ia, struct sockaddr_in *sin,
		int scrub)
{
	uint32 i = sin->sin_addr.s_addr;
	struct sockaddr_in oldsin;
	int error;
	int flags = RTF_UP;

	oldsin = ia->ia_addr;
	ia->ia_addr = *sin;

	if (dev && dev->ioctl) {
		error = (*dev->ioctl)(dev, SIOCSIFADDR, (caddr_t)ia);
		if (error) {
			ia->ia_addr = oldsin;
			return error;
		}
	}

	if (scrub) {
		ia->ia_ifa.ifa_addr = (struct sockaddr*)&oldsin;
		//in_ifscrub(ifp, ia);
		ia->ia_ifa.ifa_addr = (struct sockaddr*)&ia->ia_addr;
	}

	if (IN_CLASSA(i))
		ia->ia_netmask = IN_CLASSA_NET;
	else if (IN_CLASSB(i))
		ia->ia_netmask = IN_CLASSB_NET;
	else
		ia->ia_netmask = IN_CLASSC_NET;

	if (ia->ia_subnetmask == 0) {
		ia->ia_subnetmask = ia->ia_netmask;
		ia->ia_sockmask.sin_addr.s_addr = ia->ia_subnetmask;
	} else
		ia->ia_netmask &= ia->ia_subnetmask;
	ia->ia_net = i & ia->ia_netmask;
	ia->ia_subnet = i & ia->ia_subnetmask;
	in_socktrim(&ia->ia_sockmask);

	ia->ia_ifa.ifa_metric = dev->if_metric;
	if (dev->flags & IFF_BROADCAST) {
		ia->ia_broadaddr.sin_addr.s_addr = htonl(ia->ia_net | ~ia->ia_subnetmask);
		ia->ia_netbroadcast.s_addr = htonl(ia->ia_net | ~ia->ia_netmask);
	} else if (dev->flags & IFF_LOOPBACK) {
		ia->ia_ifa.ifa_dstaddr = ia->ia_ifa.ifa_addr;
		flags |= RTF_HOST;
	} else if (dev->flags & IFF_POINTOPOINT) {
		if (ia->ia_dstaddr.sin_family != AF_INET)
			return 0;
		flags |= RTF_HOST;
	}

	error = rtinit(&(ia->ia_ifa), (int)RTM_ADD, flags);
	if (error == 0)
		ia->ia_flags |= IFA_ROUTE;

	/* XXX - Multicast address list */

	if (!in_ifaddr)
		in_ifaddr = ia;

	return error;
}

int in_control(struct socket *so, int cmd, caddr_t data, struct ifnet *ifp)
{
	struct ifreq *ifr = (struct ifreq*)data;
	struct in_ifaddr *ia = NULL;
	struct ifaddr *ifa;
	struct in_ifaddr *oia;
	struct in_aliasreq *ifra = (struct in_aliasreq*)data;
	struct sockaddr_in oldaddr;
	int error, hostIsNew, maskIsNew;
	long i;
	
	if (ifp) /* we need to find the in_ifaddr */
		for (ia = in_ifaddr;ia; ia = ia->ia_next)
			if (ia->ia_ifp == ifp)
				break;
	
	switch (cmd) {
		case SIOCSIFADDR:
		case SIOCSIFNETMASK:
		case SIOCSIFDSTADDR:
			if (!ifp) {
				printf("No interface pointer!\n");
				return EINVAL;
			}
			if (ia == NULL) {
				oia = (struct in_ifaddr*)malloc(sizeof(struct in_ifaddr));
				if (oia == NULL)
					return ENOMEM;
				memset(oia, 0, sizeof(*oia));
				if ((ia = in_ifaddr)) {
					/* we've got other structures - add at end */
					for (; ia->ia_next; ia = ia->ia_next)
						continue;
					ia->ia_next = oia;
				} else
					in_ifaddr = oia;
				
				ia = oia;
				if ((ifa = ifp->if_addrlist)) {
					for (; ifa->ifa_next; ifa = ifa->ifa_next)
						continue;
					ifa->ifa_next = (struct ifaddr*)ia;
				} else
					ifp->if_addrlist =  (struct ifaddr*)ia;
				
				ia->ia_ifa.ifa_addr     = (struct sockaddr*) &ia->ia_addr;
				ia->ia_ifa.ifa_dstaddr  = (struct sockaddr*) &ia->ia_dstaddr;
				ia->ia_ifa.ifa_netmask  = (struct sockaddr*) &ia->ia_sockmask;
				ia->ia_sockmask.sin_len = 8;
				if (ifp->flags & IFF_BROADCAST) {
					ia->ia_broadaddr.sin_len = sizeof(ia->ia_addr);
					ia->ia_broadaddr.sin_family = AF_INET;
				}
				ia->ia_ifp = ifp;
			}
			break;
	}
	
	switch(cmd) {
		case SIOCSIFADDR:
			return in_ifinit(ifp, ia, (struct sockaddr_in*)&ifr->ifr_addr, 1);
		case SIOCSIFNETMASK:
			i = ifra->ifra_addr.sin_addr.s_addr;
			ia->ia_subnetmask = ntohl(ia->ia_sockmask.sin_addr.s_addr = i);
			break;
		case SIOCSIFDSTADDR:
			if ((ifp->flags & IFF_POINTOPOINT) == 0)
				return EINVAL;
			oldaddr = ia->ia_dstaddr;
			ia->ia_dstaddr = *(struct sockaddr_in*)&ifr->ifr_dstaddr;
			/* update the interface if required */
			if (ifp->ioctl) {
				error = ifp->ioctl(ifp, SIOCSIFDSTADDR, (caddr_t) ia);
				if (error) {
					ia->ia_dstaddr = oldaddr;
					return error;
				}
			}
			/* change the routing info if it's been set */
			if (ia->ia_flags & IFA_ROUTE) {
				ia->ia_ifa.ifa_dstaddr = (struct sockaddr*)&oldaddr;
				rtinit(&(ia->ia_ifa), RTM_DELETE, RTF_HOST);
				ia->ia_ifa.ifa_dstaddr = (struct sockaddr*)&ia->ia_dstaddr;
				rtinit(&(ia->ia_ifa), RTM_ADD, RTF_HOST|RTF_UP);
			}
		default:
			/* if we don't have enough to do the default, return */
			if (ifp == NULL || ifp->ioctl == NULL)
				return EINVAL; /* XXX - should be ENOTSUPP */
			/* send to the card and let it process it */
			return ifp->ioctl(ifp, cmd, data);
	}
	return 0;
}
