/* ethernet.c
 * ethernet encapsulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <kernel/OS.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <malloc.h>
#include <time.h>

#include "protocols.h"
#include "netinet/in_var.h"
#include "sys/protosw.h"
#include "net/if.h"
#include "net/if_arp.h"
#include "net/if_dl.h"
#include "netinet/if_ether.h"
#include "arp/arp_module.h"
#include "sys/socket.h"
#include "sys/sockio.h"
#include "net/route.h"

#include "core_module.h"
#include "net_module.h"
#include "core_funcs.h"

#ifdef _KERNEL_MODE
#include <KernelExport.h>
#include "net_device.h"

#define ETHERNET_MODULE_PATH	"network/interface/ethernet"

static timer arp_timer;

/* forward prototypes */
int ether_dev_start(ifnet *dev);
int ether_dev_stop (ifnet *dev);
#else	/* _KERNEL_MODE */
#include "net_timer.h"
#endif

static struct core_module_info *core = NULL;

struct protosw *proto[IPPROTO_MAX];
static struct ether_device *ether_devices = NULL; 	/* list of ethernet devices */

void ether_input(struct mbuf *buf);
int  ether_output(struct ifnet *ifp, struct mbuf *buf, struct sockaddr *dst,
		 struct rtentry *rt0);
int  ether_ioctl(struct ifnet *ifp, int cmd, caddr_t data);
int  ether_dev_attach(ifnet *dev);
int  ether_dev_stop(ifnet *dev);
void arp_rtrequest(int req, struct rtentry *rt, struct sockaddr *sa);
static void arpinput(struct mbuf *m);

#define DRIVER_DIRECTORY "/dev/net"

#define SIN(s)   ((struct sockaddr_in*)s)
#define SDL(s)   ((struct sockaddr_dl*)s)
#define rt_expire rt_rmx.rmx_expire

uint8 ether_bcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

/* these are in seconds... */
static int arpt_prune = (5 * 60); 	/* time interval we prune the arp cache? 5 minutes */
static int arpt_keep  = (20 * 60);	/* length of time we keep entries... (20 mins) */
static int arpt_down = 20;      /* seconds between arp flooding */																																																											


static int arp_maxtries = 5;    /* max tries before a pause */

/* stats */
static int32 arp_inuse = 0;	/* how many entries do we have? */
static int32 arp_allocated = 0;	/* how many arp entries have we created? */

struct llinfo_arp llinfo_arp;
struct in_ifaddr *primary_addr;

#define INARPQ(la, head) do { \
	(la)->la_prev = (head)->la_next->la_prev; \
	(la)->la_next = (head)->la_next; \
	(head)->la_next->la_prev = (la); \
	(head)->la_next = (la); \
} while (0)

#define REMARPQ(la) do { \
	(la)->la_prev->la_next = (la)->la_next; \
	(la)->la_next->la_prev = (la)->la_prev; \
} while (0)

static char digits[] = "0123456789abcdef";

static char *ether_sprintf(uint8 *ap)
{
	register int i;
	static char etherbuf[18];
	register char *cp = etherbuf;

	for (i = 0; i < 6; i++) {
		*cp++ = digits[*ap >> 4];
		*cp++ = digits[*ap++ & 0xf];
		*cp++ = ':';
	}
	*--cp = 0;
	return (etherbuf);
}

#if ARP_DEBUG
static void dump_arp(void *buffer)
{
	struct ether_arp *arp = (struct ether_arp *)buffer;

	printf("arp request :\n");
	printf("            : hardware type : %s\n",
           ntohs(arp->arp_hrd) == ARPHRD_ETHER ? "ethernet" : "unknown");
	printf("            : protocol type : %s\n",
           ntohs(arp->arp_pro) == ETHERTYPE_IP ? "IPv4" : "unknown");
	printf("            : hardware size : %d\n", arp->arp_hln);
	printf("            : protocol size : %d\n", arp->arp_pln);
	printf("            : op code       : ");
	switch(ntohs(arp->arp_op)) {
		case ARPOP_REPLY:
			printf("ARP Reply\n");
			break;
		case ARPOP_REQUEST:
			printf("ARP Request\n");
			break;
		default:
			printf("Who knows? %04x\n", ntohs(arp->arp_op));
	}
	printf("            : sender        : %s", ether_sprintf(&arp->arp_sha));
	printf(" [%08lx]\n", ntohl(*(uint32*)&arp->arp_spa));
	printf("            : target        : %s", ether_sprintf(&arp->arp_tha));
	printf(" [%08lx]\n", ntohl(*(uint32*)&arp->arp_tpa));
}
#endif /* ARP_DEBUG */

