/* net_server.c */

/* this is just a hack to get us an application that
 * we can develop while we continue to develop the net stack
 */

#include <stdio.h>
#include <kernel/OS.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#ifdef _KERNEL_MODE
#include <Drivers.h>
#include <module.h>
#include <KernelExport.h>

#define spawn_thread spawn_kernel_thread

#endif

#include <driver_settings.h>

#include "core_module.h"

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
#include "net/if_arp.h"
#include "netinet/if_ether.h"

struct ifnet *devices = NULL;
struct ifnet *pdevices = NULL;		/* pseudo devices - loopback etc */
int ndevs = 0;
sem_id dev_lock;

#define NETWORK_INTERFACES	"network/interface"
#define NETWORK_PROTOCOLS	"network/protocol"

/* Forward prototypes... */
static status_t core_std_ops(int32 op, ...);
static int start_stack(void);
static int stop_stack(void);
int net_sysctl(int *name, uint namelen, void *oldp, size_t *oldlenp,
               void *newp, size_t newlen);
static void add_protosw(struct protosw *[], int layer);
static struct in_ifaddr *get_primary_addr(void);
static struct net_module *module_list = NULL;

_EXPORT struct core_module_info core_info = {
	{
		CORE_MODULE_PATH,
		B_KEEP_LOADED,
		core_std_ops
	},

	start_stack,
	stop_stack,
	add_domain,
	add_protocol,
	remove_protocol,
	add_protosw,
	start_rx_thread,
	start_tx_thread,
	
	net_add_timer,
	net_remove_timer,

	pool_init,
	pool_get,
	pool_put,
	pool_destroy,
	
	sonewconn,
	soreserve,
	sbreserve,
	sbappend,
	sbappendaddr,
	sbdrop,
	sbflush,
	sowakeup,
	soisconnected,
	soisconnecting,	
	soisdisconnected,
	soisdisconnecting,
	sohasoutofband,
	socantrcvmore,
	socantsendmore,
	
	in_pcballoc,
	in_pcbdetach,
	in_pcbbind,
	in_pcbconnect,
	in_pcbdisconnect,
	in_pcblookup,
	in_control,
	in_losing,
	in_canforward,
	in_localaddr,
	in_pcbrtentry,
	in_setsockaddr,
	in_setpeeraddr,

	m_gethdr,
	m_get,
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
	sosetopt,
	sogetopt,
	set_socket_event_callback,
	sogetpeername,
	sogetsockname,
	soaccept
};

static int32 if_thread(void *data)
{
	struct ifnet *i = (struct ifnet *)data;
	status_t status;
	char buffer[ETHER_MAX_LEN];
	size_t len = ETHER_MAX_LEN;

	while ((status = read(i->devid, buffer, len)) >= B_OK) {
		struct mbuf *mb = m_devget(buffer, status, 0, i, NULL);
		if (!(i->if_flags & IFF_UP))
			break;
		if (mb) {
			if (i->input)
				i->input(mb);
			atomic_add(&i->if_ipackets, 1);
		}
		len = ETHER_MAX_LEN;
	}
	printf("%s: terminating if_thread\n", i->if_name);
	return 0;
}

/* This is used when we don't have a dev to read/write from as we're using
 * a virtual device, e.g. a loopback driver!
 *
 * It simply queue's the buf's and drags them off as normal in the tx thread.
 */
static int32 rx_thread(void *data)
{
	struct ifnet *i = (struct ifnet *)data;
	struct mbuf *m;

	while (1) {
		acquire_sem_etc(i->rxq->pop, 1, B_CAN_INTERRUPT|B_DO_NOT_RESCHEDULE, 0);
		if (!(i->if_flags & IFF_UP))
			break;
		IFQ_DEQUEUE(i->rxq, m);

		if (i->input)
			i->input(m);
		else
			printf("%s: no input function!\n", i->if_name);
	}
	printf("%s: terminating rx_thread\n", i->if_name);
	return 0;
}

