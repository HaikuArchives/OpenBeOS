/* if helper functions */

#include <kernel/OS.h>
#include <stdio.h>

#include "netinet/in.h"
#include "if.h"
#include "sys/socketvar.h"
#include "sys/sockio.h"
#include "netinet/in_var.h"
#include "net/route.h"

extern ifnet *devices;
extern int ndevs;
#define IFA_ROUTE	RTF_UP

/* XXX - Yuck! This is a hack to allow us to have access to a "primary"
 * in_ifaddr structure that we can use. This is needed as we don't currently
 * have a way of getting access to teh list kept in the ipv4 module.
 */
struct in_ifaddr *primary_addr = NULL;

struct ifnet *ifunit(char *name)
{
	ifnet *d = devices;

	for (d=devices;d;d = d->next)
		if (strcmp(d->if_name, name) == 0)
			return d;
	return NULL;
}

void dump_sockaddr(void *ptr)
{
	struct sockaddr *sa = (struct sockaddr *)ptr;
	uint8 *d = NULL;
	int i;

	switch (sa->sa_family) {
		case AF_LINK: {
			struct sockaddr_dl *sdl = (struct sockaddr_dl *)ptr;
			if (sdl->sdl_type == IFT_ETHER)
				printf("ETHERNET: ");

			printf("Interface ");
			d = &sdl->sdl_data[0];
			for (i=0;i<sdl->sdl_nlen;i++, d++) {
				printf("%c", *d);
			}
			printf(" has a link layer address of ");
			for (i=0;i<sdl->sdl_alen;i++, d++) {
				printf("%02x", *d);
				if (i< 5)
					printf(":");
			}
			break;
		}
		case AF_INET: {
			struct sockaddr_in *sin = (struct sockaddr_in *)ptr;
			struct in_addr ho;
			ho.s_addr = sin->sin_addr.s_addr;
			printf("IPv4: ");
			d = (uint8*)&ho.s_addr;
			for (i=0;i<4;i++, d++) {
				printf("%d", *d);
				if (i < 3)
					printf(".");
			}
			break;
		}
		default:
			printf("Unknown type... %d\n", sa->sa_family);
	}
}

void *protocol_address(struct ifnet *ifa, int family)
{
	struct ifaddr *a = ifa->if_addrlist;

	for (; a != NULL; a = a->ifn_next) {
		if (a->ifa_addr->sa_family == family) {
			if (family == AF_INET) {
				return &((struct sockaddr_in*)a->ifa_addr)->sin_addr;
			} else {
				return &a->ifa_addr->sa_data;
			}
		}
	}
	return NULL;
}

/*
 * Trim a mask in a sockaddr
 */
void in_socktrim(struct sockaddr_in *ap)
{
	char *cplim = (char *) &ap->sin_addr;
	char *cp = (char *) (&ap->sin_addr + 1);

	ap->sin_len = 0;
	while (--cp >= cplim)
		if (*cp) {
			(ap)->sin_len = cp - (char *) (ap) + 1;
			break;
		}
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

	if (!primary_addr)
		primary_addr = ia;

	return error;
}

#define equal(a1, a2) \
  (memcmp((caddr_t)(a1), (caddr_t)(a2), ((struct sockaddr *)(a1))->sa_len) == 0)

/*
 * Find an interface address specific to an interface best matching
 * a given address.
 */
struct ifaddr *ifaof_ifpforaddr(struct sockaddr *addr, 
				struct ifnet *ifp)
{
	struct ifaddr *ifa;
	char *cp, *cp2, *cp3;
	char *cplim;
	struct ifaddr *ifa_maybe = 0;
        uint af = addr->sa_family;

        if (af >= AF_MAX)
                return (NULL);