/* We now actually attach the device to the system... */
static void attach_device(int devid, char *driver, char *devno)
{
	struct ether_device *ed;
	struct ifnet *ifp;
	struct ifaddr *ifa;
	struct sockaddr_dl *sdl;
	status_t status;
		
	ed = malloc(sizeof(struct ether_device));
	if (!ed)
		return;	
	memset(ed, 0, sizeof(*ed));
	ifp = &ed->sc_if;
	
	/* get the MAC address... */
	status = ioctl(devid, IF_GETADDR, &ed->sc_addr, 6);
	if (status < B_OK) {
		printf("Failed to get a MAC address, ignoring %s/%s\n", driver, devno);
		close(devid);
		free(ed);
		return;
	}
	
	ifp->devid = devid;
	ifp->if_type = IFT_ETHER;
	ifp->name = strdup(driver);
	ifp->if_unit = atoi(devno);
	ifp->if_mtu = ETHERMTU;
	ifp->if_hdrlen = 14;
	ifp->if_addrlen = 6;
	ifp->if_flags |= IFF_BROADCAST;
	
	ifp->rx_thread = -1;
	ifp->tx_thread = -1;

	ifp->input = &ether_input;
	ifp->output = &ether_output;
	ifp->stop = &ether_dev_stop;
	ifp->ioctl = &ether_ioctl;
	
	if_attach(ifp);

	/* Add the MAC address to our list of addresses... */
	for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
		if ((sdl = (struct sockaddr_dl*)ifa->ifa_addr) &&
		    sdl->sdl_family == AF_LINK) {
			sdl->sdl_type = IFT_ETHER;
			sdl->sdl_alen = ifp->if_addrlen;
			memcpy(LLADDR(sdl), &ed->sc_addr, ifp->if_addrlen);
			break;
		}
	}	

	ed->next = NULL; /* we get added at the end of the list */
	/* we maintain our own list of devices as well as the global list */
	if (!ether_devices) {
		ether_devices = ed;
	} else {
		struct ether_device *dptr = ether_devices;
		while (dptr->next)
			dptr = dptr->next;

		dptr->next = ed;
	}
}
	
static void open_device(char *driver, char *devno)
{
	char path[PATH_MAX];
	int dev;
	status_t status = -1;
	
	sprintf(path, "%s/%s/%s", DRIVER_DIRECTORY, driver, devno);
	dev = open(path, O_RDWR);
	if (dev < B_OK) {
		printf("Unable to open %s, %ld [%s]\n", path,
			status, strerror(status));
		return;
	}

	status = ioctl(dev, IF_INIT, NULL, 0);
	if (status < B_OK) {
		printf("Unable to init card %s, %ld [%s]\n", path,
			status, strerror(status));
		close(dev);
		return;
	} else 
		attach_device(dev, driver, devno);
};

