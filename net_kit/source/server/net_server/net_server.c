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

#include "if.h"	/* for ifnet definition */
#include "net_server/net_server.h"
#include "protocols.h"
#include "net_module.h"
#include "net_timer.h"
#include "net_misc.h"
#include "nhash.h"
#include "sys/socket.h"

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


struct local_address {
        struct local_address *next;
        struct sockaddr *addr;
        ifnet *ifn;
};

net_hash *localhash;
struct local_address *local_addrs;

static int32 rx_thread(void *data)
{
	ifnet *i = (ifnet *)data;
	int count = 0;
	status_t status;
	char buffer[2048];
	size_t len = 2048;

	printf("%s%d: starting rx_thread...\n", i->name, i->unit);
        while ((status = read(i->dev, buffer, len)) >= B_OK && count < 10) {
                struct mbuf *mb = m_devget(buffer, len, 0, i, NULL);
printf("rx: mbuf %p\n", mb);
                global_modules[prot_table[NS_ETHER]].mod->input(mb);
		count++;
		len = 2048;
        }
	printf("%s: terminating rx_thread\n", i->name);
	return 0;
}

static int32 tx_thread(void *data)
{
	ifnet *i = (ifnet *)data;
	struct mbuf *m;
	char buffer[2048];
	size_t len = 0;
	status_t status;

	printf("%s%d: starting tx_thread...\n", i->name, i->unit);	
	while (1) {
		acquire_sem(i->txq->pop);
		IFQ_DEQUEUE(i->txq, m);

		if (m->m_flags & M_PKTHDR) 
			len = m->m_pkthdr.len;
		else 
			len = m->m_len;

		if (len > 2048) {
			printf("%s%d: tx_thread: packet was too big!\n", i->name, i->unit);
			m_freem(m);
			continue;
		}

		m_copydata(m, 0, len, buffer);

printf("TXMIT: %ld bytes to dev %d\n",len ,i->dev);
printf("tx: freeing mbuf %p\n", m);
		m_freem(m);

		status = write(i->dev, buffer, len);
		if (status < B_OK) {
			printf("Error sending data [%s]!\n", strerror(status));
		}
	}
	return 0;
}
	
ifq *start_ifq(void)
{
	ifq *nifq = malloc(sizeof(ifq));

	nifq->lock = create_sem(1, "ifq_lock");
	nifq->pop = create_sem(0, "ifq_pop");

	if (nifq->lock < B_OK || nifq->pop < B_OK)
		return NULL;

	nifq->len = 0;
	nifq->maxlen = 50;
	nifq->head = nifq->tail = NULL;
	return nifq;
}

void net_server_add_device(ifnet *ifn)
{
	if (!ifn)
		return;

	if (devices)
		ifn->next = devices;
	ifn->id = ndevs;
	ndevs++;
	devices = ifn;
}

static void start_devices(void)
{
	ifnet *d = devices;
	while (d) {
		d->rx_thread = spawn_thread(rx_thread, "net_rx_thread", B_NORMAL_PRIORITY,
						d);
		if (d->rx_thread < 0) {
			printf("Failed to start the rx_thread for %s%d\n", d->name, d->unit);
			continue;
		} else {
			resume_thread(d->rx_thread);
		}

		d->txq = start_ifq();
		if (!d->txq) {
			kill_thread(d->rx_thread);
			continue;
		}

		d->tx_thread = spawn_thread(tx_thread, "net_tx_thread", B_NORMAL_PRIORITY,
						d);
                if (d->tx_thread < 0) {
                        printf("Failed to start the tx_thread for %s%d\n", d->name, d->unit);
			kill_thread(d->tx_thread);
                        continue;
                } else {
                        resume_thread(d->tx_thread);
                }
		d = d->next; 
	}
}

