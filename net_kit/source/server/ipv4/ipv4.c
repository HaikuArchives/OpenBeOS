/* ipv4.c
 * simple ipv4 implementation
 */

#include <stdio.h>
#include <unistd.h>
#include <kernel/OS.h>
#include <malloc.h>

#include "netinet/in.h"
#include "netinet/in_var.h"
#include "ipv4/ipv4.h"
#include "ipv4/ipv4_var.h"	/* for stats */
#include "protocols.h"
#include "net_module.h"
#include "mbuf.h"

loaded_net_module *net_modules;
int *prot_table;
static struct ipstat	ipstat;
static uint16 ip_identifier = 0; /* XXX - set this better */
struct in_ifaddr *in_ifaddr;

#if SHOW_DEBUG
static void dump_ipv4_header(struct mbuf *buf)
{
	ipv4_header *ip = mtod(buf, ipv4_header *);

        printf("IPv4 Header :\n");
        printf("            : version       : %d\n", ip->ver);
        printf("            : header length : %d\n", ip->hl * 4);
        printf("            : tos           : %d\n", ip->tos);
        printf("            : total length  : %d\n", ntohs(ip->length));
        printf("            : id            : %d\n", ntohs(ip->id));
        printf("            : flags         : 0x%02x\n", IPV4_FLAGS(ip));
        printf("            : frag offset   : %d\n", ntohs(IPV4_FRAG(ip)));
        printf("            : ttl           : %d\n", ip->ttl);
        dump_ipv4_addr("            : src address   :", &ip->src);
        dump_ipv4_addr("            : dst address   :", &ip->dst);

        printf("            : protocol      : ");

        switch(ip->prot) {
                case IPPROTO_ICMP:
                        printf("ICMP\n");
                        break;
                case IPPROTO_UDP: {
                        printf("UDP\n");
                        break;
                }
                case IPPROTO_TCP:
                        printf("TCP\n");
                        break;
                default:
                        printf("unknown (0x%02x)\n", ip->prot);
        }
}
#endif

int ipv4_input(struct mbuf *buf, int hdrlen)
{
	ipv4_header *ip = mtod(buf, ipv4_header *);

#if SHOW_DEBUG
        dump_ipv4_header(buf);
#endif

	atomic_add(&ipstat.ips_total, 1);

	if (ip->ver != 4) {
		printf("Wrong IP version!\n");
		atomic_add(&ipstat.ips_badvers, 1);
		m_freem(buf);
		return 0;
	}

	if (in_cksum(buf, ip->hl * 4, 0) != 0) {
		printf("Bogus checksum! Discarding packet.\n");
		atomic_add(&ipstat.ips_badsum, 1);
		m_freem(buf);
		return 0;
	}

	/* we put the length into host order here... */
	ip->length = ntohs(ip->length);

	/* Strip excess data from mbuf */
	if (buf->m_len > ip->length)
		 m_adj(buf, ip->length - buf->m_len);

	if (net_modules[prot_table[ip->prot]].mod->input)
		net_modules[prot_table[ip->prot]].mod->input(buf, ip->hl * 4);

	return 0; 
}

