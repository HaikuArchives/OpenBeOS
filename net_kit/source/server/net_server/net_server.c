/* net_server.c */

/* this is just a hack to get us an application that
 * we can develop while we continue to develop the net stack
 */

#include <stdio.h>
#include <kernel/OS.h>
#include <unistd.h>
#include <dirent.h>
#include <Drivers.h>
#include <limits.h>
#include <string.h>
#include <image.h>
#include <stdlib.h>

#include "include/if.h"	/* for ifnet definition */
#include "net_server/net_server.h"
#include "protocols.h"

/* horrible hack to get this building... */
#include "ethernet/ethernet.h"

typedef struct net_modules {
	image_id iid;
	int (*input)(struct mbuf *);
	int proto;
} net_modules;

static net_modules *net_mod;
static int nnmod = 0;

#ifndef _NO_MAIN

static ifnet *devices;
static int ndevs = 0;

/* This is just a hack.  Don't know what a sensible figure is... */
#define MAX_DEVICES 16
#define MAX_NETMODULES	20


static int32 rx_thread(void *data)
{
	ifnet *i = (ifnet *)data;
	int count = 0;
	status_t status;
	char buffer[2048];
	size_t len = 2048;

	printf("%s: starting rx_thread...\n", i->name);
        while ((status = read(i->dev, buffer, len)) >= B_OK && count < 10) {
                struct mbuf *mb = m_devget(buffer, len, 0, NULL);
                ethernet_input(mb);
		count++;
		len = 2048;
        }
	printf("%s: terminating rx_thread\n", i->name);
	return 0;
}

static void start_rx(int device) {
        devices[device].rx_thread = spawn_thread(rx_thread, "net_rx_thread", B_NORMAL_PRIORITY,
                                        &devices[device]);
        if ( devices[device].rx_thread < 0) {
                printf("Failed to start thread for %s\n",  devices[device].name);
        } else {
                resume_thread(devices[device].rx_thread);
        }
}

static int open_device(char *driver, char *devno)
{
	char path[PATH_MAX];
	int dev;
	status_t status;

	sprintf(path, "%s/%s/%s", DRIVER_DIRECTORY, driver, devno);
	dev = open(path, O_RDWR);
	if (dev < B_OK) {
		printf("Couldn't open the device %s\n", path);
		return 0;
	}

	devices[ndevs].dev = dev;
	devices[ndevs].id = ndevs;
	sprintf(path, "%s%s", driver, devno);
	devices[ndevs].name = strdup(path);
	devices[ndevs].type = IFD_ETHERNET;
        devices[ndevs].rx_thread = -1;

	/* try to get the MAC address */
	status = ioctl(dev, IF_GETADDR, &devices[ndevs].mac, 6);
	if (status < B_OK)
		printf("Failed to get a MAC address for %s\n", path);
	else
		start_rx(ndevs);

	return ndevs++;
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

static void list_devices(void)
{
	int i;
	printf( "Dev    Name         MAC Address\n"
		"====== ============ ===========\n");
	
	for (i=0;i<ndevs;i++) {
		printf("%02d     %s       [%02x:%02x:%02x:%02x:%02x:%02x]\n", i, devices[i].name,
			devices[i].mac.addr[0], devices[i].mac.addr[1],
                        devices[i].mac.addr[2], devices[i].mac.addr[3],
                        devices[i].mac.addr[4], devices[i].mac.addr[5]);
	}
}

static void close_devices(void)
{
        int i;
        for (i=0;i<ndevs;i++) {
		close(devices[i].dev);
	}
}
#endif /* _NO_MAIN */

int run_input(int proto, struct mbuf *buf)
{
	int i;

	for (i=0;i<nnmod;i++) {
		if (net_mod[i].proto == proto)
			return net_mod[i].input(buf);
	}
	return 0;
}

#ifndef _NO_MAIN
int main(int argc, char **argv)
{
	image_id u;
	status_t status;
	int i;
	char path[PATH_MAX], cdir[PATH_MAX];

	mbinit();
	printf( "Net Server Test App!\n"
		"====================\n\n");

	devices = malloc(sizeof(ifnet) * MAX_DEVICES);
	if (!devices) {
		printf("Failed to malloc memory for devices!\n");
		exit(-1);
	}

	net_mod = malloc(sizeof(net_modules) * MAX_NETMODULES);
	if (!net_mod) {
		printf("Failed to malloc space for net modules list\n");
		exit(-1);
	}

	getcwd(cdir, PATH_MAX);
	sprintf(path, "%s/udp.so", cdir);
	printf("Trying to load %s\n", path);
	
	/* dirty hack...proof of concept... */
	u = load_add_on(path);
	if (u > 0) {
		net_mod[nnmod].iid = u;
		net_mod[nnmod].proto = PROT_UDP;
		status = get_image_symbol(u, "udp_input", B_SYMBOL_TYPE_TEXT,
				(void**)&net_mod[nnmod].input);
		if (status < B_OK){
			printf("Failed to get the udp_input symbol.\n");
		}
		nnmod++;
		printf("Loaded UDP add-on succesfully!\n");
	} else {
		printf("Failed to load the udp add-on [%ld]! %s\n", u, strerror(u));
	}


	find_devices();
/* These 2 printf's are just for "pretty" display... */
printf("\n");

	list_devices();

printf("\n");

	for (i=0;i<ndevs;i++) {
		if (devices[i].rx_thread > 0) {
			wait_for_thread(devices[i].rx_thread, &status);
		}
	}

	close_devices();

	return 0;
}
#endif /* _NO_MAIN */
 
