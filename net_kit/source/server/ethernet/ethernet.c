/* ethernet.c
 * ethernet encapsulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <kernel/OS.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include "net_module.h"
#include "protocols.h"
#include "netinet/in_var.h"

#include "ethernet/ethernet.h"

#ifdef _KERNEL_MODE
#include <KernelExport.h>
#include "net_device.h"
#include "net_server/core_module.h"
#include "arp/arp_module.h"

#define ETHERNET_MODULE_PATH	"network/interface/ethernet"

static struct core_module_info *core = NULL;
static struct arp_module_info *arp = NULL;

/* forward prototypes */
int ether_dev_start(ifnet *dev);
int ether_dev_stop(ifnet *dev);
#endif

static loaded_net_module *net_modules;
static int *prot_table;
static struct ether_device *ether_devices = NULL; 	/* list of ethernet devices */

#ifndef _KERNEL_MODE
static net_module *arp = NULL; /* shortcut to arp module */
#endif

int ether_dev_start(ifnet *dev);
int ether_dev_stop(ifnet *dev);

#define DRIVER_DIRECTORY "/dev/net"

uint8 ether_bcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static int convert_proto(uint16 p)
{
	switch (p) {
		case ETHER_ARP:
			return NS_ARP;
		case ETHER_IPV4:
			return NS_IPV4;
		default:
			return -1;
	}
}

static void open_device(char *driver, char *devno)
{
	struct ether_device *ed = malloc(sizeof(struct ether_device));
	char path[PATH_MAX];
	int dev;
	status_t status = -1;

	sprintf(path, "%s/%s/%s", DRIVER_DIRECTORY, driver, devno);
	printf("open_device: %s\n", path);
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
	ed->devid = dev;
	ed->ed_devid = dev;	/* we use this to match... */
	ed->ed_name = strdup(driver);
	ed->ed_unit = atoi(devno);
	ed->ed_type = IFT_ETHER;
	ed->ed_rx_thread = -1;
	ed->ed_tx_thread = -1;
	ed->ed_txq = NULL;
	ed->ed_if_addrlist = NULL;
	ed->ed_hdrlen = 14;
	ed->ed_addrlen = 6;
	ed->ed_start = &ether_dev_start;

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
	net_server_add_device(&ed->ifn);
#else
	core->add_device(&ed->ifn);
#endif

	return;

badcard:
	free(ed);
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
			printf("closing the directory...\n");
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
	uint16 proto = ntohs(eth->type);

        printf("Ethernet packet from ");
        print_ether_addr(&eth->src);
        printf(" to ");
        print_ether_addr(&eth->dest);

        if (buf->m_flags & M_BCAST)
                printf(" BCAST");

        printf(" proto ");
        switch (proto) {
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
                default:
                        printf("unknown (%04x)\n", proto);
        }
}
#endif