        for (ifa = ifp->if_addrlist; ifa != NULL; ifa = ifa->ifn_next) {
                if (ifa->ifa_addr->sa_family != af)
                        continue;
                ifa_maybe = ifa;
                if (ifa->ifa_netmask == 0) {
                        if (equal(addr, ifa->ifa_addr) ||
                            (ifa->ifa_dstaddr && equal(addr, ifa->ifa_dstaddr)))
                                return (ifa);
                        continue;
                }
                cp = addr->sa_data;
                cp2 = ifa->ifa_addr->sa_data;
                cp3 = ifa->ifa_netmask->sa_data;
                cplim = ifa->ifa_netmask->sa_len + (char *)ifa->ifa_netmask;
                for (; cp3 < cplim; cp3++)
                        if ((*cp++ ^ *cp2++) & *cp3)
                                break;
                if (cp3 == cplim)
                        return (ifa);
        }
        return (ifa_maybe);
}

struct ifaddr *ifa_ifwithdstaddr(struct sockaddr *addr)
{
	struct ifnet *ifp;
	struct ifaddr *ifa;

        for (ifp = devices; ifp != NULL; ifp = ifp->next)
            if (ifp->flags & IFF_POINTOPOINT)
                for (ifa = ifp->if_addrlist; ifa != NULL; ifa = ifa->ifn_next) {
                        if (ifa->ifa_addr->sa_family != addr->sa_family ||
                            ifa->ifa_dstaddr == NULL)
                                continue;
                        if (equal(addr, ifa->ifa_dstaddr))
                                return (ifa);
        }
        return (NULL);
}

struct ifaddr *ifa_ifwithaddr(struct sockaddr *addr)
{
	struct ifnet *ifp;
	struct ifaddr *ifa;

        for (ifp = devices; ifp != NULL; ifp = ifp->next)
            for (ifa = ifp->if_addrlist; ifa != NULL; ifa = ifa->ifn_next) {
                if (ifa->ifa_addr->sa_family != addr->sa_family)
                        continue;
                if (equal(addr, ifa->ifa_addr))
                        return (ifa);
                if ((ifp->flags & IFF_BROADCAST) && ifa->ifa_broadaddr &&
                    /* IP6 doesn't have broadcast */
                    ifa->ifa_broadaddr->sa_len != 0 &&
                    equal(ifa->ifa_broadaddr, addr))
                        return (ifa);
        }
        return (NULL);
}

/*
 * Find an interface on a specific network.  If many, choice
 * is most specific found.
 */
struct ifaddr *ifa_ifwithnet(struct sockaddr *addr)
{
	struct ifnet *ifp;
	struct ifaddr *ifa;
	ifaddr *ifa_maybe = 0;
        uint af = addr->sa_family;
        char *addr_data = addr->sa_data, *cplim;

	if (af == AF_LINK) {
		struct sockaddr_dl *sdl = (struct sockaddr_dl *)addr;
		if (sdl->sdl_index && sdl->sdl_index <= ndevs)
			/* XXX - hm, this is what we're supposed to do... */
                	//return (ifnet_addrs[sdl->sdl_index]);
			return NULL;
	}
        for (ifp = devices; ifp != NULL; ifp = ifp->next)
                for (ifa = ifp->if_addrlist; ifa != NULL; ifa = ifa->ifn_next) {
                        register char *cp, *cp2, *cp3;

                        if (ifa->ifa_addr->sa_family != af ||
                            ifa->ifa_netmask == 0)
                                next: continue;
                        cp = addr_data;
                        cp2 = ifa->ifa_addr->sa_data;
                        cp3 = ifa->ifa_netmask->sa_data;
                        cplim = (char *)ifa->ifa_netmask +
                                ifa->ifa_netmask->sa_len;
                        while (cp3 < cplim)
                                if ((*cp++ ^ *cp2++) & *cp3++)
                                    /* want to continue for() loop */
                                        goto next;
                        if (ifa_maybe == 0 ||
                            rn_refines((caddr_t)ifa->ifa_netmask,
                            (caddr_t)ifa_maybe->ifa_netmask))
                                ifa_maybe = ifa;
                }
        return (ifa_maybe);
}

