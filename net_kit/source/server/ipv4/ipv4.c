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
#include "sys/protosw.h"
#include "sys/domain.h"

#ifdef _KERNEL_MODE
#include <KernelExport.h>
#include "net_server/core_module.h"
#include "net_proto.h"

#define m_free              core->m_free
#define m_freem             core->m_freem
#define m_adj               core->m_adj
#define m_prepend           core->m_prepend
#define in_pcballoc         core->in_pcballoc
#define in_pcbconnect       core->in_pcbconnect
#define in_pcbdisconnect    core->in_pcbdisconnect
#define in_pcbbind          core->in_pcbbind
#define soreserve           core->soreserve
#define sbappendaddr        core->sbappendaddr
#define in_pcblookup        core->in_pcblookup
#define sowakeup            core->sowakeup
#define in_pcbdetach        core->in_pcbdetach
#define rtfree              core->rtfree
#define rtalloc             core->rtalloc
#define ifa_ifwithdstaddr	core->ifa_ifwithdstaddr
#define ifa_ifwithnet       core->ifa_ifwithnet

static struct core_module_info *core = NULL;
#define IPV4_MODULE_PATH	"network/protocol/ipv4"
#else
#define IPV4_MODULE_PATH	"modules/interface/ipv4"
#endif

struct protosw *proto[IPPROTO_MAX];
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

	if (proto[ip->prot] && proto[ip->prot]->pr_input)
		return proto[ip->prot]->pr_input(buf, ip->hl * 4);
	else
		printf("proto[%d] = %p\n", ip->prot, proto[ip->prot]);
		
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

static void ipv4_init(void)
{
	memset(proto, 0, sizeof(struct protosw *) * IPPROTO_MAX);
#ifndef _KERNEL_MODE
	add_protosw(proto, NET_LAYER2);
#else
	core->add_protosw(proto, NET_LAYER2);
#endif
}

/*
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
		for (; ifa->ifa_next; ifa = ifa->ifa_next)
			continue;
		ifa->ifa_next = (struct ifaddr*)ia;
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

	sin.sin_family = AF_INET;
	if (dev->if_type == IFT_ETHER)
		sin.sin_addr.s_addr = htonl(0xc0a80085);
	if (dev->if_type == IFT_LOOP)
		sin.sin_addr.s_addr = htonl(0x7f000001);
	sin.sin_len = sizeof(sin);
	sin.sin_port = 0;

	return in_ifinit(dev, ia, &sin, 1);
}
*/

struct protosw my_proto = {
	"IPv4",
	IPV4_MODULE_PATH,
	0,
	NULL,
	IPPROTO_IP,
	0,
	NET_LAYER2,
	
	&ipv4_init,
	&ipv4_input,
	&ipv4_output,
	NULL,
	
	NULL,
	NULL
};

#ifndef _KERNEL_MODE

static void ipv4_protocol_init(void)
{
	add_domain(NULL, AF_INET);
	add_protocol(&my_proto, AF_INET);
}

struct protocol_info protocol_info = {
	"IPv4 Module",
	&ipv4_protocol_init
};

#else /* kernel setup */

static int k_init(void)
{
	if (!core)
		get_module(CORE_MODULE_PATH, (module_info**)&core);

	core->add_domain(NULL, AF_INET);
	core->add_protocol(&my_proto, AF_INET);
	
	return 0;	
}

static status_t ipv4_ops(int32 op, ...)
{
	dprintf("ipv4_ops:\n");
	switch (op) {
		case B_MODULE_INIT:
			k_init();
			break;
		case B_MODULE_UNINIT:
			break;
		default:
			return B_ERROR;
	}
	return B_OK;
}

static struct protocol_module_info my_module = {
	{
		IPV4_MODULE_PATH,
		B_KEEP_LOADED,
		ipv4_ops
	},
	
	/* ??? */
};

_EXPORT module_info *modules[] = {
	(module_info*) &my_module,
	NULL
};
			
#endif		 
