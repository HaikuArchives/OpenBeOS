/* ipv4.c
 * simple ipv4 implementation
 */

#ifndef _KERNEL_MODE
#include <stdio.h>
#endif
#include <unistd.h>
#include <kernel/OS.h>
#include <sys/time.h>

#include "netinet/in.h"
#include "netinet/in_var.h"
#include "netinet/in_systm.h"
#include "netinet/ip.h"
#include "netinet/ip_var.h"
#include "netinet/in_pcb.h"
#include "protocols.h"
#include "net_module.h"
#include "mbuf.h"
#include "sys/protosw.h"
#include "sys/domain.h"

#include "ipv4_module.h"

#ifdef _KERNEL_MODE
#include <KernelExport.h>
#include "net_server/core_module.h"
#include "net_server/core_funcs.h"

static struct core_module_info *core = NULL;

#endif	/* _KERNEL_MODE */

struct protosw *proto[IPPROTO_MAX];
struct in_ifaddr *in_ifaddr;
uint16 ip_id = 0;
int ipforwarding = 0; /* we don't forward IP packets by default... */

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
			printf("unknown (0x%02x)\n", ip->ip_p);
	}
}
#endif


/* IP Options variables and structures... */
int ip_nhops = 0;
static struct ip_srcrt {
	struct in_addr dst;
	char nop;
	char srcopt[IPOPT_OFFSET + 1];
	struct in_addr route[MAX_IPOPTLEN / sizeof(struct in_addr)];
} ip_srcrt;

/*
 * Strip out IP options, at higher
 * level protocol in the kernel.
 * Second argument is buffer to which options
 * will be moved, and return value is their length.
 * XXX should be deleted; last arg currently ignored.
 */
void ip_stripoptions(struct mbuf *m, struct mbuf *mopt)
{
	int i;
	struct ip *ip = mtod(m, struct ip *);
	caddr_t opts;
	int olen;

printf("ip_stripoptions\n");

	olen = (ip->ip_hl<<2) - sizeof (struct ip);
	opts = (caddr_t)(ip + 1);
	i = m->m_len - (sizeof (struct ip) + olen);
	memcpy(opts, opts  + olen, (unsigned)i);
	m->m_len -= olen;
	if (m->m_flags & M_PKTHDR)
		m->m_pkthdr.len -= olen;
	ip->ip_hl = sizeof(struct ip) >> 2;
}

int ip_dooptions(struct mbuf *m)
{
	struct ip *ip = mtod(m, struct ip*);
//	u_char *cp;
	struct in_addr dst;
printf("ip_dooptions\n");	
	dst = ip->ip_dst;
// XXX - complete me!	
	return 1;
}

struct mbuf * ip_srcroute(void)
{
	struct in_addr *p, *q;
	struct mbuf *m;

printf("ip_srcroute\n");
	
	if (ip_nhops == 0)
		return NULL;
	m = m_get(MT_SOOPTS);
	if (!m)
		return NULL;
#define OPTSIZ (sizeof(ip_srcrt.nop) + sizeof(ip_srcrt.srcopt))

	m->m_len = ip_nhops * sizeof(struct in_addr) + sizeof(struct in_addr) + OPTSIZ;
	
	p = &ip_srcrt.route[ip_nhops-1];
	*(mtod(m, struct in_addr*)) = *p--;
	ip_srcrt.nop = IPOPT_NOP;
	ip_srcrt.srcopt[IPOPT_OFFSET] = IPOPT_MINOFF;
	memcpy(mtod(m, caddr_t) + sizeof(struct in_addr), &ip_srcrt.nop, OPTSIZ);
	q = (struct in_addr*)(mtod(m, caddr_t) + sizeof(struct in_addr) + OPTSIZ);
#undef OPTSIZ

	while (p >= ip_srcrt.route) {
		*q++ = *q--;
	}
	
	*q = ip_srcrt.dst;
	return m;
}