int ipv4_output(struct mbuf *buf, struct mbuf *opt, struct route *ro,
		int flags, void *optp)
{
	ipv4_header *ip = mtod(buf, ipv4_header*);
	struct route iproute; /* temporary route we may need */
	struct sockaddr_in *dst; /* destination address */
	struct in_ifaddr *ia;
	int error = 0;
	struct ifnet *ifp = NULL;

	/* route the packet! */
	if (!ro) {
		ro = &iproute;
		memset(ro, 0, sizeof(iproute));
	}
	dst = (struct sockaddr_in *)&ro->ro_dst;

	if (ro && ro->ro_rt && 
	    ((ro->ro_rt->rt_flags & RTF_UP) == 0 || /* route isn't available */
	     dst->sin_addr.s_addr != ip->dst.s_addr)) { /* not same ip address */
		RTFREE(ro->ro_rt);
		ro->ro_rt = NULL;
	}
	if (ro->ro_rt == NULL) {
		dst->sin_family = AF_INET;
		dst->sin_len = sizeof(*dst);
		dst->sin_addr = ip->dst;
	}
	if (flags & IP_ROUTETOIF) {
		/* we're routing to an interface... */
		if (!(ia = ifatoia(ifa_ifwithdstaddr(sintosa(dst)))) &&
		    !(ia = ifatoia(ifa_ifwithnet(sintosa(dst))))) {
			error = ENETUNREACH;
			goto bad;
		}
	} else {
		/* normal routing */
		if (ro->ro_rt == NULL)
			rtalloc(ro);
		if (ro->ro_rt == NULL) {
#if SHOW_DEBUG
			printf("EHOSTUNREACH\n");
#endif
			error = EHOSTUNREACH;
			goto bad;
		}
#if SHOW_DEBUG
		printf("Host is reachable...\n");
#endif
		ia = ifatoia(ro->ro_rt->rt_ifa);
		ifp = ro->ro_rt->rt_ifp;
		atomic_add(&ro->ro_rt->rt_use, 1);
		if (ro->ro_rt->rt_flags & RTF_GATEWAY)
			dst = (struct sockaddr_in *) ro->ro_rt->rt_gateway;
	}
	/* make sure we have an outgoing address. if not yet specified, use the
	 * address of the outgoing interface
	 */
	if (ip->src.s_addr == INADDR_ANY)
		ip->src = IA_SIN(ia)->sin_addr;

	/* XXX - deal with broadcast */

#if SHOW_ROUTE
	/* This just shows which interface we're planning on using */
	printf("Sending to address ");
	print_ipv4_addr(&ip->dst);
	printf(" via device %s using source ", ifp->if_name);
	dump_ipv4_addr("address ", &ip->src);
#endif

	ip->ver = 4;
	ip->hl = 5; /* XXX - only if we have no options following...hmmm fix this! */

	if (ip->length <= ifp->if_mtu) { /* can we send it? */
		ip->length = htons(ip->length);
		ip->id = htons(ip_identifier++);
		ip->hdr_cksum = 0;
		ip->hdr_cksum = in_cksum(buf, (ip->hl * 4), 0);
		/* now send the packet! */
		error = (*ifp->output)(ifp, buf, (struct sockaddr *)dst, ro->ro_rt);
	}

done:
	if (ro == &iproute && /* we used our own variable */
	    (flags & IP_ROUTETOIF) == 0 && /* we didn't route to an iterface */
	    ro->ro_rt) { /* we have an allocated route */
		RTFREE(ro->ro_rt); /* free the route */
	}

	return error;
bad:
	m_free(buf);
	goto done;
}

int ipv4_init(loaded_net_module *ln, int *pt)
{
	net_modules = ln;
	prot_table = pt;

	return 0;
}

int ipv4_dev_init(ifnet *dev)
{
	struct in_ifaddr *oia;
	struct in_ifaddr *ia;
	struct sockaddr_in sin;
	struct ifaddr *ifa;

	if (!dev)
		return EINVAL;

	oia = (struct in_ifaddr *)malloc(sizeof(struct in_ifaddr));

	if (!oia)
		return ENOMEM;

	memset(oia, 0, sizeof(struct in_ifaddr));

	if ((ia = in_ifaddr)) {
		for (;ia->ia_next; ia = ia->ia_next)
			continue;
		ia->ia_next = oia;
	} else 
		in_ifaddr = oia;

	ia = oia;

	if ((ifa = dev->if_addrlist)) {
		for (; ifa->ifn_next; ifa = ifa->ifn_next)
			continue;
		ifa->ifn_next = (struct ifaddr*)ia;
	} else
		dev->if_addrlist = (struct ifaddr*)ia;

	ia->ia_ifa.ifa_addr     = (struct sockaddr*) &ia->ia_addr;
	ia->ia_ifa.ifa_dstaddr  = (struct sockaddr*) &ia->ia_dstaddr;
	ia->ia_ifa.ifa_netmask  = (struct sockaddr*) &ia->ia_sockmask;
	ia->ia_sockmask.sin_len = 8;
	if (dev->flags & IFF_BROADCAST) {
		ia->ia_broadaddr.sin_len = sizeof(ia->ia_addr);
		ia->ia_broadaddr.sin_family = AF_INET;
	}
	ia->ia_ifp = dev;

	/* we now have a structure ready to accept an address, mask and so on... */
	sin.sin_family = AF_INET;
	if (dev->if_type == IFT_ETHER)
		sin.sin_addr.s_addr = htonl(0xc0a80085);
	if (dev->if_type == IFT_LOOP)
		sin.sin_addr.s_addr = htonl(0x7f000001);
	sin.sin_len = sizeof(sin);
	sin.sin_port = 0;

	return in_ifinit(dev, ia, &sin, 1);
}

net_module net_module_data = {
	"IPv4 module",
	NS_IPV4,
	NET_LAYER2,
        0,      /* users can't create sockets in this module! */
        0,
	0,

	&ipv4_init,
	&ipv4_dev_init,
	&ipv4_input, 
	&ipv4_output,
	NULL,	
	NULL
};

