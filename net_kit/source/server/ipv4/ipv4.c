/* ipv4.c
 * simple ipv4 implementation
 */

#include <stdio.h>
#include <unistd.h>
#include <kernel/OS.h>
#include <sys/time.h>

#include "netinet/in.h"
#include "netinet/in_var.h"
#include "netinet/in_systm.h"
#include "netinet/ip.h"
#include "netinet/ip_var.h"
#include "protocols.h"
#include "net_module.h"
#include "mbuf.h"
#include "sys/protosw.h"
#include "sys/domain.h"

#ifdef _KERNEL_MODE
#include <KernelExport.h>
#include "ipv4_module.h"
#include "net_server/core_module.h"

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

#else
#define IPV4_MODULE_PATH	"modules/interface/ipv4"
#endif

struct protosw *proto[IPPROTO_MAX];
static uint16 ip_identifier = 0;
struct in_ifaddr *in_ifaddr;

#if SHOW_DEBUG
static void dump_ipv4_header(struct mbuf *buf)
{
	struct ip *ip = mtod(buf, struct ip *);

	printf("IPv4 Header :\n");
	printf("            : version       : %d\n", ip->ip_v);
	printf("            : header length : %d\n", ip->ip_hl * 4);
	printf("            : tos           : %d\n", ip->ip_tos);
	printf("            : total length  : %d\n", ntohs(ip->ip_len));
	printf("            : id            : %d\n", ntohs(ip->ip_id));
//	printf("            : flags         : 0x%02x\n", IPV4_FLAGS(ip));
//	printf("            : frag offset   : %d\n", ntohs(IPV4_FRAG(ip)));
	printf("            : ttl           : %d\n", ip->ip_ttl);
	dump_ipv4_addr("            : src address   :", &ip->ip_src);
	dump_ipv4_addr("            : dst address   :", &ip->ip_dst);

	printf("            : protocol      : ");

	switch(ip->ip_p) {
		case IPPROTO_ICMP:
			printf("ICMP\n");
			break;
		case IPPROTO_UDP:
			printf("UDP\n");
			break;
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
	struct ip *ip = mtod(buf, struct ip *);

#if SHOW_DEBUG
        dump_ipv4_header(buf);
#endif

	atomic_add(&ipstat.ips_total, 1);

	if (ip->ip_v != IPVERSION) {
		printf("Wrong IP version!\n");
		atomic_add(&ipstat.ips_badvers, 1);
		m_freem(buf);
		return 0;
	}

	if (in_cksum(buf, ip->ip_hl * 4, 0) != 0) {
		printf("Bogus checksum! Discarding packet.\n");
		atomic_add(&ipstat.ips_badsum, 1);
		m_freem(buf);
		return 0;
	}

	/* we put the length into host order here... */
	ip->ip_len = ntohs(ip->ip_len);

	/* Strip excess data from mbuf */
	if (buf->m_len > ip->ip_len)
		 m_adj(buf, ip->ip_len - buf->m_len);

	if (proto[ip->ip_p] && proto[ip->ip_p]->pr_input) {
		return proto[ip->ip_p]->pr_input(buf, ip->ip_hl * 4);
	} else
		printf("proto[%d] = %p\n", ip->ip_p, proto[ip->ip_p]);

	return 0; 
}

int ipv4_output(struct mbuf *buf, struct mbuf *opt, struct route *ro,
                int flags, void *optp)
{
	struct ip *ip = mtod(buf, struct ip*);
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
	     dst->sin_addr.s_addr != ip->ip_dst.s_addr)) { /* not same ip address */
		RTFREE(ro->ro_rt);
		ro->ro_rt = NULL;
	}
	if (ro->ro_rt == NULL) {
		dst->sin_family = AF_INET;
		dst->sin_len = sizeof(*dst);
		dst->sin_addr = ip->ip_dst;
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
			printf("EHOSTUNREACH\n");
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
	if (ip->ip_src.s_addr == INADDR_ANY)
		ip->ip_src = IA_SIN(ia)->sin_addr;

	/* XXX - deal with broadcast */

#if SHOW_ROUTE
	/* This just shows which interface we're planning on using */
	printf("Sending to address ");
	printf("%08lx", ip->ip_dst.s_addr);
	printf(" via device %s using source ", ifp->if_name);
	printf("%08lx\n", ip->ip_src.s_addr);
#endif

	ip->ip_v = IPVERSION;
	ip->ip_hl = 5; /* XXX - only if we have no options following...hmmm fix this! */

	if (ip->ip_len <= ifp->if_mtu) { /* can we send it? */
		ip->ip_len = htons(ip->ip_len);
/* XXX - locking!! */		
		ip->ip_id = htons(ip_identifier++);
/* XXX - unlock me !! */
		ip->ip_sum = 0;
		ip->ip_sum = in_cksum(buf, (ip->ip_hl * 4), 0);
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

uint16 ip_id(void)
{
/* XXX - locking */
	return ip_identifier++;
/* XXX - unlocking */
}

static int ipv4_ctloutput(int op, struct socket *so, int level,
                        int optnum, struct mbuf **mp)
{
	struct inpcb *inp = sotoinpcb(so);
	struct mbuf *m = *mp;
	int optval;
	int error = 0;
	
	if (level != IPPROTO_IP) {
		error = EINVAL;
		if (op == PRCO_SETOPT && *mp)
			m_free(*mp);
	} else {
		switch(op) {
			case PRCO_SETOPT:
				switch(optnum) {
					case IP_OPTIONS:
						/* process options... */
						break;
					case IP_TOS:
					case IP_TTL:
					case IP_RECVOPTS:
					case IP_RECVRETOPTS:
					case IP_RECVDSTADDR:
						if (m->m_len != sizeof(int))
							error = EINVAL;
						else {
							optval = *mtod(m, int*);
							switch (optnum) {
								case IP_TOS:
									inp->inp_ip.ip_tos = optval;
									break;
								case IP_TTL:
									inp->inp_ip.ip_ttl = optval;
									break;
#define OPTSET(bit) \
	if (optval) \
		inp->inp_flags |= bit; \
	else \
		inp->inp_flags &= ~bit;
		
								case IP_RECVOPTS:
									OPTSET(INP_RECVOPTS);
									break;
								case IP_RECVRETOPTS:
									OPTSET(INP_RECVRETOPTS);
									break;
								case IP_RECVDSTADDR:
									OPTSET(INP_RECVDSTADDR);
									break;
							}
						}
						break;
//freeit:
					default:
						error = EINVAL;
						break;
				}
				if (m)
					m_free(m);
				break;
			case PRCO_GETOPT:
				switch(optnum) {
					/* XXX - add the code here */		
					default:
						error = ENOPROTOOPT;
						break;
				}
				break;
		}
	}
	return error;
}

static void ipv4_init(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	if (ip_identifier == 0)
		ip_identifier = tv.tv_sec & 0xffff;

	memset(proto, 0, sizeof(struct protosw *) * IPPROTO_MAX);
#ifndef _KERNEL_MODE
	add_protosw(proto, NET_LAYER2);
#else
	core->add_protosw(proto, NET_LAYER2);
#endif
}

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
	NULL,             /* pr_userreq */
	NULL,             /* pr_sysctl */
	&ipv4_ctloutput,
	
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

static struct ipv4_module_info my_module = {
		{
		IPV4_MODULE_PATH,
		B_KEEP_LOADED,
		ipv4_ops
		},
		ipv4_output,
		ip_id,
		ipv4_ctloutput
};

_EXPORT module_info *modules[] = {
	(module_info*) &my_module,
	NULL
};
			
#endif		 