static void find_devices(void)
{
	DIR *dir, *driv_dir;
	struct dirent *de, *dre;
	char path[PATH_MAX];

	dir = opendir(DRIVER_DIRECTORY);
	if (!dir) {
		printf("Couldn't open the directory %s\n", DRIVER_DIRECTORY);
		return;
	}

	while ((de = readdir(dir)) != NULL) {
		/* hmm, is it a driver? */
		if (strcmp(de->d_name, ".") == 0 ||
	    	strcmp(de->d_name, "..") == 0 ||
	    	strcmp(de->d_name, "socket") == 0 ||
	    	strcmp(de->d_name, "stack") == 0)
			continue;

		/* OK we assume it's a driver...but skip the ether driver
		 * as I don't really know what it is!
		 */
		if (strcmp(de->d_name, "ether") == 0)
			continue;
                        
		sprintf(path, "%s/%s", DRIVER_DIRECTORY, de->d_name);
		driv_dir = opendir(path);
		if (!driv_dir) {
			printf("I couldn't find any drivers in the %s driver directory\n",
			       de->d_name);
		} else {
			while ((dre = readdir(driv_dir)) != NULL) {
				/* skip . and .. */
				if (strcmp(dre->d_name, ".") == 0 ||
					strcmp(dre->d_name, "..") == 0)
					continue;
                       		    
				open_device(de->d_name, dre->d_name);
			}
			closedir(driv_dir);
		}
	}
	closedir(dir);

	return;
}

#if SHOW_DEBUG
static void dump_ether_details(struct mbuf *buf)
{
	struct ether_header *eth = mtod(buf, struct ether_header *);

	printf("Ethernet packet from ");
	print_ether_addr(&eth->src);
	printf(" to ");
	print_ether_addr(&eth->dest);

	if (buf->m_flags & M_BCAST)
		printf(" BCAST");

	printf(" proto ");
	switch (eth->type) {
		case ETHER_ARP:
			printf("ARP\n");
			break;
		case ETHER_RARP:
			printf("RARP\n");
			break;
		case ETHER_IPV4:
			printf("IPv4\n");
			break;
		case ETHER_IPV6:
			printf("IPv6\n");
			break;
		default:
			printf("unknown (%04x)\n", eth->type);
	}
}
#endif

void ether_input(struct mbuf *m)
{
	struct ether_header *eth = mtod(m, struct ether_header *);
	int len = sizeof(struct ether_header);
	
	eth->ether_type = ntohs(eth->ether_type);

	if (memcmp((void*)&eth->ether_dhost, (void*)&ether_bcast, 6) == 0)
		m->m_flags |= M_BCAST;
	if (eth->ether_dhost[0] & 1)
		m->m_flags |= M_MCAST;
		
#if SHOW_DEBUG	
	dump_ether_details(buf);
#endif
	m_adj(m, len);
	
	switch(eth->ether_type) {
		case ETHERTYPE_ARP:
			arpinput(m);
			break;
		case ETHERTYPE_IP:
			if (proto[IPPROTO_IP] && proto[IPPROTO_IP]->pr_input)
				return proto[IPPROTO_IP]->pr_input(m, 0);
			else
				printf("proto[%d] = %p, not called...\n", IPPROTO_IP,
					proto[IPPROTO_IP]);
			break;
		default:
			printf("Couldn't process unknown protocol %04x\n", eth->ether_type);
	}

	m_freem(m);
	return;	
}

#define senderr(e)	{ error = (e); goto bad; }

int ether_output(struct ifnet *ifp, struct mbuf *buf, struct sockaddr *dst,
		 struct rtentry *rt0)
{
	struct ether_header *eh;
	struct rtentry *rt;
	struct arpcom *ac = (struct arpcom*)ifp;
	uint8 edst[6];
	int off;
	uint16 type;
	struct mbuf *mcopy = NULL;
	int error = 0;
	
	if ((ifp->if_flags & (IFF_UP | IFF_RUNNING)) != (IFF_UP | IFF_RUNNING))
		senderr(ENETDOWN);

	if ((rt = rt0)) {
		if ((rt->rt_flags & RTF_UP) == 0) {
			if ((rt0 = rt = rtalloc1(dst, 1)) != NULL)
				rt->rt_refcnt--;
			else
				senderr(EHOSTUNREACH);
		}
		if (rt->rt_flags & RTF_GATEWAY) {
			if (!rt->rt_gwroute)
				goto lookup;
			if (((rt = rt->rt_gwroute)->rt_flags & RTF_UP) == 0) {
				rtfree(rt0);
				rt = rt0;
lookup:	
				rt->rt_gwroute = rtalloc1(rt->rt_gateway, 1);
			
				if ((rt = rt->rt_gwroute) == NULL)
					senderr(EHOSTUNREACH);
			}
		}
		if (rt->rt_flags & RTF_REJECT)
			if (rt->rt_expire == 0 ||
			    rt->rt_expire > real_time_clock()) {
			/* XXX - add test for expired here... */
			printf("flags & RTF_REJECT\n");
			printf("Error: %s\n", rt == rt0 ? "EHOSTDOWN" : "EHOSTUNREACH");
			senderr(rt == rt0 ? EHOSTDOWN : EHOSTUNREACH);
		}
	}

