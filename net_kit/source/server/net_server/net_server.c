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
#include "netinet/in_pcb.h"

static loaded_net_module *global_modules;
static int nmods = 0;
static int prot_table[255];

static ifnet *devices;
static int ndevs = 0;

/* This is just a hack.  Don't know what a sensible figure is... */
#define MAX_DEVICES 16
#define MAX_NETMODULES	20

#define RECV_MSGS	100

struct local_address {
        struct local_address *next;
        struct sockaddr addr;
        ifnet *ifn;
};

net_hash *localhash;
struct local_address *local_addrs;

static int32 if_thread(void *data)
{
	ifnet *i = (ifnet *)data;
	status_t status;
	char buffer[i->mtu];
	size_t len = i->mtu;
	int count = 0;

	while ((status = read(i->dev, buffer, len)) >= B_OK && count < RECV_MSGS) {
		struct mbuf *mb = m_devget(buffer, status, 0, i, NULL);
		if (i->input)
			i->input(mb);

		count++;
		len = i->mtu;
	}
	printf("%s: terminating if_thread\n", i->name);
	return 0;
}

/* This is used when we don't have a dev to read/write from as we're using
 * a virtual device, e.g. a loopback driver!
 *
 * It simply queue's the buf's and drgas them off as normal in the tx thread.
 */
static int32 rx_thread(void *data)
{
	ifnet *i = (ifnet *)data;
	struct mbuf *m;

	printf("%s%d: starting rx_thread...\n", i->name, i->unit);
        while (1) {
		acquire_sem(i->rxq->pop);
		IFQ_DEQUEUE(i->rxq, m);
		if (i->input)
                	i->input(m);
		else
			printf("%s%d: no input function!\n", i->name, i->unit);
        }
	printf("%s%d: terminating rx_thread\n", i->name, i->unit);
	return 0;
}

/* This is the same regardless of either method of getting the packets... */
static int32 tx_thread(void *data)
{
	ifnet *i = (ifnet *)data;
	struct mbuf *m;
	char buffer[2048];
	size_t len = 0;
	status_t status;
#if SHOW_DEBUG
	int txc = 0;
#endif

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

#if SHOW_DEBUG
		printf("TXMIT %d: %ld bytes to dev %d\n", txc++, len ,i->dev);
#endif
		m_freem(m);
		/* this is soooooooo useful, but not currently needed */
		//dump_buffer(buffer, len);
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
	char tname[32];
	int priority = B_REAL_TIME_DISPLAY_PRIORITY;

	while (d) {
		sprintf(tname, "%s%d_rx_thread", d->name, d->unit);
		if (d->type == IFD_ETHERNET) {
			d->rx_thread = spawn_thread(if_thread, tname, priority,
							d);
		} else {
			d->rxq = start_ifq();
			d->rx_thread = spawn_thread(rx_thread, tname, priority,
							d);
		}
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

		d->tx_thread = spawn_thread(tx_thread, "net_tx_thread", priority,
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
	printf("\n");
}

static void list_devices(void)
{
	ifnet *d = devices;
	int i = 1;
	printf( "Dev Name         MTU  MAC Address       Flags\n"
		"=== ============ ==== ================= ===========================\n");
	
	while (d) {
		printf("%2d  %s%d       %4d ", i++, d->name, d->unit, d->mtu);
		if (d->link_addr) {
			print_ether_addr((ether_addr*)&d->link_addr->sa_data);
		} else {
			printf("                 ");
		}
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
				if (nmods > 0)
					global_modules[nmods].next = &global_modules[nmods -1];
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
                la->addr = *sa;
                la->ifn = dev;
		nhash_set(localhash, &la->addr.sa_data, la->addr.sa_len, la);
                if (local_addrs)
                        la->next = local_addrs;
                local_addrs = la;
		if (!in_ifaddr && dev->type == IFD_ETHERNET && sa->sa_family == AF_INET)
			in_ifaddr = &la->addr;
	}
}

int is_address_local(void *ptr, int len)
{
	struct local_address *la;

	la = (struct local_address*)nhash_get(localhash, ptr, len);
	if (la)
		return 1;
	return 0;
}

ifnet *interface_for_address(void *data, int len)
{
        struct local_address *la;

        la = (struct local_address*)nhash_get(localhash, data, len);
        if (!la)
                return NULL;
        return la->ifn;
}

net_module *pffindtype(int domain, int type)
{
        int i;
        net_module *n;

	for (i=0;i<nmods;i++) {
                n = global_modules[i].mod;

		if (n->domain == domain && n->sock_type == type)
			return n;
	}
	return NULL;
}

net_module *pffindproto(int domain, int protocol, int type)
{
        int i;
        net_module *n;

        for (i=0;i<nmods;i++) {
                n = global_modules[i].mod;

                if (n->domain == domain &&
                        n->proto == protocol &&
                        n->sock_type == type)
                        return n;
        }
        return NULL;
}

int main(int argc, char **argv)
{
	status_t status;
	ifnet *d;
	int s, rv, i;
	struct sockaddr_in sin;
	char data[100];
	char *bigbuf = malloc(sizeof(char) * 1024);

	mbinit();
	localhash = nhash_make();

	sockets_init();
	inpcb_init();

	in_ifaddr = NULL;

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

	if (ndevs == 0) {
		printf("\nFATAL: no devices configured!\n");
		exit(-1);
	}

	list_devices();
	list_modules();

	/* Just to see if it works! */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(7777);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((rv = bind(s, (struct sockaddr*)&sin, sizeof(sin))))
		printf("Call to bind failed. [%s]\n", strerror(rv));
	else 
		printf("Call to bind was OK: port = %d\n", ntohs(sin.sin_port));


	for (i=0;i<100;i+=2) {
		data[i] = 0xde;
		data[i+1] = 0xad;
	}

	sin.sin_addr.s_addr = htonl(0xc0a8006e);
	rv = sendto(s, data, 100, 0, (struct sockaddr*)&sin, sizeof(sin));
	if (rv < 0)
		printf("sendto gave %d [%s]\n", rv, strerror(rv));
	else
		printf("WooHoo - we sent %d bytes!\n", rv);

	rv = recvfrom(s, bigbuf, 1024, 0, (struct sockaddr*)&sin, sizeof(sin));
	if (rv < 0)
		printf("recvfrom gave %d [%s]\n", rv, strerror(rv));
	else
		printf("WooHoo - we got %d bytes!\n%s\n", rv, bigbuf);

	d = devices;
	while (d) {
		if (d->rx_thread  && d->type == IFD_ETHERNET) {
			printf("waiting on thread for %s%d\n", d->name, d->unit);
			wait_for_thread(d->rx_thread, &status);
		}
		d = d->next; 
	}

	sockets_shutdown();

	close_devices();

	net_shutdown_timer();

	return 0;
}
 