int ipv4_input(struct mbuf *m, int hdrlen)
{
	struct ip *ip;
	struct in_ifaddr *ia = get_primary_addr();
	int hlen;
	
//printf("ipv4_input\n");	

#if SHOW_DEBUG
	dump_ipv4_header(buf);
#endif
	if (!m)
		return -1;
	/* If we don't have a pointer to our IP addresses we can't go on */
	if (!ia)
		goto bad;

	ipstat.ips_total++;
	/* Get the whole header in the first mbuf */
	if (m->m_len < sizeof(struct ip) && 
	    (m = m_pullup(m, sizeof(struct ip))) == NULL) {
		ipstat.ips_toosmall++;
		return -1;
	}
	ip = mtod(m, struct ip *);

	/* Check IP version... */
	if (ip->ip_v != IPVERSION) {
		printf("Wrong IP version! %d\n", ip->ip_v);
		ipstat.ips_badvers++;
		goto bad;
	}
	/* Figure out of header length */
	hlen = ip->ip_hl << 2;
	/* Check we're at least the minimum possible length */
	if (hlen < sizeof(struct ip)) {
		ipstat.ips_badhlen++;
		goto bad;
	}
	/* Check again we have the entire header in the first mbuf */
	if (hlen > m->m_len) {
		if ((m = m_pullup(m, hlen)) == NULL) {
			ipstat.ips_badhlen++;
			goto bad;
		}
		ip = mtod(m, struct ip *);
	}

	/* Checksum (should be 0) */	
	if ((ip->ip_sum = in_cksum(m, hlen, 0)) != 0) {
		printf("ipv4_input: checksum failed\n");
		ipstat.ips_badsum++;
		goto bad;
	}

	/* we put the length into host order here... */
	ip->ip_len = ntohs(ip->ip_len);
	/* sanity check. Datagram MUST be longer than the header! */
	if (ip->ip_len < hlen) {
		ipstat.ips_badhlen++;
		goto bad;
	}
	ip->ip_id = ntohs(ip->ip_id);
	ip->ip_off = ntohs(ip->ip_off);
	
	/* the first mbuf should be the packet hdr, so check it's length */
	if (m->m_pkthdr.len < ip->ip_len) {
		ipstat.ips_tooshort++;
		goto bad;
	}

	/* Strip excess data from mbuf */
	if (m->m_pkthdr.len > ip->ip_len) {
		if (m->m_len == m->m_pkthdr.len) {
			m->m_len = ip->ip_len;
			m->m_pkthdr.len = ip->ip_len;
		} else 
			m_adj(m, ip->ip_len - m->m_pkthdr.len);
	}

	/* options processing */	
	ip_nhops = 0;
	if (hlen > sizeof(struct ip) && ip_dooptions(m))
		return 0;
	
	for (;ia; ia = ia->ia_next) {
		if (IA_SIN(ia)->sin_addr.s_addr == ip->ip_dst.s_addr)
			goto ours;
		
		if (ia->ia_ifp == m->m_pkthdr.rcvif && 
		    (ia->ia_ifp->if_flags & IFF_BROADCAST)) {
			uint32 t;
			
			if (satosin(&ia->ia_broadaddr)->sin_addr.s_addr == ip->ip_dst.s_addr)
				goto ours;
			if (ip->ip_dst.s_addr == ia->ia_netbroadcast.s_addr)
				goto ours;
			t = ntohl(ip->ip_dst.s_addr);
			if (t == ia->ia_subnet)
				goto ours;
			if (t == ia->ia_net)
				goto ours;
		}
	}
	
	if (ip->ip_dst.s_addr == INADDR_BROADCAST)
		goto ours;
	if (ip->ip_dst.s_addr == INADDR_ANY)
		goto ours;
	
	if (ipforwarding == 0) {
		ipstat.ips_cantforward++;
		m_freem(m);
	}
	/* XXX - Add ip_forward routine and call it here */
	return -1;
ours:
	/* If offset or IF_MF are set, datagram must be reassembled.
	 * Otherwise, we don't need to do anything.
	 */
	/* XXX - add refragment here... */
	ip->ip_len -= hlen;

	ipstat.ips_delivered++;
	if (proto[ip->ip_p] && proto[ip->ip_p]->pr_input) {
		return proto[ip->ip_p]->pr_input(m, hlen);
	} else {
		printf("proto[%d] = %p\n", ip->ip_p, proto[ip->ip_p]);
		goto bad;
	}
	
	return 0; 
bad:
	m_freem(m);
	return -1;
}