	switch (dst->sa_family) {
		case AF_INET:
			if (!arpresolve(ac, rt, buf, dst, edst)) {
				return 0;
			}
			if ((buf->m_flags & M_BCAST) && (ifp->if_flags & IFF_SIMPLEX))
				mcopy = m_copym(buf, 0, (int)M_COPYALL);
			off = buf->m_pkthdr.len - buf->m_len;
			type = ETHERTYPE_IP;
			break;
		case AF_UNSPEC:
			eh = (struct ether_header*)dst->sa_data;
			memcpy((caddr_t)edst, (caddr_t)eh->ether_dhost, sizeof(edst));
			type = eh->ether_type;
			break;
	}
	
/* Hmmm, can't find a good way of getting this to work without expanding the
 * macro for the kernel case, so it's here expanded. I've found that using the
 * directly causes segafults. Bear in mind we want ALL allocation/free actions
 * to take place in the core to keep as small a memory footprint as possible.
 */
#ifndef _KERNEL_MODE
	M_PREPEND(buf, sizeof(struct ether_header));
#else
#define M_LEADINGSPACE(m) \
        ((m)->m_flags & M_EXT ? (m)->m_data - (m)->m_ext.ext_buf : \
         (m)->m_flags & M_PKTHDR ? (m)->m_data - (m)->m_pktdat : \
         (m)->m_data - (m)->m_dat)

        if (M_LEADINGSPACE(buf) >= sizeof(struct ether_header)) {
                buf->m_data -= sizeof(struct ether_header);
                buf->m_len += sizeof(struct ether_header);
        } else 
                buf = m_prepend(buf, sizeof(struct ether_header));
        if (buf && buf->m_flags & M_PKTHDR)
                buf->m_pkthdr.len += sizeof(struct ether_header);
#endif

	if (buf == NULL)
		senderr(ENOMEM);
	eh = mtod(buf, struct ether_header*);
	type = htons(type);
	memcpy(&eh->ether_type, &type, sizeof(eh->ether_type));
	memcpy(&eh->ether_dhost, edst, sizeof(edst));
	memcpy(&eh->ether_shost, ac->ac_enaddr, sizeof(eh->ether_shost));

	IFQ_ENQUEUE(ifp->txq, buf);

	return error;
bad:
	if (buf)
		m_free(buf);
	printf("ether_output: returning %d\n", error);
	return error;
}

static struct llinfo_arp *arplookup(uint32 addr, int create, int proxy)
{
	struct rtentry *rt;
	static struct sockaddr_inarp sin;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_len = sizeof(sin);
	sin.sin_addr.s_addr = addr;
	sin.sin_other = proxy ? SIN_PROXY : 0;
	rt = rtalloc1((struct sockaddr*) &sin, create);
	if (!rt)
		return NULL;
	rt->rt_refcnt--;
	if ((rt->rt_flags & RTF_GATEWAY) || (rt->rt_flags & RTF_LLINFO) == 0 ||
	    rt->rt_gateway->sa_family != AF_LINK) {
		if (create)
			printf("arptnew failed on %08lx\n", ntohl(addr));
		return NULL;
	}
	return ((struct llinfo_arp *)rt->rt_llinfo);
}
	
int arpresolve(struct arpcom *ac, struct rtentry *rt, struct mbuf *m,
               struct sockaddr *dst, uint8 *desten)
{
	struct llinfo_arp *la;	
	struct sockaddr_dl *sdl;

