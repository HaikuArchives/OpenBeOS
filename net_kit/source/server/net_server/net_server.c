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
#include "net_module.h"
#include "net_misc.h"

/* horrible hack to get this building... */
#include "ethernet/ethernet.h"

static loaded_net_module *global_modules;
static int nmods = 0;
static int prot_table[255];

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
                global_modules[prot_table[NS_ETHER]].mod->input(mb);
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
	int dev, rv = 0;

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

	if (global_modules[prot_table[NS_ETHER]].mod->dev_init) {
		/* try to init the device... */
		rv = global_modules[prot_table[NS_ETHER]].mod->dev_init(&devices[ndevs]);
	}

	if (rv) {
		atomic_add(&global_modules[prot_table[NS_ETHER]].ref_count, 1);
		start_rx(ndevs);
		ndevs++;
	}

	return rv;
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
	printf( "Dev Name         MTU  MAC Address       Flags\n"
		"=== ============ ==== ================= ===========================\n");
	
	for (i=0;i<ndevs;i++) {
		printf("%2d  %s       %4d ", i, devices[i].name, devices[i].mtu);
		print_ether_addr(&devices[i].mac);
		if (devices[i].flags & IFF_UP)
			printf(" UP");
		if (devices[i].flags & IFF_RUNNING)
			printf(" RUNNING");
		if (devices[i].flags & IFF_PROMISC)
			printf(" PROMISCUOUS");
		if (devices[i].flags & IFF_BROADCAST)
			printf(" BROADCAST");
		if (devices[i].flags & IFF_MULTICAST)
			printf(" MULTICAST");
		printf("\n");
	}
}

static void close_devices(void)
{
        int i;
        for (i=0;i<ndevs;i++) {
		close(devices[i].dev);
	}
}

static void find_modules(void)
{
	char path[PATH_MAX], cdir[PATH_MAX];
	image_id u;
	net_module *nm;
	DIR *dir;
	struct dirent *m;
	status_t status;

	getcwd(cdir, PATH_MAX);
	sprintf(cdir, "%s/modules", cdir);
	dir = opendir(cdir);

	while ((m = readdir(dir)) != NULL) {
		/* last 2 entries are only valid for development... */
		if (strcmp(m->d_name, ".") == 0 || strcmp(m->d_name, "..") == 0
			|| strcmp(m->d_name, ".cvsignore") == 0 
			|| strcmp(m->d_name, "CVS") == 0)
                        continue;
		/* ok so we try it... */
		sprintf(path, "%s/%s", cdir, m->d_name);
       		u = load_add_on(path);
		if (u > 0) {
			status = get_image_symbol(u, "net_module_data", B_SYMBOL_TYPE_DATA,
						(void**)&nm);
			if (status == B_OK) {
				global_modules[nmods].iid = u;
				global_modules[nmods].ref_count = 0;
				global_modules[nmods].mod = nm;
				printf("Added module: %s\n", global_modules[nmods].mod->name);
				prot_table[nm->proto] = nmods;
				if (nm->init)
					nm->init(global_modules, (int*)&prot_table);
				nmods++;
			} else {
				printf("Found %s, but not a net module.\n", m->d_name);
			}
		} else {
			printf("unable to load %s\n", path);
		}
	}
	printf("\n");
}

static void list_modules(void)
{
	int i;

	printf("\nModules List\n"
		"No. Ref Cnt Proto Name\n"
		"=== ======= ===== ===================\n");
	for (i=0;i<nmods;i++) {
		printf("%02d     %ld     %3d  %s\n", i,
			global_modules[i].ref_count,
			global_modules[i].mod->proto, 
			global_modules[i].mod->name);
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	status_t status;
	int i;

	mbinit();
	printf( "Net Server Test App!\n"
		"====================\n\n");

	devices = malloc(sizeof(ifnet) * MAX_DEVICES);
	if (!devices) {
		printf("Failed to malloc memory for devices!\n");
		exit(-1);
	}

	global_modules = malloc(sizeof(loaded_net_module) * MAX_NETMODULES);
	if (!global_modules) {
		printf("Failed to malloc space for net modules list\n");
		exit(-1);
	}

	find_modules();

	find_devices();
/* These 2 printf's are just for "pretty" display... */
printf("\n");

	list_devices();
	list_modules();

printf("\n");

	for (i=0;i<ndevs;i++) {
		if (devices[i].rx_thread > 0) {
			wait_for_thread(devices[i].rx_thread, &status);
		}
	}

	close_devices();

	return 0;
}
 
