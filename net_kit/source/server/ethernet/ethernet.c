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

#include "net_module.h"
#include "protocols.h"
#include "netinet/in_var.h"
#include "sys/protosw.h"
#include "ethernet/ethernet.h"
#include "arp/arp_module.h"
#include "net/if_dl.h"
#include "sys/socket.h"
#include "sys/sockio.h"

#ifdef _KERNEL_MODE
#include <KernelExport.h>
#include "net_device.h"
#include "net_server/core_module.h"

#define start_rx_thread	core->start_rx_thread
#define start_tx_thread	core->start_tx_thread

#define ETHERNET_MODULE_PATH	"network/interface/ethernet"

static struct core_module_info *core = NULL;

/* forward prototypes */
int ether_dev_start(ifnet *dev);
int ether_dev_stop (ifnet *dev);
#endif

static struct arp_module_info *arp = NULL;
struct protosw *proto[IPPROTO_MAX];
static struct ether_device *ether_devices = NULL; 	/* list of ethernet devices */

#ifndef _KERNEL_MODE
image_id arpid;
#endif

int ether_input(struct mbuf *buf);
int ether_output(struct ifnet *ifp, struct mbuf *buf, struct sockaddr *dst,
		 struct rtentry *rt0);
int ether_ioctl(struct ifnet *ifp, int cmd, caddr_t data);
int ether_dev_attach(ifnet *dev);
int ether_dev_stop(ifnet *dev);

#define DRIVER_DIRECTORY "/dev/net"