	if (m->m_flags & M_BCAST) {
		memcpy(desten, &ether_bcast, sizeof(ether_bcast));
		return 1;
	}
	if (m->m_flags & M_MCAST) {
		ETHER_MAP_IP_MULTICAST(&SIN(dst)->sin_addr, desten);
		return 1;
	}

	if (rt) {
		la = (struct llinfo_arp*) rt->rt_llinfo;
	} else {
		if ((la = arplookup(SIN(dst)->sin_addr.s_addr, 1, 0)))
			rt = la->la_rt;
	}
	if (la == NULL || rt == NULL) {
		printf("arpresolve: can't allocate llinfo!\n");
		m_freem(m);
		return 0;
	}
	sdl = SDL(rt->rt_gateway);

	if ((rt->rt_expire == 0 || rt->rt_expire > real_time_clock()) &&
	    sdl->sdl_family == AF_LINK && sdl->sdl_alen != 0) {
		memcpy(desten, LLADDR(sdl), sdl->sdl_alen);
		return 1;
	}
	
	if (la->la_hold) {
		m_freem(la->la_hold);
	}

	la->la_hold = m;
	
	if (rt->rt_expire) {
		rt->rt_flags &= ~RTF_REJECT;
		if (la->la_asked == 0 || rt->rt_expire != real_time_clock()) {
			rt->rt_expire = real_time_clock();
			if (la->la_asked++ < arp_maxtries) {
				arpwhohas(ac, &(SIN(dst)->sin_addr));
			} else {
				rt->rt_flags |= RTF_REJECT;
				rt->rt_expire += arpt_down;
				la->la_asked = 0;
			}
		}
	}
	return 0;
}

static void arptfree(struct llinfo_arp *la)
{
	struct rtentry *rt = la->la_rt;
	struct sockaddr_dl *sdl;
	
	if (rt == NULL)
		return;
	if (rt->rt_refcnt > 0 && (sdl = SDL(rt->rt_gateway)) &&
	    sdl->sdl_family == AF_LINK) {
		sdl->sdl_alen = 0;
		la->la_asked = 0;
		rt->rt_flags &= ~RTF_REJECT;
		return;
	}
	rtrequest(RTM_DELETE, rt_key(rt), NULL, rt_mask(rt), 0, NULL);
}

#ifndef _KERNEL_MODE
typedef void timer;
#endif
static int32 arptimer(timer *t)
{
	struct llinfo_arp *la = llinfo_arp.la_next;
	
	while (la != &llinfo_arp) {
		struct rtentry *rt = la->la_rt;
		la = la->la_next;
		if (rt->rt_expire && rt->rt_expire <= real_time_clock())
			arptfree(la->la_prev);
	}
	return 0;
}

static void arprequest(struct arpcom *ac, uint32 *sip, uint32 *tip, uint8 *enaddr)
{
	struct mbuf *m;
	struct ether_header *eh;
	struct ether_arp *ea;
	struct sockaddr sa;

	if ((m = m_gethdr(MT_DATA)) == NULL)
		return;
	m->m_len = sizeof(*ea);
	m->m_pkthdr.len = sizeof(*ea);
	MH_ALIGN(m, sizeof(*ea));
	
	ea = mtod(m, struct ether_arp*);
	eh = (struct ether_header*) sa.sa_data;
	memset(ea, 0, sizeof(*ea));
	
	memcpy(eh->ether_dhost, &ether_bcast, sizeof(eh->ether_dhost));
	eh->ether_type = ETHERTYPE_ARP;
	
	ea->arp_hrd = htons(ARPHRD_ETHER);
	ea->arp_pro = htons(ETHERTYPE_IP);
	ea->arp_hln = sizeof(ea->arp_sha);
	ea->arp_pln = sizeof(ea->arp_spa);
	ea->arp_op = htons(ARPOP_REQUEST);
	
	memcpy((caddr_t)ea->arp_sha, (caddr_t)enaddr, sizeof(ea->arp_sha));
	memcpy(ea->arp_spa, sip, sizeof(ea->arp_spa));
	memcpy(ea->arp_tpa, tip, sizeof(ea->arp_tpa));
	
	sa.sa_family = AF_UNSPEC;
	sa.sa_len = sizeof(sa);

	(*ac->ac_if.output)(&ac->ac_if, m, &sa, NULL);
}

