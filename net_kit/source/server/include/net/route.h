/* route.h */

#ifndef NET_ROUTE_H
#define NET_ROUTE_H

#include "net/radix.h"

/*
 * A route consists of a destination address and a reference
 * to a routing entry.  These are often held by protocols
 * in their control blocks, e.g. inpcb.
 */
struct route {
        struct  rtentry *ro_rt;
        struct  sockaddr ro_dst;
};

/*
 * These numbers are used by reliable protocols for determining
 * retransmission behavior and are included in the routing structure.
 */
struct rt_metrics {
        u_long  rmx_locks;      /* Kernel must leave these values alone */
        u_long  rmx_mtu;        /* MTU for this path */
        u_long  rmx_hopcount;   /* max hops expected */
        u_long  rmx_expire;     /* lifetime for route, e.g. redirect */
        u_long  rmx_recvpipe;   /* inbound delay-bandwith product */
        u_long  rmx_sendpipe;   /* outbound delay-bandwith product */
        u_long  rmx_ssthresh;   /* outbound gateway buffer limit */
        u_long  rmx_rtt;        /* estimated round trip time */
        u_long  rmx_rttvar;     /* estimated rtt variance */
        u_long  rmx_pksent;     /* packets sent using this route */
};

struct rtentry {
        struct  radix_node rt_nodes[2]; /* tree glue, and other values */
        struct  sockaddr *rt_gateway;   /* value */
        uint	rt_flags;               /* up/down?, host/net */
        int     rt_refcnt;              /* # held references */
        uint32	rt_use;                 /* raw # packets forwarded */
        struct  ifnet  *rt_ifp;         /* the answer: interface to use */
        struct  ifaddr *rt_ifa;         /* the answer: interface to use */
        struct  sockaddr *rt_genmask;   /* for generation of cloned routes */
        caddr_t rt_llinfo;              /* pointer to link level info cache */
        struct  rt_metrics rt_rmx;      /* metrics used by rx'ing protocols */
        struct  rtentry *rt_gwroute;    /* implied entry for gatewayed routes */
        struct  rtentry *rt_parent;     /* If cloned, parent of this route. */
	/* XXX - add this! */
        // rt_timer;  * queue of timeouts for misc funcs *
};
#define rt_use  rt_rmx.rmx_pksent
#define rt_key(r)       ((struct sockaddr *)((r)->rt_nodes->rn_key))
#define rt_mask(r)      ((struct sockaddr *)((r)->rt_nodes->rn_mask))

#define RTF_UP          0x1             /* route usable */
#define RTF_GATEWAY     0x2             /* destination is a gateway */
#define RTF_HOST        0x4             /* host entry (net otherwise) */
#define RTF_REJECT      0x8             /* host or net unreachable */
#define RTF_DYNAMIC     0x10            /* created dynamically (by redirect) */
#define RTF_MODIFIED    0x20            /* modified dynamically (by redirect) */
#define RTF_DONE        0x40            /* message confirmed */
#define RTF_MASK        0x80            /* subnet mask present */
#define RTF_CLONING     0x100           /* generate new routes on use */
#define RTF_XRESOLVE    0x200           /* external daemon resolves name */
#define RTF_LLINFO      0x400           /* generated by ARP or ESIS */
#define RTF_STATIC      0x800           /* manually added */
#define RTF_BLACKHOLE   0x1000          /* just discard pkts (during updates) */
#define RTF_PROTO3      0x2000          /* protocol specific routing flag */
#define RTF_PROTO2      0x4000          /* protocol specific routing flag */
#define RTF_PROTO1      0x8000          /* protocol specific routing flag */

#define RTM_VERSION     3       /* Up the ante and ignore older versions */

#define RTM_ADD         0x1     /* Add Route */
#define RTM_DELETE      0x2     /* Delete Route */
#define RTM_CHANGE      0x3     /* Change Metrics or flags */
#define RTM_GET         0x4     /* Report Metrics */
#define RTM_LOSING      0x5     /* Kernel Suspects Partitioning */
#define RTM_REDIRECT    0x6     /* Told to use different route */
#define RTM_MISS        0x7     /* Lookup failed on this address */
#define RTM_LOCK        0x8     /* fix specified metrics */
#define RTM_OLDADD      0x9     /* caused by SIOCADDRT */
#define RTM_OLDDEL      0xa     /* caused by SIOCDELRT */
#define RTM_RESOLVE     0xb     /* req to resolve dst to LL addr */
#define RTM_NEWADDR     0xc     /* address being added to iface */
#define RTM_DELADDR     0xd     /* address being removed from iface */
#define RTM_IFINFO      0xe     /* iface going up/down etc. */