int ipv4_output(struct mbuf *buf, struct mbuf *opt, struct route *ro,
                int flags, void *optp)
{
	struct ip *ip = mtod(buf, struct ip*);
	struct route iproute; /* temporary route we may need */
	struct sockaddr_in *dst; /* destination address */
	struct in_ifaddr *ia;
	int error = 0, hlen = sizeof(struct ip);
	struct ifnet *ifp = NULL;

	/* handle options... */
	if (opt) {
		printf("We have options!!!\n");
	}

	ip = mtod(buf, struct ip*);
	
	if ((flags & (IP_FORWARDING | IP_RAWOUTPUT)) == 0) {
		ip->ip_v = IPVERSION;
		ip->ip_off &= IP_DF;
		ip->ip_id = htons(ip_id++);
		ip->ip_hl = hlen >> 2;
		ipstat.ips_localout++;
	} else
		hlen = ip->ip_hl << 2;
		
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
			ipstat.ips_noroute++;
			error = ENETUNREACH;
			goto bad;
		}
		ifp = ia->ia_ifp;
		ip->ip_ttl = 1;
	} else {
		/* normal routing */
		if (ro->ro_rt == NULL)
			rtalloc(ro);
		if (ro->ro_rt == NULL) {
			ipstat.ips_noroute++;
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

	if ((in_broadcast(dst->sin_addr, ifp))) {
		if ((ifp->if_flags & IFF_BROADCAST) == 0) {
			error = EADDRNOTAVAIL;
			goto bad;
		}
		if ((flags & IP_ALLOWBROADCAST) == 0) {
			error = EACCES;
			goto bad;
		}
		if (ip->ip_len > ifp->if_mtu) {
			error = EMSGSIZE;
			goto bad;
		}
		buf->m_flags |= M_BCAST;
	} else 
		buf->m_flags &= ~M_BCAST;

#if SHOW_ROUTE
	/* This just shows which interface we're planning on using */
	printf("Sending to address ");
	printf("%08lx", ntohl(ip->ip_dst.s_addr));
	printf(" via device %s using source ", ifp->if_name);
	printf("%08lx\n", ntohl(ip->ip_src.s_addr));
#endif

	if (ip->ip_len <= ifp->if_mtu) {
		ip->ip_len = htons(ip->ip_len);
		ip->ip_off = htons(ip->ip_off);
		ip->ip_sum = 0;
		ip->ip_sum = in_cksum(buf, hlen, 0);
		/* now send the packet! */
		error = (*ifp->output)(ifp, buf, (struct sockaddr *)dst, ro->ro_rt);
		goto done;
	}

	/* Deal with fragmentation... */
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

uint16 get_ip_id(void)
{
/* XXX - locking */
	return ip_id++;
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
	if (ip_id == 0)
		ip_id = real_time_clock() & 0xffff;


	memset(proto, 0, sizeof(struct protosw *) * IPPROTO_MAX);

	add_protosw(proto, NET_LAYER2);
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

struct ipv4_module_info ipv4_module_info = {
	ipv4_output,
	get_ip_id,
	ipv4_ctloutput,
	ip_srcroute,
	ip_stripoptions
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
	get_ip_id,
	ipv4_ctloutput,
	ip_srcroute,
	ip_stripoptions
};

_EXPORT module_info *modules[] = {
	(module_info*) &my_module,
	NULL
};
			
#endif		 