void arpwhohas(struct arpcom *ac, struct in_addr *ia)
{
	arprequest(ac, &ac->ac_ipaddr.s_addr, &ia->s_addr, ac->ac_enaddr);
}


void arp_rtrequest(int req, struct rtentry *rt, struct sockaddr *sa)
{
	struct sockaddr *gate = rt->rt_gateway;
	static struct sockaddr_dl null_sdl = {sizeof(null_sdl), AF_LINK};
	struct llinfo_arp *la = (struct llinfo_arp*)rt->rt_llinfo;

	if (rt->rt_flags & RTF_GATEWAY)
		return;
       
	switch (req) {
		case RTM_ADD:
			if ((rt->rt_flags & RTF_HOST) == 0 &&
			    SIN(rt_mask(rt))->sin_addr.s_addr != 0xffffffff)
				rt->rt_flags |= RTF_CLONING;
			if (rt->rt_flags & RTF_CLONING) {
				rt_setgate(rt, rt_key(rt), (struct sockaddr*)&null_sdl);
				gate = rt->rt_gateway;
				SDL(gate)->sdl_type = rt->rt_ifp->if_type;
				SDL(gate)->sdl_index = rt->rt_ifp->if_index;
				rt->rt_expire = real_time_clock();
				break;
			}
			if (rt->rt_flags & RTF_ANNOUNCE)
				arprequest((struct arpcom*) rt->rt_ifp, 
				           &SIN(rt_key(rt))->sin_addr.s_addr,
				           &SIN(rt_key(rt))->sin_addr.s_addr,
				           (uint8*)LLADDR(SDL(gate)));
		case RTM_RESOLVE:
			if (gate->sa_family != AF_LINK ||
			    gate->sa_len < sizeof(null_sdl)) {
				printf("arp_rtrequest: bad gateway value!\n");
				break;
			}
			SDL(gate)->sdl_type = rt->rt_ifp->if_type;
			SDL(gate)->sdl_index = rt->rt_ifp->if_index;
			if (la)
				break;

			R_Malloc(la, struct llinfo_arp *, sizeof(*la));
			if (!la) {
				printf("arp_rtrequest: malloc failed!\n");
				break;
			}
			rt->rt_llinfo = (caddr_t)la;
			arp_inuse++, arp_allocated++;
			Bzero(la, sizeof(*la));
			
			la->la_rt = rt;
			rt->rt_flags |= RTF_LLINFO;
			INARPQ(la, &llinfo_arp);
			
			if (SIN(rt_key(rt))->sin_addr.s_addr == 
			    (IA_SIN(rt->rt_ifa))->sin_addr.s_addr) {
				rt->rt_expire = 0;
				Bcopy(((struct arpcom*)rt->rt_ifp)->ac_enaddr,
				      LLADDR(SDL(gate)), SDL(gate)->sdl_alen = 6);
/*
XXX - add function to get the ifnet * for the loopback from the "core"
      and ability to find the loopback device when added.
				if (useloopback)
					rt->rt_ifp = &loif;
*/
			}
			break;
		case RTM_DELETE:
			if (!la)
				break;
			arp_inuse--;
			REMARPQ(la);
			rt->rt_llinfo = NULL;
			rt->rt_flags &= ~ RTF_LLINFO;
			if (la->la_hold)
				m_freem(la->la_hold);
			Free((caddr_t)la);
	}
} 

