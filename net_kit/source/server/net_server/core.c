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
#include "sys/domain.h"
#include "sys/protosw.h"

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

	while ((status = read(i->devid, buffer, len)) >= B_OK && count < RECV_MSGS) {
		struct mbuf *mb = m_devget(buffer, status, 0, i, NULL);

dprintf("if_thread: read %d bytes of data!\n", status);

		if (i->input)
			i->input(mb);
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
 * It simply queue's the buf's and drags them off as normal in the tx thread.
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
			i->input(m);
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
#if SHOW_MALLOC_USAGE
	dprintf("core.c: start_ifq: malloc(%ld)\n", sizeof(ifq));
#endif
	nifq = malloc(sizeof(ifq));

	nifq->lock = create_sem(1, "ifq_lock");
	nifq->pop = create_sem(0, "ifq_pop");
	set_sem_owner(nifq->lock, B_SYSTEM_TEAM);
	set_sem_owner(nifq->pop, B_SYSTEM_TEAM);
	
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

	sprintf(dname, "%s%d", ifn->name, ifn->unit);
dprintf("net_server_add_device: %s\n", dname);
	ifn->if_name = strdup(dname);
dprintf("ifn->if_name = %s\n", ifn->if_name);
dprintf("devices = %p\n", devices);

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
dprintf("device was id %d\n", ndevs - 1);
}

/* For all the devices we need to, here we start the threads
 * they'll use for the rx/tx operations...
 */
