/* net_server.c */

/* this is just a hack to get us an application that
 * we can develop while we continue to develop the net stack
 */

#ifndef _KERNEL_MODE

#error "This MUST be built as kernel!!"

#else

#include <stdio.h>
#include <kernel/OS.h>
#include <unistd.h>
#include <dirent.h>
#include <Drivers.h>
#include <limits.h>
#include <string.h>
#include <image.h>
#include <stdlib.h>

#include <driver_settings.h>
#include "core_module.h"
#include "net_device.h"

#include "if.h"	/* for ifnet definition */
#include "net_server/net_server.h"
#include "protocols.h"
#include "net_module.h"
#include "net_timer.h"
#include "net_misc.h"
#include "nhash.h"
#include "sys/socket.h"
#include "netinet/in_pcb.h"
#include "net/route.h"

loaded_net_module *global_modules;
static int nmods = 0;
int prot_table[255];

struct ifnet *devices = NULL;
struct ifnet *pdevices = NULL;		/* pseudo devices - loopback etc */
int ndevs = 0;
sem_id dev_lock;
 
#define NETWORK_INTERFACES	"network/interface"
#define NETWORK_PROTOCOLS	"network/protocol"

/* This is just a hack.  Don't know what a sensible figure is... */
#define MAX_DEVICES 16
#define MAX_NETMODULES	20

#define RECV_MSGS	100

struct in_ifaddr * get_primary_addr(void);

static int32 if_thread(void *data)
{
	ifnet *i = (ifnet *)data;
	status_t status;
	char buffer[i->if_mtu];
	size_t len = i->if_mtu;
	int count = 0;

dprintf("if_thread: %s running! calling read() on dev %d\n", 
		i->if_name, i->devid);

	while ((status = read(i->devid, buffer, len)) >= B_OK && count < RECV_MSGS) {
		struct mbuf *mb = m_devget(buffer, status, 0, i, NULL);

dprintf("if_thread: read %d bytes of data!\n", status);

		if (i->input)
			i->input(mb, 0);
		atomic_add(&i->if_ipackets, 1);
		count++;
		len = i->if_mtu;
	}
	dprintf("%s: terminating if_thread\n", i->if_name);
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

	dprintf("%s: starting rx_thread...\n", i->if_name);
	while (1) {
		acquire_sem(i->rxq->pop);
		IFQ_DEQUEUE(i->rxq, m);
		if (i->input)
			i->input(m, 0);
		else
			dprintf("%s%d: no input function!\n", i->name, i->unit);
	}
	dprintf("%s: terminating rx_thread\n", i->if_name);
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

	dprintf("%s: starting tx_thread...\n", i->if_name);	
	while (1) {
		acquire_sem(i->txq->pop);
		IFQ_DEQUEUE(i->txq, m);

		if (m->m_flags & M_PKTHDR) 
			len = m->m_pkthdr.len;
		else 
			len = m->m_len;

		if (len > i->if_mtu) {
			dprintf("%s%d: tx_thread: packet was too big!\n", i->name, i->unit);
			m_freem(m);
			continue;
		}

		m_copydata(m, 0, len, buffer);

#if SHOW_DEBUG
		dprintf("TXMIT %d: %ld bytes to dev %d\n", txc++, len ,i->devid);
#endif
		m_freem(m);
#if SHOW_DEBUG
		dump_buffer(buffer, len);
#endif
		status = write(i->devid, buffer, len);
		if (status < B_OK) {
			printf("Error sending data [%s]!\n", strerror(status));
		}

	}
	return 0;
}
	