/* what should the return value be? */
int ether_input(struct mbuf *buf)
{
	ethernet_header *eth = mtod(buf, ethernet_header *);
	int plen = ntohs(eth->type); /* remove one call to ntohs() */
	int len = sizeof(ethernet_header);
	int fproto = convert_proto(plen);

	if (memcmp((void*)&eth->dest, (void*)&ether_bcast, 6) == 0)
		buf->m_flags |= M_BCAST;
		
	if (plen < 1500) {
		eth802_header *e8 = mtod(buf, eth802_header*);
		printf("It's an 802.x encapsulated packet - type %04x\n", ntohs(e8->type));
		fproto = convert_proto(ntohs(e8->type));
		len = sizeof(eth802_header);
	}

#if SHOW_DEBUG	
	dump_ether_details(buf);
#endif
#ifndef _KERNEL_MODE
	m_adj(buf, len);
#else
	core->m_adj(buf, len);
#endif
	
/*
	if (fproto >= 0 && net_modules[prot_table[fproto]].mod->input) {
		return net_modules[prot_table[fproto]].mod->input(buf, 0);
	} else {
		printf("Failed to determine a valid protocol fproto = %d\n", fproto);
	}
*/
#ifndef _KERNEL_MODE
	m_freem(buf);
#else
	dprintf("input routines not called...\n");
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

	if ((ifp->flags & (IFF_UP | IFF_RUNNING)) != (IFF_UP | IFF_RUNNING))
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

/*
int ether_ioctl(struct ifnet *dev, int cmd, caddr_t data)
{
	struct ifaddr *ifa = (struct ifaddr*)data;
	struct ether_device *ed;

printf("ether_ioctl!\n");

	switch (ifa->ifa_addr->sa_family) {
		case AF_INET:
			ed = (struct ether_device*)dev;
			ed->i_addr = IA_SIN(dev)->sin_addr;
printf("setting ether_device ip address to %08x\n", ntohl(IA_SIN(dev)->sin_addr.s_addr));
			arpwhohas(ed,&IA_SIN(dev)->sin_addr);
			break;
	}

	return 0;
}
*/


int ether_dev_start(ifnet *dev)
{
	status_t status;
	int on = 1;
	struct ether_device *ed = (struct ether_device *)dev;
	char tname[12];
	struct sockaddr_dl *sdl;
	struct ifaddr *ifa;
 	size_t mem_sz, namelen;
	size_t masklen; /* smaller of the 2 */
	size_t socklen; /* the sockaddr_dl with link level address */ 

	if (!ed || dev->if_type != IFT_ETHER)
		return -1;

#ifndef _KERNEL_MODE
	if (!arp && net_modules[prot_table[NS_ARP]].mod)
		arp = net_modules[prot_table[NS_ARP]].mod;
#endif

	sprintf(tname, "%s%d", dev->name, dev->unit); /* make name */
	namelen = strlen(tname);

	/* try to get the MAC address */
	status = ioctl(dev->devid, IF_GETADDR, &ed->e_addr, 6);
	if (status < B_OK) {
		printf("Failed to get a MAC address, ignoring %s%d\n", dev->name, dev->unit);
		return -1;
	}

	/* memory: we need to allocate enough memory for the following...
	 * struct ifaddr
	 * struct sockaddr_dl that will hold the link level address and name
	 * struct sockaddr_dl that will hold the mask
	 */
	masklen = ((int)((caddr_t)&((struct sockaddr_dl*)NULL)->sdl_data[0]))
		  + namelen;
	socklen = masklen + dev->if_addrlen;
	/* round to nearest 4 byte boundry */
	socklen = 1 + ((socklen - 1) | (sizeof(uint32) - 1));

	if (socklen < sizeof(*sdl))
		socklen = sizeof(*sdl);

	mem_sz = sizeof(*ifa) + 2 * socklen;

	ifa = (struct ifaddr*)malloc(mem_sz);
	memset(ifa, 0, mem_sz);

	sdl = (struct sockaddr_dl *)(ifa + 1);
	sdl->sdl_len = socklen;
	sdl->sdl_family = AF_LINK;
	memcpy(&sdl->sdl_data, tname, strlen(tname));
	memcpy((caddr_t)sdl->sdl_data + strlen(tname), &ed->e_addr, 6);
	sdl->sdl_nlen = strlen(tname);
	sdl->sdl_alen = 6;
	sdl->sdl_index = dev->id;
	sdl->sdl_type = dev->if_type;
	ifa->ifn = dev;
	ifa->ifa_next = dev->if_addrlist;
	ifa->ifa_addr = (struct sockaddr*)sdl;
	dev->if_addrlist = ifa;

	/* now do mask... */
	sdl = (struct sockaddr_dl *)((caddr_t)sdl + socklen);
	ifa->ifa_netmask = (struct sockaddr*)sdl;
	sdl->sdl_len = masklen;
	/* build the mask */
	while (socklen != 0)
		sdl->sdl_data[socklen--] = 0xff;

	dev->input = &ether_input;
	dev->output = &ether_output;
#ifdef _KERNEL_MODE
	dev->stop = &ether_dev_stop;
#endif
	/* XXX - ioctl needs to be added back in... */
	
	status = ioctl(dev->devid, IF_SETPROMISC, &on, 1);
	if (status == B_OK) {
		dev->flags |= IFF_PROMISC;
	} else {
		/* not a hanging offence */
		printf("Failed to set %s%d into promiscuous mode\n", dev->name, dev->unit);
	}

	dev->flags |= (IFF_UP|IFF_RUNNING|IFF_BROADCAST|IFF_MULTICAST);
	dev->if_mtu = ETHERMTU;
	
	return 0;
}

#ifndef _KERNEL_MODE

static int ether_init(void)
{
	find_devices();
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

	return 0;
}

static int k_uninit(void)
{
	/* we should really stop all cards etc here! */
	return 0;
}

int ether_dev_stop(ifnet *dev)
{
	dev->flags &= ~IFF_UP;

	/* should find better ways of doing this... */
	kill_thread(dev->rx_thread);
	kill_thread(dev->tx_thread);
	
	return 0;
}

static int32 ether_ops(int32 op, ...)
{
	switch(op) {
		case B_MODULE_INIT:
			if (get_module(CORE_MODULE_PATH,
				(module_info**)&core) != B_OK) {
				dprintf("Failed to get core pointer, declining!\n");
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
