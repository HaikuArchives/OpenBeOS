/* ethernet.c
 * ethernet encapsulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <kernel/OS.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include "net_misc.h"
#include "protocols.h"
#include "ethernet.h"
#include "mbuf.h"
#include "net_module.h"
#include "include/if.h"

static loaded_net_module *net_modules;
static int *prot_table;

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
	ifnet *ifn = malloc(sizeof(ifnet));
        char path[PATH_MAX];
        int dev;
	status_t status;

	sprintf(path, "%s/%s/%s", DRIVER_DIRECTORY, driver, devno);
	dev = open(path, O_RDWR);
	if (dev < B_OK) {
		/* we just silently ignore the card */
		//printf("Couldn't open the device %s%s\n", driver, devno);
		free(ifn);
		return;
	}

	status = ioctl(dev, IF_INIT, NULL, 0);
	if (status < B_OK) {
		/* we just silently ignore the card */
		//printf("Failed to init %s%s!\n", driver, devno);
		free(ifn);
		return;
	}

	ifn->dev = dev;
	ifn->name = strdup(driver);
	ifn->unit = atoi(devno);
	ifn->type = IFD_ETHERNET;
	ifn->rx_thread = -1;
	ifn->tx_thread = -1;
	ifn->txq = NULL;
	ifn->if_addrlist = NULL;
	ifn->link_addr = NULL;

	net_server_add_device(ifn);
	atomic_add(&net_modules[prot_table[NS_ETHER]].ref_count, 1);
}

static void find_devices(void)
{
        DIR *dir;
        DIR *driv_dir;
        struct dirent *de;
        struct dirent *dre;
        char path[PATH_MAX];

        dir = opendir(DRIVER_DIRECTORY);
        if (!dir) {
                printf("Couldn't open the directory %s\n", DRIVER_DIRECTORY);
                return;
        }

        while ((de = readdir(dir)) != NULL) {
                /* hmm, is it a driver? */
                if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
                        continue;

                /* OK we assume it's a driver...but skip the ether driver
                 * as I don't really know what it is!
                 */
                if (strcmp(de->d_name, "ether") == 0)
                        continue;

                sprintf(path, "%s/%s", DRIVER_DIRECTORY, de->d_name);
                driv_dir = opendir(path);
                if (!driv_dir) {
                        printf("I coudln't find any drivers in the %s driver directory",
                                de->d_name);
                } else {
                        while ((dre = readdir(driv_dir)) != NULL) {
                                if (!strcmp(dre->d_name, "0")) {
                                        open_device(de->d_name, dre->d_name);
                                }
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
/* should probably also pass a structure that identifies the interface */
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

	m_adj(buf, len);
	
	if (fproto >= 0 && net_modules[prot_table[fproto]].mod->input) {
		return net_modules[prot_table[fproto]].mod->input(buf);
	} else {
		printf("Failed to determine a valid protocol fproto = %d\n", fproto);
	}

	m_freem(buf);
	return 0;	
}

static void arp_callback(int result, struct mbuf *buf,  struct sockaddr *tgt)
{
	ethernet_header *eth =  mtod(buf, ethernet_header*);

	if (result == ARP_LOOKUP_FAILED) {
		m_freem(buf);
		return;
	}

	memcpy(&eth->dest, &tgt->sa_data, 6);

	IFQ_ENQUEUE(buf->m_pkthdr.rcvif->txq, buf);

	return;
}

int ether_output(struct mbuf *buf, int prot, struct sockaddr *tgt)
{
	ethernet_header *eth;

	M_PREPEND(buf, sizeof(ethernet_header));
	eth = mtod(buf, ethernet_header*);

	memcpy(&eth->src, &buf->m_pkthdr.rcvif->link_addr->sa_data,
		buf->m_pkthdr.rcvif->link_addr->sa_len);

	if (prot == NS_ARP) {
		eth->type = htons(ETHER_ARP);
		/* hack - we assume the sockaddr has a valid link address */
	}
	if (prot == NS_IPV4) {
		int rv = ARP_LOOKUP_FAILED;

		if (tgt->sa_family != AF_INET) {
			/* oh dear! We can't go on from here as we're looking for an ipv4
			 * address and we haven't been given one to send to! Doh!
			 */
			m_freem(buf);
			return 0;
		}

		eth->type = htons(ETHER_IPV4);
		rv = net_modules[prot_table[NS_ARP]].mod->lookup(buf, tgt, &arp_callback);
		/* if we failed, free the mbuf */
		if (rv == ARP_LOOKUP_FAILED)
			m_freem(buf);
		/* if we didn't succeed, we're returning. If we've been queued then the callback
		 * will take care of it and we won't have freed the mbuf, if we failed outright
		 * then we'll have freed the mbuf and will be exiting.
		 */
		if (rv != ARP_LOOKUP_OK)
			return 0;
	}

	memcpy(&eth->dest, &tgt->sa_data, 6);

	IFQ_ENQUEUE(buf->m_pkthdr.rcvif->txq, buf);

	return 0;
}

int ether_init(loaded_net_module *ln, int *pt)
{
	net_modules = ln;
	prot_table = pt;

	find_devices();
	return 0;
}

int ether_dev_init(ifnet *dev)
{
	status_t status;
	int on = 1;
	ifaddr *ifa = malloc(sizeof(ifaddr));

	if (dev->type != IFD_ETHERNET)
		return 0;

        /* try to get the MAC address */
        status = ioctl(dev->dev, IF_GETADDR, &ifa->if_addr.sa_data, 6);
        if (status < B_OK) {
                printf("Failed to get a MAC address, ignoring %s%d\n", dev->name, dev->unit);
                return 0;
        }
printf("Adding input func for %s%d\n", dev->name, dev->unit);
        dev->input = &ether_input;
        dev->output = &ether_output;

	/* Add the link address to address list for device */
	ifa->if_addr.sa_len = 6;
	ifa->if_addr.sa_family = AF_LINK;
	ifa->ifn = dev;

	ifa->next = NULL;
	if (dev->if_addrlist)
		dev->if_addrlist->next = ifa;
	else
		dev->if_addrlist = ifa;
	/* also add link from dev->link_addr */
	dev->link_addr = &ifa->if_addr;
	insert_local_address(dev->link_addr, dev);

        status = ioctl(dev->dev, IF_SETPROMISC, &on, 1);
        if (status == B_OK) {
		dev->flags |= IFF_PROMISC;
	} else {
		/* not a hanging offence */
                printf("Failed to set %s%d into promiscuous mode\n", dev->name, dev->unit);
        }

	dev->flags |= (IFF_UP|IFF_RUNNING|IFF_BROADCAST|IFF_MULTICAST);
	dev->mtu = ETHERMTU;
		
	return 1;
}

net_module net_module_data = {
	"Ethernet/802.x module",
	NS_ETHER,
	NET_LAYER1,
        0,      /* users can't create sockets in this module! */
        0,

	&ether_init,
	&ether_dev_init,
	&ether_input,
	&ether_output,
	NULL
};