uint8 ether_bcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static void open_device(char *driver, char *devno)
{
	struct ether_device *ed;
	struct ifnet *ifn;
	char path[PATH_MAX];
	int dev;
	status_t status = -1;
	struct sockaddr_dl *sdl;
	struct ifaddr *ifa;
	
	sprintf(path, "%s/%s/%s", DRIVER_DIRECTORY, driver, devno);
	dev = open(path, O_RDWR);
	if (dev < B_OK) {
		printf("Unable to open %s, %ld [%s]\n", path,
			status, strerror(status));
		goto badcard;
	}

	status = ioctl(dev, IF_INIT, NULL, 0);
	if (status < B_OK) {
		printf("Unable to init card %s, %ld [%s]\n", path,
			status, strerror(status));
		close(dev);
		goto badcard;
	}
	
	/* Hmm, this should probably actually be done by the device drivers
	 * but we're not changing the device drivers so we'll do it here.
	 * The type we set is just the generic IFT_ETHER but it could be
	 * more accurate if the driver set it. Oh well.
	 */
	 
	ed = malloc(sizeof(struct ether_device));
	if (!ed)
		return;	

	memset(ed, 0, sizeof(*ed));
	ifn = &ed->ifn;

	/* get the MAC address... */
	status = ioctl(dev, IF_GETADDR, &ed->e_addr, 6);
	if (status < B_OK) {
		printf("Failed to get a MAC address, ignoring %s\n", ifn->if_name);
		close(dev);
		free(ed);
		return;
	}
	
	ed->devid = dev;
	ed->ed_devid = dev;	/* we use this to match... */
	ed->ed_name = strdup(driver);
	ed->ed_unit = atoi(devno);
	ed->ed_type = IFT_ETHER;
	ed->ed_rx_thread = -1;
	ed->ed_tx_thread = -1;
	ed->ed_hdrlen = 14;
	ed->ed_addrlen = 6;
	ifn->if_mtu = ETHERMTU;

	ifn->input = &ether_input;
	ifn->output = &ether_output;
	ifn->stop = &ether_dev_stop;
	ifn->ioctl = &ether_ioctl;

	/* Add the MAC address to our list of addresses... */
	for (ifa = ifn->if_addrlist; ifa; ifa = ifa->ifa_next) {
		if ((sdl = (struct sockaddr_dl*)ifa->ifa_addr) &&
		    sdl->sdl_family == AF_LINK) {
			sdl->sdl_type = IFT_ETHER;
			sdl->sdl_alen = ifn->if_addrlen;
			memcpy(LLADDR(sdl), &ed->e_addr, ifn->if_addrlen);
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

#if SHOW_DEBUG
	printf("added ethernet device %s%d\n", ed->ed_name, ed->ed_unit);
#endif

#ifndef _KERNEL_MODE
	if_attach(&ed->ifn);
#else
	core->if_attach(&ed->ifn);
#endif
badcard:
	return;
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
	    	strcmp(de->d_name, "socket") == 0)
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
	ethernet_header *eth = mtod(buf, ethernet_header *);

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

/* what should the return value be? */
int ether_input(struct mbuf *buf)
{
	ethernet_header *eth = mtod(buf, ethernet_header *);
	int len = sizeof(ethernet_header);

	eth->type = ntohs(eth->type);

	if (memcmp((void*)&eth->dest, (void*)&ether_bcast, 6) == 0)
		buf->m_flags |= M_BCAST;
		
	if (eth->type < 1500) {
		eth802_header *e8 = mtod(buf, eth802_header*);
		e8->type = ntohs(e8->type);
		printf("It's an 802.x encapsulated packet - type %04x\n", e8->type);
		len = sizeof(eth802_header);
		eth->type = e8->type;
	}

#if SHOW_DEBUG	
	dump_ether_details(buf);
#endif
#ifndef _KERNEL_MODE
	m_adj(buf, len);
#else
	core->m_adj(buf, len);
#endif
	
	switch(eth->type) {
		case ETHER_ARP:
			return arp->input(buf, 0);
		case ETHER_IPV4:
			if (proto[IPPROTO_IP] && proto[IPPROTO_IP]->pr_input)
				return proto[IPPROTO_IP]->pr_input(buf, 0);
			else
				printf("proto[%d] = %p, not called...\n", IPPROTO_IP,
					proto[IPPROTO_IP]);
			break;
		default:
			printf("Couldn't process unknown protocol %04x\n", eth->type);
	}


#ifndef _KERNEL_MODE
	m_freem(buf);
#else
	core->m_freem(buf);
#endif

	return 0;	
}

static void arp_callback(int result, struct mbuf *buf)
{
	if (result == ARP_LOOKUP_FAILED) {
#ifndef _KERNEL_MODE
		m_freem(buf);
#else
		core->m_freem(buf);
#endif
		return;
	}

	IFQ_ENQUEUE(buf->m_pkthdr.rcvif->txq, buf);

	return;
}

#define senderr(e)	{ error = (e); goto bad; }

int ether_output(struct ifnet *ifp, struct mbuf *buf, struct sockaddr *dst,
		 struct rtentry *rt0)
{
	ethernet_header *eth;
	struct ether_device *d = (struct ether_device *)ifp;
	struct rtentry *rt;
	int error = 0;

	if ((ifp->if_flags & (IFF_UP | IFF_RUNNING)) != (IFF_UP | IFF_RUNNING))
		senderr(ENETDOWN);

	if ((rt = rt0) != NULL) {
		if ((rt->rt_flags & RTF_UP) == 0) {
#ifndef _KERNEL_MODE
			rt = rtalloc1(dst, 1);
#else
			rt = core->rtalloc1(dst, 1);
#endif
			if ((rt0 = rt) != NULL)
				rt->rt_refcnt--;
			else
				senderr(EHOSTUNREACH);
		}
		if (rt->rt_flags & RTF_GATEWAY) {
			if (!rt->rt_gwroute)
				goto lookup;
			if (((rt = rt->rt_gwroute)->rt_flags & RTF_UP) == 0) {
#ifndef _KERNEL_MODE
				rtfree(rt0);
#else
				core->rtfree(rt0);
#endif
				rt = rt0;
lookup:	
#ifndef _KERNEL_MODE
				rt->rt_gwroute = rtalloc1(rt->rt_gateway, 1);
#else
				rt->rt_gwroute = core->rtalloc1(rt->rt_gateway, 1);
#endif
			
				if ((rt = rt->rt_gwroute) == NULL)
					senderr(EHOSTUNREACH);
			}
		}
		if (rt->rt_flags & RTF_REJECT) {
printf("flags & RTF_REJECT\n");
			senderr(rt == rt0 ? EHOSTDOWN : EHOSTUNREACH);
		}
	}

#ifndef _KERNEL_MODE
	M_PREPEND(buf, sizeof(ethernet_header));
#else
#define M_LEADINGSPACE(m) \
        ((m)->m_flags & M_EXT ? (m)->m_data - (m)->m_ext.ext_buf : \
         (m)->m_flags & M_PKTHDR ? (m)->m_data - (m)->m_pktdat : \
         (m)->m_data - (m)->m_dat)

        if (M_LEADINGSPACE(buf) >= sizeof(ethernet_header)) {
                buf->m_data -= sizeof(ethernet_header);
                buf->m_len += sizeof(ethernet_header);
        } else 
                buf = core->m_prepend(buf, sizeof(ethernet_header));
        if (buf && buf->m_flags & M_PKTHDR)
                buf->m_pkthdr.len += sizeof(ethernet_header);
#endif

	eth = mtod(buf, ethernet_header*);
	memcpy(&eth->src, &d->e_addr, 6); /* copy in outgoing MAC address */

	if (buf->m_flags & M_BCAST)
		memset(&eth->dest, 0xff, 6);

	if (buf == NULL)
		senderr(ENOMEM);

	switch (dst->sa_family) {
		case AF_INET:
			eth->type = htons(ETHER_IPV4);

			error = arp->resolve(buf, rt0, dst, &eth->dest, &arp_callback);
			if (error == ARP_LOOKUP_QUEUED) {
				return 0; /* not yet resolved */
			}
			/* add code to loopback copy if required */
			break;
		case AF_UNSPEC: /* packet is complete... */
			eth->type = htons(eth->type);
			break;
		default:
			printf("ether_output: Unknown dst type %d!\n", dst->sa_family);
			senderr(EAFNOSUPPORT);
	}
	IFQ_ENQUEUE(ifp->txq, buf);

	return error;
bad:
	printf("bad! %s\n", strerror(error));
	if (buf)
#ifndef _KERNEL_MODE	
		m_free(buf);
#else
		core->m_free(buf);
#endif
	return error;
}

int arpwhohas(struct ether_device *ed, struct in_addr *ia)
{
	return 0;
}


int ether_ioctl(struct ifnet *ifp, int cmd, caddr_t data)
{
	struct ether_device *ed = (struct ether_device*)ifp;
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

	switch (cmd) {
		case SIOCSIFADDR:
			ifp->if_flags |= IFF_UP;
			switch (ifa->ifa_addr->sa_family) {
				case AF_INET:
					ed->i_addr = ((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
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

int ether_dev_attach(ifnet *dev)
{
	status_t status;
	struct ether_device *ed = (struct ether_device *)dev;
	struct sockaddr_dl *sdl = NULL;
	struct ifaddr *ifa;

printf("ether_dev_start %s\n", dev->if_name);

	if (!ed || dev->if_type != IFT_ETHER)
		return -1;

	/* try to get the MAC address */
	status = ioctl(dev->devid, IF_GETADDR, &ed->e_addr, 6);
	if (status < B_OK) {
		printf("Failed to get a MAC address, ignoring %s\n", dev->if_name);
		return -1;
	}

	for (ifa = dev->if_addrlist; ifa; ifa = ifa->ifa_next) {
		if ((sdl = (struct sockaddr_dl*)ifa->ifa_addr) &&
		    sdl->sdl_family == AF_LINK) {
			sdl->sdl_type = IFT_ETHER;
			sdl->sdl_alen = dev->if_addrlen;
			memcpy(LLADDR(sdl), &ed->e_addr, dev->if_addrlen);
			break;
		}
	}	

	dev->if_mtu = ETHERMTU;
	dev->input = &ether_input;
	dev->output = &ether_output;
	dev->stop = &ether_dev_stop;
	dev->ioctl = &ether_ioctl;
	
	dev->if_flags |= (IFF_RUNNING|IFF_BROADCAST|IFF_MULTICAST);
printf("done attaching %s\n", dev->if_name);
	
	return 0;
}

#ifndef _KERNEL_MODE

static int ether_init(void)
{
	find_devices();

	memset(proto, 0, sizeof(struct protosw *) * IPPROTO_MAX);
	add_protosw(proto, NET_LAYER2);

	if (!arp) {
		char path[PATH_MAX];
		getcwd(path, PATH_MAX);
		strcat(path, "/" ARP_MODULE_PATH);

		arpid = load_add_on(path);
		if (arpid > 0) {
			status_t rv = get_image_symbol(arpid, "arp_module_info",
								B_SYMBOL_TYPE_DATA, (void**)&arp);
			if (rv < 0) {
				printf("Failed to get access to ARP!\n");
				return rv;
			}
		} else { 
			printf("Failed to load the arp module...\n");
			return -1;
		}
	}
	arp->init();

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
	get_module(ARP_MODULE_PATH, (module_info **)&arp);

	find_devices();

	memset(proto, 0, sizeof(struct protosw *) * IPPROTO_MAX);

	core->add_protosw(proto, NET_LAYER2);

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