static void arpinput(struct mbuf *m)
{
	struct ether_arp *ea;
	struct arpcom *ac = (struct arpcom *)m->m_pkthdr.rcvif;
	struct ether_header *eh;
	struct llinfo_arp *la = NULL;
	struct in_ifaddr *ia = NULL, *maybe_ia = NULL;
	struct in_addr isaddr, itaddr, myaddr;
	struct rtentry *rt;
	int op;
	struct sockaddr_dl *sdl;
	struct sockaddr sa;

#if ARP_DEBUG
	dump_arp(mtod(m, void*));
#endif

	if (!primary_addr)
		primary_addr = get_primary_addr();

	ea = mtod(m, struct ether_arp *);
	op = ntohs(ea->arp_op);
	memcpy(&isaddr, ea->arp_spa, sizeof(isaddr));
	memcpy(&itaddr, ea->arp_tpa, sizeof(isaddr));

	/* find out if it's for us... */
	for (ia = primary_addr;ia ; ia = ia->ia_next)
		if (ia->ia_ifp == &ac->ac_if) {
			maybe_ia = ia;
			if ((itaddr.s_addr == ia->ia_addr.sin_addr.s_addr) ||
			    (isaddr.s_addr == ia->ia_addr.sin_addr.s_addr))
				break;
		}
	if (!maybe_ia)
		goto out;

	myaddr = ia ? ia->ia_addr.sin_addr : maybe_ia->ia_addr.sin_addr;

	if (!memcmp(ac->ac_enaddr, ea->arp_sha, sizeof(ea->arp_sha)))
		goto out;
	if (!memcmp(ea->arp_sha, &ether_bcast, sizeof(ea->arp_sha))) {
		printf("arp_input: ether address was broadcast for %08lx\n",
		       htonl(isaddr.s_addr));
		goto out;
	}
	if (isaddr.s_addr == myaddr.s_addr) {
		printf("arp_input: duplicate IP address sent from %s\n",
		       ether_sprintf(ea->arp_sha));
		itaddr = myaddr;
		goto reply;
	}
	la = arplookup(isaddr.s_addr, itaddr.s_addr == myaddr.s_addr, 0);
	if (la && (rt = la->la_rt) && (sdl = SDL(rt->rt_gateway))) {
		if (sdl->sdl_alen && memcmp(ea->arp_sha, LLADDR(sdl), sdl->sdl_alen))
			printf("arp info overwritten for %08lx by %s\n",
			       isaddr.s_addr, ether_sprintf(ea->arp_sha));
		memcpy(LLADDR(sdl), ea->arp_sha, sdl->sdl_alen = sizeof(ea->arp_sha));
		if (rt->rt_expire) {
			rt->rt_expire = real_time_clock() + arpt_keep;
		}
		rt->rt_flags &= ~RTF_REJECT;
		la->la_asked = 0;
		if (la->la_hold) {
			(*ac->ac_if.output)(&ac->ac_if, la->la_hold, rt_key(rt), rt);
			la->la_hold = NULL;
		}
	}

reply:
	if (op != ARPOP_REQUEST) {
out:
		return;
	}

	if (itaddr.s_addr == myaddr.s_addr) {
		memcpy(ea->arp_tha, ea->arp_sha, sizeof(ea->arp_sha));
		memcpy(ea->arp_sha, ac->ac_enaddr, sizeof(ea->arp_sha));
	} else {
		la = arplookup(itaddr.s_addr, 0, SIN_PROXY);
		if (la == NULL)
			goto out;
		rt = la->la_rt;
		memcpy(ea->arp_tha, ea->arp_sha, sizeof(ea->arp_sha));
		sdl = SDL(rt->rt_gateway);
		memcpy(ea->arp_sha, LLADDR(sdl), sizeof(ea->arp_sha));
	}

	memcpy(ea->arp_tpa, ea->arp_spa, sizeof(ea->arp_spa));
	memcpy(ea->arp_spa, &itaddr, sizeof(ea->arp_spa));
	ea->arp_op = htons(ARPOP_REPLY);
	ea->arp_pro = htons(ETHERTYPE_IP);
	eh = (struct ether_header *)sa.sa_data;
	memcpy(eh->ether_dhost, ea->arp_tha, sizeof(eh->ether_dhost));
	eh->ether_type = ETHERTYPE_ARP;
	sa.sa_family = AF_UNSPEC;
	sa.sa_len = sizeof(sa);
	
	(*ac->ac_if.output)(&ac->ac_if, m, &sa, NULL);
	return;
}