#define RTV_MTU         0x1     /* init or lock _mtu */
#define RTV_HOPCOUNT    0x2     /* init or lock _hopcount */
#define RTV_EXPIRE      0x4     /* init or lock _hopcount */
#define RTV_RPIPE       0x8     /* init or lock _recvpipe */
#define RTV_SPIPE       0x10    /* init or lock _sendpipe */
#define RTV_SSTHRESH    0x20    /* init or lock _ssthresh */
#define RTV_RTT         0x40    /* init or lock _rtt */
#define RTV_RTTVAR      0x80    /* init or lock _rttvar */

/*
 * Bitmask values for rtm_addr.
 */
#define RTA_DST         0x1     /* destination sockaddr present */
#define RTA_GATEWAY     0x2     /* gateway sockaddr present */
#define RTA_NETMASK     0x4     /* netmask sockaddr present */
#define RTA_GENMASK     0x8     /* cloning mask sockaddr present */
#define RTA_IFP         0x10    /* interface name sockaddr present */
#define RTA_IFA         0x20    /* interface addr sockaddr present */
#define RTA_AUTHOR      0x40    /* sockaddr for author of redirect */
#define RTA_BRD         0x80    /* for NEWADDR, broadcast or p-p dest addr */

/*
 * Index offsets for sockaddr array for alternate internal encoding.
 */
#define RTAX_DST        0       /* destination sockaddr present */
#define RTAX_GATEWAY    1       /* gateway sockaddr present */
#define RTAX_NETMASK    2       /* netmask sockaddr present */
#define RTAX_GENMASK    3       /* cloning mask sockaddr present */
#define RTAX_IFP        4       /* interface name sockaddr present */
#define RTAX_IFA        5       /* interface addr sockaddr present */
#define RTAX_AUTHOR     6       /* sockaddr for author of redirect */
#define RTAX_BRD        7       /* for NEWADDR, broadcast or p-p dest addr */
#define RTAX_MAX        8       /* size of array to allocate */

struct rt_msghdr {
	uint16 rtm_msglen;
	uint8 rtm_version;
	uint8 rtm_type;
	
	uint16 rtm_index;
	int rtm_flags;
	int rtm_addrs;
	int rtm_seq;
	int rtm_errno;
	int rtm_use;
	uint32 rtm_inits;
	struct rt_metrics rtm_rmx;
};

struct rt_addrinfo {
        int     rti_addrs;
        struct  sockaddr *rti_info[RTAX_MAX];
        int     rti_flags;
        struct  ifaddr *rti_ifa;
        struct  ifnet *rti_ifp;
        struct  rt_msghdr *rti_rtm;
};

struct route_cb {
	int32 ip_count;     /* how many AF_INET structures we have */
	int32 any_count;    /* total of all above... */
};

struct walkarg {
	int w_op;
	int w_arg;
	int w_given;
	int w_needed;
	int w_tmemsize;
	caddr_t w_where;
	caddr_t w_tmem;
};

/*
 * Routing statistics.
 */
struct  rtstat {
        int32 rts_badredirect;      /* bogus redirect calls */
        int32 rts_dynamic;          /* routes created by redirects */
        int32 rts_newgateway;               /* routes modified by redirects */
        int32 rts_unreach;          /* lookups which failed */
        int32 rts_wildcard;         /* lookups satisfied by a wildcard */
};

#define RTFREE(rt) do { \
        if ((rt)->rt_refcnt <= 1) \
                rtfree(rt); \
        else \
                (rt)->rt_refcnt--; \
} while (0)

struct  rtstat  rtstat;
struct  radix_node_head *rt_tables[AF_MAX+1];

void	route_init(void);

int	rtinit (struct ifaddr *, int, int);
void	rtalloc (struct route *);
struct rtentry *rtalloc1 (struct sockaddr *, int);
void	rtfree (struct rtentry *);
int	rtrequest (int, struct sockaddr *,
                        struct sockaddr *, struct sockaddr *, int,
                        struct rtentry **);
void	rt_maskedcopy(struct sockaddr *src,
                struct sockaddr *dst,
                struct sockaddr *netmask);
int	rt_setgate (struct rtentry *, struct sockaddr *,
                         struct sockaddr *);

struct radix_node_head ** get_rt_tables(void);

#endif /* NET_ROUTE_H */