/* This is the same regardless of either method of getting the packets... */
static int32 tx_thread(void *data)
{
	struct ifnet *i = (struct ifnet *)data;
	struct mbuf *m;
	char buffer[2048];
	size_t len = 0;
	status_t status;
#if SHOW_DEBUG
	int txc = 0;
#endif

	while (1) {
		acquire_sem_etc(i->txq->pop,1,B_CAN_INTERRUPT|B_DO_NOT_RESCHEDULE, 0);
		if (!(i->if_flags & IFF_UP))
			break;

		IFQ_DEQUEUE(i->txq, m);

		if (!m)
			continue;
		
		if (m->m_flags & M_PKTHDR) 
			len = m->m_pkthdr.len;
		else 
			len = m->m_len;

		if (len > i->if_mtu) {
			printf("%s: tx_thread: packet was too big!\n", i->if_name);
			m_freem(m);
			continue;
		}

		m_copydata(m, 0, len, buffer);

#if SHOW_DEBUG
		printf("TXMIT %d: %ld bytes to dev %d\n", txc++, len ,i->devid);
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
	printf("%s: terminating tx_thread\n", i->if_name);
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
void start_rx_thread(struct ifnet *dev)
{
	int32 priority = B_NORMAL_PRIORITY;
	char name[B_OS_NAME_LENGTH]; /* 32 */
	sprintf(name, "%s_rx_thread", dev->if_name);

	if (dev->if_type != IFT_ETHER) {
		if (!dev->rxq)
			dev->rxq = start_ifq();
		if (!dev->rxq)
			return;
		dev->rx_thread = spawn_thread(rx_thread, name, 
		                              priority, dev);
	} else {
		/* don't need an rxq... */
		dev->rx_thread = spawn_thread(if_thread, name, 
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
void start_tx_thread(struct ifnet *dev)
{
	int32 priority = B_NORMAL_PRIORITY;
	char name[B_OS_NAME_LENGTH]; /* 32 */
	if (!dev->txq)
		dev->txq = start_ifq();
	if (!dev->txq)
		return;

	sprintf(name, "%s_tx_thread", dev->if_name);
	dev->tx_thread = spawn_thread(tx_thread, name, priority, dev);
	if (dev->tx_thread < 0) {
		printf("Failed to start the tx_thread for %s\n", dev->if_name);
		dev->tx_thread = -1;
		return;
	}
	resume_thread(dev->tx_thread);
}

void net_server_add_device(struct ifnet *ifn)
{
	char dname[16];

	if (!ifn)
		return;

	sprintf(dname, "%s%d", ifn->name, ifn->if_unit);
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
		printf("No devices!\n");
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

/* Hmm, we should do this via the ioctl for the device/module to tell it that
 * the card is going down...
 * XXX - implement this correctly.
 */
static void close_devices(void)
{
	struct ifnet *d = devices;
	while (d) {
		d->if_flags &= ~IFF_UP;
		kill_thread(d->rx_thread);
		kill_thread(d->tx_thread);
		close(d->devid);
		d->if_flags &= ~IFF_RUNNING;		
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
				printf("Don't know how to add domain %d\n", fam);
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
		printf("Unable to add protocol due to no domain available!\n");
		return;
	}
	
	/* OK, we can add it... */
	for (;psw;psw = psw->pr_next) {
		if (psw->pr_type == pr->pr_type &&
		    psw->pr_protocol == pr->pr_protocol &&
		    psw->pr_domain == dm) {
		    printf("duplicate protocol detected!!\n");
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

void remove_protocol(struct protosw *pr)
{
	struct protosw *psw = protocols, *opr = NULL;
	struct domain *dm;
	
	for (;psw;psw = psw->pr_next) {
		if (psw == pr) {
			if (opr) {
				opr->pr_next = psw->pr_next;
			} else {
				/* first entry */
				protocols = psw->pr_next;
			}
			break;
		}
		opr = psw;
	}
	dm = pr->pr_domain;
	opr = NULL;
	for (psw = dm->dom_protosw; psw; psw = psw->dom_next) {
		if (psw == pr) {
			if (opr) 
				opr->dom_next = psw->dom_next;
			else
				dm->dom_protosw = psw->dom_next;
			break;
		}
		opr = psw;
	}
}

static void add_protosw(struct protosw *prt[], int layer)
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
		printf("Domain: %s\n", d->dom_name);
		p = d->dom_protosw;
		for (;p;p = p->dom_next) {
			printf("\t%s provided by %s\n", p->name, p->mod_path);
		}
	}
}

/* Add protocol modules. Each module is loaded and this triggers
 * the init routine which should call the add_domain and add_protocol
 * functions to make sure we know what it does!
 * NB these don't have any additional functions so we just use the
 * system defined module_info structures
 */
#ifdef _KERNEL_MODE
static void find_protocol_modules(void)
{
	void *ml = open_module_list(NETWORK_PROTOCOLS);
	size_t sz = B_PATH_NAME_LENGTH;
	char name[sz];
	struct net_module *nm = NULL; 
	int rv;

	if (ml == NULL) {
		printf("failed to open the %s directory\n", 
			NETWORK_PROTOCOLS);
		return;
	}

	while (read_next_module_name(ml, name, &sz) == B_OK) {
		nm = (struct net_module *)malloc(sizeof(struct net_module));
		if (!nm)
			return;
		memset(nm, 0, sizeof(*nm));
		nm->name = strdup(name);
		printf("module: %s\n", name);
		rv = get_module(name, (module_info **)&nm->ptr);
		if (rv == 0) {
			nm->next = module_list;
			module_list = nm;		
		} else {
			free(nm);
		}
		sz = B_PATH_NAME_LENGTH;
	}

	close_module_list(ml);

	/* we call these here as we want to load all modules ourselves first
	 */
	for (nm = module_list; nm; nm = nm->next) {
		nm->ptr->start(NULL);
		nm->status = 1;
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
	struct net_module *nm = NULL; 
	int rv;

	if (ml == NULL) {
		printf("failed to open the %s directory\n", 
			NETWORK_INTERFACES);
		return;
	}

	while (read_next_module_name(ml, name, &sz) == B_OK) {
		nm = (struct net_module *)malloc(sizeof(struct net_module));
		if (!nm)
			return;
		memset(nm, 0, sizeof(*nm));
		nm->name = strdup(name);
		printf("module: %s\n", name);
		rv = get_module(name, (module_info**)&nm->ptr);
		if (rv == 0) {
			nm->next = module_list;
			module_list = nm;		
			nm->ptr->start(NULL);
			nm->status = 1;
		} else {
			free(nm);
		}
		
		sz = B_PATH_NAME_LENGTH;
	}

	close_module_list(ml);
}

#else

static void find_interface_modules(void)
{
	char path[PATH_MAX], cdir[PATH_MAX];
	DIR *dir;
	struct dirent *fe;
	struct net_module *nm = NULL;
	status_t status;
	
	getcwd(cdir, PATH_MAX);
	sprintf(cdir, "%s/modules/interface", cdir);
	dir = opendir(cdir);
	
	while ((fe = readdir(dir)) != NULL) {
		/* last 2 entries are only valid for development... */
		if (strcmp(fe->d_name, ".") == 0 || strcmp(fe->d_name, "..") == 0
			|| strcmp(fe->d_name, ".cvsignore") == 0 
			|| strcmp(fe->d_name, "CVS") == 0)
                        continue;
		sprintf(path, "%s/%s", cdir, fe->d_name);

		nm = (struct net_module*)malloc(sizeof(struct net_module));
		if (!nm)
			return;
		nm->name = strdup(fe->d_name);
		nm->iid = load_add_on(path);
		if (nm->iid > 0) {		
			status = get_image_symbol(nm->iid, "device_info",
						B_SYMBOL_TYPE_DATA, (void**)&nm->ptr);
			if (status == B_OK) {
				nm->next = module_list;
				module_list = nm;
				nm->ptr->start(&core_info);
				nm->status = 1;
			} else {
				free(nm);
			}
		}
	}
}

static void find_protocol_modules(void)
{
	char path[PATH_MAX], cdir[PATH_MAX];
	DIR *dir;
	struct dirent *m;
	struct net_module *nm = NULL;
	status_t status;

printf("userland: find_protocol_modules...\n");

	getcwd(cdir, PATH_MAX);
	sprintf(cdir, "%s/modules/protocol", cdir);
	dir = opendir(cdir);

	while ((m = readdir(dir)) != NULL) {
		/* last 2 entries are only valid for development... */
		if (strcmp(m->d_name, ".") == 0 || strcmp(m->d_name, "..") == 0
			|| strcmp(m->d_name, ".cvsignore") == 0 
			|| strcmp(m->d_name, "CVS") == 0)
                        continue;
		/* ok so we try it... */
		sprintf(path, "%s/%s", cdir, m->d_name);

		nm = (struct net_module*)malloc(sizeof(struct net_module));
		if (!nm)
			return;
		nm->name = strdup(m->d_name);
		nm->iid = load_add_on(path);
		if (nm->iid > 0) {		
			status = get_image_symbol(nm->iid, "protocol_info",
						B_SYMBOL_TYPE_DATA, (void**)&nm->ptr);
			if (status == B_OK) {
			printf("loaded %s ok\n", path);
				nm->next = module_list;
				module_list = nm;
				nm->ptr->start(&core_info);
				nm->status = 1;
			} else {
				free(nm);
			}
		}
	}

	printf("\n");
}

#endif

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
		if (p->pr_type && p->pr_type == type) {
			return p;
		}
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

	if (namelen < 3) {
		printf("net_sysctl: EINVAL (namelen < 3, %d)\n", namelen);
		return EINVAL; // EISDIR??
	}
	family = name[0];
	protocol = name[1];
	
	if (family == 0)
		return 0;
	
	for (dp=domains; dp; dp= dp->dom_next)
		if (dp->dom_family == family)
			goto found;
	printf("net_sysctl: EPROTOOPT (domain)\n");
	return EINVAL; //EPROTOOPT;
found:
	for (pr=dp->dom_protosw; pr; pr = pr->dom_next) {
		if (pr->pr_protocol == protocol && pr->pr_sysctl) {
			return ((*pr->pr_sysctl)(name+2, namelen -2, oldp, oldlenp, newp, newlen));
		}
	}
	printf("net_sysctl: EPROTOOPT (protocol)\n");
	return EINVAL;//EPROTOOPT;
}

static int start_stack(void)
{
	
	/* have we already been started??? */
	if (domains != NULL)
		return -1;

	net_init_timer();
		
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

	return 0;
}

static int stop_stack(void)
{
	struct net_module *nm = module_list, *onm = NULL;
	
	printf("core network module: Stopping network stack!\n");


	close_devices();

	/* unload all modules... */
	printf("trying to stop modules\n");
	for (;nm; nm = nm->next) {
		if (nm->ptr->stop)
			nm->ptr->stop();
	}
	printf("trying to unload modules\n");
	nm = module_list;
	do {
#ifdef _KERNEL_MODE
		put_module(nm->name);
#else
		unload_add_on(nm->iid);
#endif
		onm = nm;
		nm = nm->next;
		free(onm);
	} while (nm);

	/* This is more likely the correct place for this... 
	 * can't call it at the moment due to the way we call this function!
	sockets_shutdown();
	 */

	return 0;
}


static status_t core_std_ops(int32 op, ...) 
{
	/* XXX ??? - is there anything we should
	 * be doing here?
	 */
	switch(op) {
		case B_MODULE_INIT:
#ifdef LOAD_SYMBOLS
			load_driver_symbols("core");
#endif
			break;
		case B_MODULE_UNINIT:
			// the stack is keeping loaded, so don't stop it
			printf("core: B_MODULE_UNINIT\n");
			break;

		default:
			return B_ERROR;
	}
	return B_OK;
}


#ifdef _KERNEL_MODE
_EXPORT module_info *modules[] = {
	(module_info*) &core_info,
	NULL
};
#else
_EXPORT module_info *modules = (module_info *)&core_info;
#endif