ifq *start_ifq(void)
{
	ifq *nifq;
	nifq = malloc(sizeof(ifq));

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
	char dname[16];

	if (!ifn)
		return;
dprintf("net_server_add_device: %p\n", ifn);

	sprintf(dname, "%s%d", ifn->name, ifn->unit);
	ifn->if_name = strdup(dname);
dprintf("device to be added is %s\n", ifn->if_name);

	if (ifn->devid < 0) {
		/* pseudo device... */
		if (pdevices)
			ifn->next = pdevices;
		else
			ifn->next = NULL;
		pdevices = ifn;
	} else {
		if (devices)
			ifn->next = devices;
		else
			ifn->next = NULL;
		ifn->id = ndevs++;
		devices = ifn;
	}
dprintf("add_device: added %s\n", ifn->if_name);	
}

/* For all the devices we need to, here we start the threads
 * they'll use for the rx/tx operations...
 */
static void start_device_threads(void)
{
	ifnet *d = devices;
	char tname[32];
	int priority = B_NORMAL_PRIORITY;
dprintf("start_devices()...\n");

	acquire_sem(dev_lock);

	while (d) {
		sprintf(tname, "%s%d_rx_thread", d->name, d->unit);
dprintf("starting rx_thread...\n");
		if (d->if_type == IFT_ETHER) {
			d->rx_thread = spawn_kernel_thread(if_thread, tname, priority,
							d);
		} else {
			d->rxq = start_ifq();
			d->rx_thread = spawn_kernel_thread(rx_thread, tname, priority,
							d);
		}
		if (d->rx_thread < 0) {
			dprintf("Failed to start the rx_thread for %s%d\n", d->name, d->unit);
			continue;
		} else {
dprintf("resuming rx_thread\n");
			resume_thread(d->rx_thread);
		}
dprintf("starting tx_thread...\n");		
		d->txq = start_ifq();
		if (!d->txq) {
			kill_thread(d->rx_thread);
			continue;
		}

		d->tx_thread = spawn_kernel_thread(tx_thread, "net_tx_thread",
										priority, d);
		if (d->tx_thread < 0) {
			dprintf("Failed to start the tx_thread for %s%d\n", d->name, d->unit);
			kill_thread(d->tx_thread);
			continue;
		} else {
			resume_thread(d->tx_thread);
		}
dprintf("all done, %s has been started!\n", d->if_name);

		d = d->next; 
	}

	release_sem(dev_lock);
}

static void merge_devices(void)
{
	struct ifnet *d = NULL;

	if (!devices && !pdevices) {
		dprintf("No devices!\n");
		return;
	}

	acquire_sem(dev_lock);
	if (devices) {
		/* Now append the pseudo devices and then start them. */
		for (d = devices; d->next != NULL; d = d->next) {
			continue;
		}
	}
	if (pdevices) {
		if (d) {
			d->next = pdevices;
			d = d->next;
		} else {
			devices = pdevices;
			d = devices;
		}
		while (d) {
			d->id = ndevs++;
			d = d->next;
		}
	}
	release_sem(dev_lock);
}

static void list_devices(void)
{
	ifnet *d = devices;
	int i = 1;
	dprintf( "Dev Name         MTU  MAC Address       Flags\n"
		"=== ============ ==== ================= ===========================\n");
	
	while (d) {
		dprintf("%2d  %s%d       %4ld ", i++, d->name, d->unit, d->if_mtu);
		dprintf("                 ");
		if (d->flags & IFF_UP)
			dprintf(" UP");
		if (d->flags & IFF_RUNNING)
			dprintf(" RUNNING");
		if (d->flags & IFF_PROMISC)
			dprintf(" PROMISCUOUS");
		if (d->flags & IFF_BROADCAST)
			dprintf(" BROADCAST");
		if (d->flags & IFF_MULTICAST)
			dprintf(" MULTICAST");
		printf("\n");
		if (d->if_addrlist) {
			ifaddr *ifa = d->if_addrlist;
			dprintf("\t\t Addresses:\n");
			while (ifa) {
				dump_sockaddr(ifa->ifa_addr);
				dprintf("\n");
				ifa = ifa->ifn_next;
			}
		}

		d = d->next; 
	}
}

