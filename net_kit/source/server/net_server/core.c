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
#include <stdlib.h>
#include <module.h>
#include <driver_settings.h>
#include <KernelExport.h>

#include "sys/socket.h"
#include "sys/socketvar.h"
#include "net/if.h"	/* for ifnet definition */
#include "net_server/net_server.h"
#include "protocols.h"
#include "net_module.h"
#include "net_timer.h"
#include "net_misc.h"
#include "nhash.h"
#include "netinet/in_var.h"
#include "netinet/in_pcb.h"
#include "sys/domain.h"
#include "sys/protosw.h"
#include "net/route.h"
#include "net_malloc.h"

#include "core_module.h"
#include "net_device.h"

struct ifnet *devices = NULL;
struct ifnet *pdevices = NULL;		/* pseudo devices - loopback etc */
int ndevs = 0;
sem_id dev_lock;
 
#define NETWORK_INTERFACES	"network/interface"
#define NETWORK_PROTOCOLS	"network/protocol"

static int32 if_thread(void *data)
{
	ifnet *i = (ifnet *)data;
	status_t status;
	char buffer[i->if_mtu];
	size_t len = i->if_mtu;
	int count = 0;

	while ((status = read(i->devid, buffer, len)) >= B_OK) {
		struct mbuf *mb = m_devget(buffer, status, 0, i, NULL);

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

	while (1) {
		acquire_sem_etc(i->rxq->pop, 1, B_CAN_INTERRUPT|B_DO_NOT_RESCHEDULE, 0);
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

	while (1) {
		acquire_sem_etc(i->txq->pop,1,B_CAN_INTERRUPT|B_DO_NOT_RESCHEDULE, 0);
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
	ifq *nifq = NULL;
	
	nifq = (ifq*)malloc(sizeof(*nifq));
	memset(nifq, 0, sizeof(*nifq));
	
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

/* Start an RX thread and an RX queue if reqd */
void start_rx_thread(ifnet *dev)
{
	int32 priority = B_NORMAL_PRIORITY;
	char name[B_OS_NAME_LENGTH]; /* 32 */
	sprintf(name, "%s_rx_thread", dev->if_name);

	if (dev->if_type != IFT_ETHER) {
		dev->rxq = start_ifq();
		if (!dev->rxq)
			return;
		dev->rx_thread = spawn_kernel_thread(rx_thread, name, 
		                              priority, dev);
	} else {
		/* don't need an rxq... */
		dev->rx_thread = spawn_kernel_thread(if_thread, name, 
		                              priority, dev);
	}		
	
	if (dev->rx_thread < 0) {
		printf("Failed to start the rx_thread for %s\n", dev->if_name);
		dev->rx_thread = -1;
		return;
	}
	resume_thread(dev->rx_thread);
}

/* Start a TX thread and a TX queue */
void start_tx_thread(ifnet *dev)
{
	int32 priority = B_NORMAL_PRIORITY;
	char name[B_OS_NAME_LENGTH]; /* 32 */
	dev->txq = start_ifq();
	if (!dev->txq)
		return;

	sprintf(name, "%s_tx_thread", dev->if_name);
	dev->tx_thread = spawn_kernel_thread(tx_thread, name, priority, dev);
	if (dev->tx_thread < 0) {
		printf("Failed to start the tx_thread for %s\n", dev->if_name);
		dev->tx_thread = -1;
		return;
	}
	resume_thread(dev->tx_thread);
}

void net_server_add_device(ifnet *ifn)
{
	char dname[16];

	if (!ifn)
		return;

	sprintf(dname, "%s%d", ifn->name, ifn->unit);
	ifn->if_name = strdup(dname);

	if (ifn->if_type != IFT_ETHER) {
		/* pseudo device... */
		if (pdevices)
			ifn->if_next = pdevices;
		else
			ifn->if_next = NULL;
		pdevices = ifn;
	} else {
		if (devices)
			ifn->if_next = devices;
		else
			ifn->if_next = NULL;
		ifn->id = ndevs++;
		devices = ifn;
	}
}

/*
static void merge_devices(void)
{
	struct ifnet *d = NULL;

	if (!devices && !pdevices) {
		dprintf("No devices!\n");
		return;
	}

	acquire_sem(dev_lock);
	if (devices) {
		for (d = devices; d->if_next != NULL; d = d->if_next) {
			continue;
		}
	}
	if (pdevices) {
		if (d) {
			d->if_next = pdevices;
			d = d->if_next;
		} else {
			devices = pdevices;
			d = devices;
		}
		while (d) {
			d->id = ndevs++;
			d = d->if_next;
		}
	}
	release_sem(dev_lock);
}
*/
/*
static void list_devices(void)
{
	ifnet *d = devices;
	int i = 1;
	dprintf( "Dev Name         MTU  MAC Address       Flags\n"
		"=== ============ ==== ================= ===========================\n");
	
	while (d) {
		dprintf("%2d  %s%d       %4ld ", i++, d->name, d->unit, d->if_mtu);
		dprintf("                 ");
		if (d->if_flags & IFF_UP)
			dprintf(" UP");
		if (d->if_flags & IFF_RUNNING)
			dprintf(" RUNNING");
		if (d->if_flags & IFF_PROMISC)
			dprintf(" PROMISCUOUS");
		if (d->if_flags & IFF_BROADCAST)
			dprintf(" BROADCAST");
		if (d->if_flags & IFF_MULTICAST)
			dprintf(" MULTICAST");
		printf("\n");
		if (d->if_addrlist) {
			struct ifaddr *ifa = d->if_addrlist;
			dprintf("\t\t Addresses:\n");
			while (ifa) {
				dump_sockaddr(ifa->ifa_addr);
				dprintf("\n");
				ifa = ifa->ifa_next;
			}
		}

		d = d->if_next; 
	}
}
*/

/*
static void attach_devices(void) 
{
	ifnet *d = NULL;

	merge_devices();

	if (devices == NULL)
		return;

	d = devices;
	while (d) {
		if (d->attach)
			d->attach(d);
		d = d->if_next;
	}
}
*/

static void close_devices(void)
{
	ifnet *d = devices;
	while (d) {
		kill_thread(d->rx_thread);
		kill_thread(d->tx_thread);
		close(d->devid);
		d = d->if_next;
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
				ndm = (struct domain*)malloc(sizeof(*ndm));
				*ndm = af_inet_domain;
				if (dm)
					dm->dom_next = ndm;
				else
					domains = ndm;
				return;
			default:
				dprintf("Don't know how to add domain %d\n", fam);
		}
	} else {
		/* find the end of the chain and add ourselves there */
		for (dm = domains; dm->dom_next;dm = dm->dom_next)
			continue;
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

void add_protosw(struct protosw *prt[], int layer)
{
	struct protosw *p;
	
	for (p = protocols; p; p = p->pr_next) {
		if (p->layer == layer)
			prt[p->pr_protocol] = p;
		if (layer == NET_LAYER3 && p->layer == NET_LAYER2)
			prt[p->pr_protocol] = p;
		if (layer == NET_LAYER1 && p->layer == NET_LAYER2)
			prt[p->pr_protocol] = p;
		if (layer == NET_LAYER2 && p->layer == NET_LAYER3)
			prt[p->pr_protocol] = p;
	}
}

static void domain_init(void)
{
	struct domain *d;
	struct protosw *p;

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

int net_sysctl(int *name, uint namelen, void *oldp, size_t *oldlenp,
               void *newp, size_t newlen)
{
	struct domain *dp;
	struct protosw *pr;
	int family, protocol;

	dprintf("net_sysctl\n");	
	if (namelen < 3) {
		dprintf("net_sysctl: EINVAL (namelen < 3, %d)\n", namelen);
		return EINVAL; // EISDIR??
	}
	family = name[0];
	protocol = name[1];
	
	if (family == 0)
		return 0;
	
	for (dp=domains; dp; dp= dp->dom_next)
		if (dp->dom_family == family)
			goto found;
	dprintf("net_sysctl: EPROTOOPT (domain)\n");
	return EINVAL; //EPROTOOPT;
found:
	for (pr=dp->dom_protosw; pr; pr = pr->dom_next) {
		if (pr->pr_protocol == protocol && pr->pr_sysctl) {
			return ((*pr->pr_sysctl)(name+2, namelen -2, oldp, oldlenp, newp, newlen));
		}
	}
	dprintf("net_sysctl: EPROTOOPT (protocol)\n");
	return EINVAL;//EPROTOOPT;
}

int start_stack(void)
{
	/* have we already been started??? */
	if (domains != NULL)
		return -1;
		
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
	if_init();
	
	dev_lock = create_sem(1, "device_lock");
	set_sem_owner(dev_lock, B_SYSTEM_TEAM);

	find_interface_modules();
	//start_devices();

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
			load_driver_symbols(CORE_MODULE_PATH);
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
	add_protosw,
	start_rx_thread,
	start_tx_thread,

	pool_init,
	pool_get,
	pool_put,
	pool_destroy,
	
	soreserve,
	sbappendaddr,
	sowakeup,
	soisconnected,
	soisdisconnected,
	socantsendmore,
	
	in_pcballoc,
	in_pcbdetach,
	in_pcbbind,
	in_pcbconnect,
	in_pcbdisconnect,
	in_pcblookup,
	in_control,

	m_gethdr,
	m_adj,
	m_prepend,
	m_pullup,
	m_copyback,
	m_copydata,
	m_copym,
	m_free,
	m_freem,
	
	net_server_add_device,
	get_interfaces,
	in_broadcast,
	
	rtalloc,
	rtalloc1,
	rtfree,
	rtrequest,
	rn_addmask,
	rn_head_search,
	get_rt_tables,
	rt_setgate,
	
	ifa_ifwithdstaddr,
	ifa_ifwithnet,
	if_attach,
	ifa_ifwithaddr,
	ifa_ifwithroute,
	ifaof_ifpforaddr,
	ifafree,
	
	get_primary_addr,

	initsocket,
	socreate,
	soclose,
	sobind,
	solisten,
	soconnect,
	recvit,
	sendit,
	soo_ioctl,
	net_sysctl,
	writeit,
	readit,
	soselect,
	sodeselect,
	sosetopt,
	sogetopt
};

_EXPORT module_info *modules[] = {
	(module_info*) &core_info,
	NULL
};

#endif /* we were built by the kernel */