static void start_device_threads(void)
{
	ifnet *d = devices;
	char tname[32];
	int priority = B_NORMAL_PRIORITY;

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
				ifa = ifa->ifa_next;
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
			/* if we had a problem, remove the device... */
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

static struct domain af_inet_domain = {
	AF_INET,
	"internet",
	NULL,
	NULL,
	NULL,
	rn_inithead,
	32,
	sizeof(struct sockaddr_in)
};
	
/* Domain support */
void add_domain(struct domain *dom, int fam)
{
	struct domain *dm = domains;
	struct domain *ndm;

dprintf("add_domain: %d\n", fam);

	for(; dm; dm = dm->dom_next) {
		if (dm->dom_family == fam)
			/* already done */
			return;
	}	

	if (dom == NULL) {
		/* we're trying to add a builtin domain! */

		switch (fam) {
			case AF_INET:
				/* ok, add it... */
#if SHOW_MALLOC_USAGE
	dprintf("core.c: add_domain: malloc(%ld)\n", sizeof(*ndm));
#endif
				ndm = (struct domain*)malloc(sizeof(*ndm));
				*ndm = af_inet_domain;
				if (dm)
					dm->dom_next = ndm;
				else
					domains = ndm;
				dprintf("Added AF_INET domain\n");
				return;
			default:
				dprintf("Don't know how to add domain %d\n", fam);
		}
	} else {
		if (dm)
			dm->dom_next = dom;
		else
			domains = dom;
	}
	return;
}

void add_protocol(struct protosw *pr, int fam)
{
	struct protosw *psw = protocols;
	struct domain *dm = domains;
	
	dprintf("add_protocol: %s\n", pr->name);
	
	/* first find the correct domain... */
	for (; dm; dm= dm->dom_next) {
		if (dm->dom_family == fam)
			break;
	}

	if (dm == NULL) {
		dprintf("Unable to add protocol due to no domain available!\n");
		return;
	}
	
	/* OK, we can add it... */
	for (;psw;psw = psw->pr_next) {
		if (psw->pr_type == pr->pr_type &&
		    psw->pr_protocol == pr->pr_protocol &&
		    psw->pr_domain == dm) {
		    dprintf("duplicate protocol detected!!\n");
			return;
		}
	}

	/* find last entry in protocols list */
	if (protocols) {
		for (psw = protocols;psw->pr_next; psw = psw->pr_next)
			continue;
		psw->pr_next = pr;
	} else
		protocols = pr;

	pr->pr_domain = dm;

	/* Now add to domain */
	if (dm->dom_protosw) {
		psw = dm->dom_protosw;
		for (;psw->dom_next;psw = psw->dom_next)
			continue;
		psw->dom_next = pr;
	} else {
		dm->dom_protosw = pr;
	}

	return;
}

static void domain_init(void)
{
	struct domain *d;
	struct protosw *p;

dprintf("domain_init()\n");
	
	for (d = domains;d;d = d->dom_next) {
		if (d->dom_init)
			d->dom_init();

		for (p = d->dom_protosw;p;p = p->dom_next) {
			if (p->pr_init)
				p->pr_init();
		}
	}
}

static void walk_domains(void)
{
	struct domain *d;
	struct protosw *p;
	
	for (d = domains;d;d = d->dom_next) {
		dprintf("Domain: %s\n", d->dom_name);
		p = d->dom_protosw;
		for (;p;p = p->dom_next) {
			dprintf("\t%s provided by %s\n", p->name, p->mod_path);
		}
	}
}

/* Add protocol modules. Each module is loaded and this triggers
 * the init routine which should call the add_domain and add_protocol
 * functions to make sure we know what it does!
 * NB these don't have any additional functions so we just use the
 * system defined module_info structures
 */
static void find_protocol_modules(void)
{
	void *ml = open_module_list(NETWORK_PROTOCOLS);
	size_t sz = B_PATH_NAME_LENGTH;
	char name[sz];
	module_info *dmi = NULL;
	int rv;

	if (ml == NULL) {
		dprintf("failed to open the %s directory\n", 
			NETWORK_PROTOCOLS);
		return;
	}

	while (read_next_module_name(ml, name, &sz) == B_OK) {
		rv = get_module(name, &dmi);
		sz = B_PATH_NAME_LENGTH;
	}

	close_module_list(ml);
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
		if (rv == 0) {
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


struct protosw *pffindtype(int domain, int type)
{
	struct domain *d;
	struct protosw *p;
	
	for (d = domains; d; d = d->dom_next) {
		if (d->dom_family == domain)
			goto found;
	}
	return NULL;
found:
	for (p=d->dom_protosw; p; p = p->dom_next) {
		if (p->pr_type && p->pr_type == type)
			return p;
	}
	return NULL;
}

struct protosw *pffindproto(int domain, int protocol, int type)
{
	struct domain *d;
	struct protosw *p, *maybe = NULL;

	if (domain == 0)
		return NULL;
	
	for (d = domains; d; d = d->dom_next) {
		if (d->dom_family == domain)
			goto found;
	}
	return NULL;

found:
	for (p=d->dom_protosw;p;p = p->dom_next) {
		if (p->pr_protocol == protocol && p->pr_type == type)
			return p;
		/* deal with SOCK_RAW and AF_UNSPEC */
		if (type == SOCK_RAW && p->pr_type == SOCK_RAW &&
			p->pr_protocol == AF_UNSPEC && maybe == NULL)
			maybe = p;
	}
	return maybe;
}

int start_stack(void)
{
	/* have we already been started??? */
	if (domains != NULL)
		return;
		
	dprintf("core network module: Starting network stack...\n");

	domains = NULL;
	protocols = NULL;
	devices = NULL;
	pdevices = NULL;
	
	find_protocol_modules();
	
	walk_domains();
	
	domain_init();
	
	mbinit();
	sockets_init();
	inpcb_init();
	route_init();

#if SHOW_MALLOC_USAGE
	dprintf("core.c: start_ifq: malloc(%ld)\n", 
		sizeof(loaded_net_module) * 255);
#endif

	global_modules = malloc(sizeof(loaded_net_module) * 255);
	dev_lock = create_sem(1, "device_lock");
	set_sem_owner(dev_lock, B_SYSTEM_TEAM);

	find_interface_modules();
	start_devices();

	list_devices();
	start_device_threads();

	list_modules();

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
	add_domain,
	add_protocol,

	soo_ioctl,

	initsocket,
	socreate,
	soclose,
	sobind,
	soreserve,
	sbappendaddr,
	sowakeup,
	
	in_pcballoc,
	in_pcbdetach,
	in_pcbbind,
	in_pcbconnect,
	in_pcbdisconnect,
	in_pcblookup,

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