/* This calls the start function for each device, which should
 * finish the required init (if any) and set the flags such
 * that the device is "up", i.e. IFF_UP should be set when the device
 * returns.
 * Any value other than 0 will lead to the device being removed from 
 * this list. NB it won't be removed from the module that created it at
 * present. XXX - make this happen correctly.
 *
 * This is also where we need to add code to check if we're supposed
 * to start the device!
 * XXX - do this!
 * XXX - need preferences to be done before we can do this.
 */
static void start_devices(void) 
{
	ifnet *d = NULL;
	ifnet *old = NULL;
	int i;

	merge_devices();

	if (devices == NULL)
		return;

	d = devices;
	while (d) {
		if (d->start(d) != 0) {
			if (old)
				old->next = d->next;
		} else 
			old = d;
		d = d->next;
	}
}

static void close_devices(void)
{
	ifnet *d = devices;
	while (d) {
		kill_thread(d->rx_thread);
		kill_thread(d->tx_thread);
		close(d->devid);
		d = d->next;
	}
}

/* This is a little misnamed. This goes through and tries to
 * load all the interface modules it finds and calls each one's
 * init function. The init functions should build a lit of devices
 * that can be used and add each one to the stack. Until we run
 * start_devices() they'll not do anything and other apps can use
 * them (AFAIK), so this shouldn't be an issue.
 */
static void find_interface_modules(void)
{
	void *ml = open_module_list(NETWORK_INTERFACES);
	size_t sz = B_PATH_NAME_LENGTH;
	char name[sz];
	device_module_info *dmi = NULL;
	int rv;

	if (ml == NULL) {
		dprintf("failed to open the %s directory\n", 
			NETWORK_INTERFACES);
		return;
	}

	while (read_next_module_name(ml, name, &sz) == B_OK) {
		rv = get_module(name, (module_info**)&dmi);
		dprintf("get_module(%s) has returned %d\n", name, rv);
		if (rv == 0) {
			dprintf("Loaded %s, running init\n", name);
			dmi->init();
		}

		sz = B_PATH_NAME_LENGTH;
	}

	close_module_list(ml);
}

static void list_modules(void)
{
	int i;

	dprintf("Network modules List\n"
		"====================\n");

	for (i=0;i<nmods;i++) {
		dprintf("%d: %s provided by %s\n", i,
			global_modules[i].mod->name,
			global_modules[i].path);
	}
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

int start_stack(void)
{
	dprintf("core network module: Starting network stack...\n");

	mbinit();
	sockets_init();
	inpcb_init();
	route_init();

	global_modules = malloc(sizeof(loaded_net_module) * 255);
	dev_lock = create_sem(1, "device_lock");
	set_sem_owner(dev_lock, B_SYSTEM_TEAM);

	find_interface_modules();
	start_devices();

	list_devices();
//	start_device_threads();

	list_modules();

dprintf("done starting the stack!!!\n");

	return 0;
}

int stop_stack(void)
{
	dprintf("core network module: Stopping network stack!\n");

	sockets_shutdown();

	close_devices();

	return 0;
}


static status_t core_std_ops(int32 op, ...) 
{
	/* XXX ??? - is there anything we should
	 * be doing here?
	 */
	switch(op) {
		case B_MODULE_INIT:
			break;
		case B_MODULE_UNINIT:
			break;
		default:
			return B_ERROR;
	}
	return B_OK;
}

static struct core_module_info core_info = {
	{
		CORE_MODULE_PATH,
		B_KEEP_LOADED,
		core_std_ops
	},

	start_stack,
	stop_stack,

	soo_ioctl,

	initsocket,
	socreate,
	soclose,
	sobind,

	m_free,
	m_freem,
	m_gethdr,
	m_adj,
	m_prepend,

	net_server_add_device,

	rtalloc1,
	rtfree,
	get_primary_addr
};

_EXPORT module_info *modules[] = {
	(module_info*) &core_info,
	NULL
};

#endif /* we were built by the kernel */