static void list_devices(void)
{
	ifnet *d = devices;
	int i = 1;
	printf( "Dev Name         MTU  MAC Address       Flags\n"
		"=== ============ ==== ================= ===========================\n");
	
	while (d) {
		printf("%2d  %s%d       %4d ", i++, d->name, d->unit, d->mtu);
		print_ether_addr((ether_addr*)&d->link_addr->sa_data);
		if (d->flags & IFF_UP)
			printf(" UP");
		if (d->flags & IFF_RUNNING)
			printf(" RUNNING");
		if (d->flags & IFF_PROMISC)
			printf(" PROMISCUOUS");
		if (d->flags & IFF_BROADCAST)
			printf(" BROADCAST");
		if (d->flags & IFF_MULTICAST)
			printf(" MULTICAST");
		printf("\n");
		if (d->if_addrlist) {
			ifaddr *ifa = d->if_addrlist;
			printf("\t\t Addresses:\n");
			while (ifa) {
				printf("\t\t\t");
				if (ifa->if_addr.sa_family == AF_LINK) {
					printf("Link Address: ");
			                print_ether_addr((ether_addr*)&ifa->if_addr.sa_data);
				}
				if (ifa->if_addr.sa_family == AF_INET) {
					printf("IPv4: ");
					print_ipv4_addr((ipv4_addr*)&ifa->if_addr.sa_data);
				}
				printf("\n");
				ifa = ifa->next;
			}
		}

		d = d->next; 
	}
}

static void init_devices(void) {
	ifnet *d = devices;
	int i;

	while (d) {
	 	for (i=0;i<nmods;i++) {
                	printf("calling init_dev for %s%d, %s\n", d->name, d->unit, 
				global_modules[i].mod->name);
			if (global_modules[i].mod->dev_init) {
				global_modules[i].mod->dev_init(d);
			}
		}
		d = d->next;
	}
}

static void close_devices(void)
{
	ifnet *d = devices;
	while (d) {
		kill_thread(d->rx_thread);
		kill_thread(d->tx_thread);
		close(d->dev);
		d = d->next;
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

void insert_local_address(struct sockaddr *sa, ifnet *dev)
{
        if (!nhash_get(localhash, sa->sa_data, sa->sa_len)) {
                struct local_address *la = malloc(sizeof(struct local_address));
                printf("inserting local address\n");
                nhash_set(localhash, sa->sa_data, sa->sa_len, la);
                la->addr = sa;
                la->ifn = dev;
                if (local_addrs)
                        la->next = local_addrs;
                local_addrs = la;
        }
}

ifnet *interface_for_address(struct sockaddr *sa)
{
        struct local_address *la;

        la = (struct local_address*)nhash_get(localhash, sa->sa_data, sa->sa_len);
        if (!la)
                return NULL;
        return la->ifn;
}


int main(int argc, char **argv)
{
	status_t status;
	int i;
	struct sockaddr sa, sb;

	mbinit();
	localhash = nhash_make();

	printf( "Net Server Test App!\n"
		"====================\n\n");

	if (net_init_timer() < B_OK)
		printf("timer service won't work!\n");

	devices = NULL;
	local_addrs = NULL;

	global_modules = malloc(sizeof(loaded_net_module) * MAX_NETMODULES);
	if (!global_modules) {
		printf("Failed to malloc space for net modules list\n");
		exit(-1);
	}

	find_modules();
	init_devices();
	start_devices();

/* These 2 printf's are just for "pretty" display... */
printf("\n");

	list_devices();
	list_modules();

	sa.sa_family = AF_INET;
	sa.sa_len = 4;
	sa.sa_data[0] = 192;
	sa.sa_data[1] = 168;
	sa.sa_data[2] = 0;
	sa.sa_data[3] = 1;

        sb.sa_family = AF_INET;
        sb.sa_len = 4;
        sb.sa_data[0] = 192;
        sb.sa_data[1] = 168;
        sb.sa_data[2] = 0;
        sb.sa_data[3] = 133;

	/* dirst hack to get us sending a request! */
	global_modules[prot_table[NS_ARP]].mod->lookup(&sb, &sa);
snooze(10000);
        global_modules[prot_table[NS_ARP]].mod->lookup(&sb, &sa);
snooze(10000);
        global_modules[prot_table[NS_ARP]].mod->lookup(&sb, &sa);

printf("\n");

	for (i=0;i<ndevs;i++) {
		if (devices[i].rx_thread > 0) {
			wait_for_thread(devices[i].rx_thread, &status);
		}
	}

	close_devices();

	net_shutdown_timer();

	return 0;
}
 