void arp_init(void)
{
	llinfo_arp.la_next = llinfo_arp.la_prev = &llinfo_arp;
	
#ifdef _KERNEL_MODE
	add_timer(&arp_timer, arptimer, 
	          arpt_prune * 1000000,
			  B_PERIODIC_TIMER);
#else
	net_add_timer(&arptimer, NULL, arpt_prune * 1000000);
#endif
}

int ether_ioctl(struct ifnet *ifp, int cmd, caddr_t data)
{
	struct arpcom *ac = (struct arpcom *)ifp;
	struct ifaddr *ifa = (struct ifaddr*)data;

	if ((ifp->if_flags & IFF_UP) == 0 &&
	    (ifp->rx_thread > 0 || ifp->tx_thread > 0)) {
		/* shutdown our threads and remove the IFF_RUNNING flag... */
		if (ifp->rx_thread > 0)
			kill_thread(ifp->rx_thread);
		if (ifp->tx_thread > 0)
			kill_thread(ifp->tx_thread);
		ifp->rx_thread = ifp->tx_thread = -1;
		ifp->if_flags &= ~IFF_RUNNING;
	}
	ifa->ifa_rtrequest = &arp_rtrequest;
	
	switch (cmd) {
		case SIOCSIFADDR:
			ifp->if_flags |= IFF_UP;
			switch (ifa->ifa_addr->sa_family) {
				case AF_INET:
					ac->ac_ipaddr = ((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
#ifdef ETHER_DEBUG
					printf("setting ether_device ip address to %08lx\n", 
						ntohl(((struct sockaddr_in*)ifa->ifa_addr)->sin_addr.s_addr));
#endif
					break;
				default:
					printf("don't know how to work with address family %d\n", 
						ifa->ifa_addr->sa_family);
			}
			break;
		default:
			printf("unhandled call to ethernet_ioctl\n");
	}

	if ((ifp->if_flags & IFF_UP) &&
	    (ifp->rx_thread == -1 || ifp->tx_thread == -1)) {
		/* start our threads and add the IFF_RUNNING flag... */
		if (ifp->rx_thread < 0)
			start_rx_thread(ifp);
		if (ifp->tx_thread < 0)
			start_tx_thread(ifp);
		ifp->if_flags |= IFF_RUNNING;
	}

	return 0;
}

int ether_dev_stop(ifnet *dev)
{
	dev->if_flags &= ~IFF_UP;

	/* should find better ways of doing this... */
	kill_thread(dev->rx_thread);
	kill_thread(dev->tx_thread);
	
	return 0;
}

#ifndef _KERNEL_MODE

static int ether_init(struct core_module_info *cp)
{
	core = cp;
	find_devices();

	memset(proto, 0, sizeof(struct protosw *) * IPPROTO_MAX);
	add_protosw(proto, NET_LAYER2);
	arp_init();
	
	return 0;
}

struct device_info device_info = {
	"ethernet mdoule",
	&ether_init
};

#else

static int k_init(void)
{
	if (!core)
		get_module(CORE_MODULE_PATH,
			(module_info**)&core);

	find_devices();

	memset(proto, 0, sizeof(struct protosw *) * IPPROTO_MAX);
	add_protosw(proto, NET_LAYER2);
	arp_init();

	return 0;
}

static int k_uninit(void)
{
	/* we should really stop all cards etc here! */
	return 0;
}


static int32 ether_ops(int32 op, ...)
{
	switch(op) {
		case B_MODULE_INIT:
			if (get_module(CORE_MODULE_PATH,
				(module_info**)&core) != B_OK) {
				printf("Failed to get core pointer, declining!\n");
				return B_ERROR;
			}
			break;
		case B_MODULE_UNINIT:
			return k_uninit();
		default:
			return B_ERROR;
	}
	return B_OK;
}
			
static struct device_module_info my_module = {
	{
		ETHERNET_MODULE_PATH,
		B_KEEP_LOADED,
		ether_ops
	},

	k_init
};

_EXPORT module_info * modules[] = {
	(module_info *)&my_module,
	NULL
};

#endif
