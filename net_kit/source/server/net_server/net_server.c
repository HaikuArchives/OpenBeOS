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

#include "net/if.h"	/* for ifnet definition */
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
#include "sys/sockio.h"
#include "net/if_arp.h"
#include "netinet/if_ether.h"

struct ifnet *devices = NULL;
struct ifnet *pdevices = NULL;		/* pseudo devices - loopback etc */
int ndevs = 0;
sem_id dev_lock;

static int32 if_thread(void *data)
{
	ifnet *i = (ifnet *)data;
	status_t status;
	char buffer[ETHER_MAX_LEN];
	size_t len = ETHER_MAX_LEN;

	while ((status = read(i->devid, buffer, len)) >= B_OK) {
		struct mbuf *mb = m_devget(buffer, status, 0, i, NULL);
		/* Hmm, not sure about this... */
		if ((i->if_flags & IFF_UP) == 0) {
			m_freem(mb);
			continue;
		}
		printf("read %ld bytes\n", status);
		if (i->input)
			i->input(mb);
			
		atomic_add(&i->if_ipackets, 1);
		len = ETHER_MAX_LEN;
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
	
	while (1) {
		acquire_sem(i->rxq->pop);
		IFQ_DEQUEUE(i->rxq, m);

		if ((i->if_flags & IFF_UP) == 0) {
			m_freem(m);
			continue;
		}
		
		if (i->input)
			i->input(m);
		else
			printf("%s%d: no input function!\n", i->name, i->if_unit);

	}
	printf("%s%d: terminating rx_thread\n", i->name, i->if_unit);
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
		acquire_sem(i->txq->pop);
		IFQ_DEQUEUE(i->txq, m);

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
		printf("TXMIT %d: %ld bytes to dev %s[%d]\n", txc++, len ,i->if_name, i->devid);
#endif
		m_freem(m);
#if SHOW_DEBUG
		printf("output: (%d bytes)\n", len);
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

	sprintf(dname, "%s%d", ifn->name, ifn->if_unit);
	ifn->if_name = strdup(dname);

	acquire_sem(dev_lock);
	
	if (ifn->devid < 0) {
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
	release_sem(dev_lock);
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

//	merge_devices();

	if (devices == NULL)
		return;

	d = devices;
	while (d) {
		if (d->start)
			d->start(d);
		d = d->if_next;
	}
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
void start_tx_thread(ifnet *dev)
{
	int32 priority = B_NORMAL_PRIORITY;
	char name[B_OS_NAME_LENGTH]; /* 32 */
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
	
static void list_devices(void)
{
	ifnet *d = devices;
	int i = 1;
	printf( "Dev Name         MTU  MAC Address       Flags\n"
		"=== ============ ==== ================= ===========================\n");
	
	while (d) {
		printf("%2d  %s%d       %4ld ", i++, d->name, d->if_unit, d->if_mtu);
		printf("                 ");
		if (d->if_flags & IFF_UP)
			printf(" UP");
		if (d->if_flags & IFF_RUNNING)
			printf(" RUNNING");
		if (d->if_flags & IFF_PROMISC)
			printf(" PROMISCUOUS");
		if (d->if_flags & IFF_BROADCAST)
			printf(" BROADCAST");
		if (d->if_flags & IFF_MULTICAST)
			printf(" MULTICAST");
		printf("\n");
		if (d->if_addrlist) {
			struct ifaddr *ifa = d->if_addrlist;
			while (ifa) {
				dump_sockaddr(ifa->ifa_addr);
				ifa = ifa->ifa_next;
			}
		}

		d = d->if_next; 
	}
}

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

static void find_interface_modules(void)
{
	char path[PATH_MAX], cdir[PATH_MAX];
	image_id id;
	DIR *dir;
	struct dirent *fe;
	struct device_info *di;
	
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
		id = load_add_on(path);
		if (id > 0) {
			
			status = get_image_symbol(id, "device_info",
						B_SYMBOL_TYPE_DATA, (void**)&di);
			if (status == B_OK)
				di->init();
		}
	}
}
		
static void find_protocol_modules(void)
{
	char path[PATH_MAX], cdir[PATH_MAX];
	image_id u;
	struct protocol_info *pi;
	DIR *dir;
	struct dirent *m;
	status_t status;

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
		u = load_add_on(path);

		if (u > 0) {
			status = get_image_symbol(u, "protocol_info", 
					B_SYMBOL_TYPE_DATA, (void**)&pi);
			if (status == B_OK) {
				pi->init();
			} else {
				printf("unable to load %s\n", path);
			}
		}
	}

	printf("\n");
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
		/* find the last domain and add ourselves at the end */
		for (dm = domains; dm->dom_next; dm = dm->dom_next)
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

void add_protosw(struct protosw *prt[], int layer)
{
	struct protosw *p = protocols;
	
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

int start_stack(void)
{
	/* we use timers in TCP, so init our timer here...
	 * kernel version uses kernel timer so doesn't need this!
	 */
	net_init_timer();
	find_protocol_modules();
	
	domain_init();
	walk_domains();
	
	mbinit();
	sockets_init();
	inpcb_init();
	route_init();
	
	dev_lock = create_sem(1, "device lock");

	find_interface_modules();
	start_devices();
	
	return 0;
}

int stop_stack(void)
{
	sockets_shutdown();

	close_devices();

	net_shutdown_timer();

	return 0;
}

int32 create_sockets(void *data)
{
	int howmany = *(int*)data;
	int intval = howmany / 10;
	void *sp = NULL;
	int rv, cnt = 0;
	bigtime_t tnow;

	printf("\n*** socket creation test ***\n\n");
	
#define stats(x)	printf("Total sockets created: %d\n", x);

	tnow = real_time_clock_usecs();
	while (cnt < howmany) {
		rv = initsocket(&sp);
		if (rv != 0) {
			printf("failed to create a new empty socket!\n");
			stats(cnt);
			return -1;
		} else {
			rv = socreate(AF_INET, sp, SOCK_DGRAM, 0);
			if (rv != 0) {
				printf("socreate failed! %d [%s]\n", rv, strerror(rv));
				stats(cnt);
				return -1;
			}
		}
		rv = soclose(sp);
		if (rv != 0) {
			printf("Error closing socket! %d [%s]\n", rv, strerror(rv));
			stats(cnt);
			return -1;
		}
		cnt++;
		if ((cnt % intval) == 0)
			printf("socket test: %3d %%\n", (cnt / intval) * 10);
	}
	printf("%d sockets in %lld usecs...\n", howmany, real_time_clock_usecs() - tnow);	
	printf("I have created %d sockets...\n", howmany);
	
	return 0;
}

void assign_addresses(void)
{
	void *sp; /* socket pointer... */
	int rv;
	struct ifreq ifr;
	
	printf("*************** assign socket addresses **************\n"
	       "Have you changed these to match your card and network?\n"
	       "******************************************************\n");

	rv = initsocket(&sp);
	if (rv < 0) {
		printf("Couldn't get a socket!\n");
		return;
	}
	
	rv = socreate(AF_INET, sp, SOCK_DGRAM, 0);
	if (rv < 0) {
		printf("Failed to create a socket to use...\n");
		return;
	}
	
	strcpy(ifr.ifr_name, "tulip0");
	
	memset(&ifr.ifr_addr, 0, sizeof(ifr.ifr_addr));
	
	rv = soo_ioctl(sp, SIOCGIFFLAGS, (caddr_t)&ifr);
	if (rv < 0) {
		printf("soo_ioctl gave %d\n", rv);
		return;
	}

	((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr = htonl(0xc0a80085);
	((struct sockaddr_in*)&ifr.ifr_addr)->sin_family = AF_INET;
	((struct sockaddr_in*)&ifr.ifr_addr)->sin_len = sizeof(struct sockaddr_in);
	
	rv = soo_ioctl(sp, SIOCSIFADDR, (caddr_t)&ifr);
	if (rv < 0) {
		printf("error %d [%s]\n", rv, strerror(rv));
		return;
	}
	
	strcpy(ifr.ifr_name,"loop0");
	((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr = htonl(0x7f000001);
	rv = soo_ioctl(sp, SIOCSIFADDR, (caddr_t)&ifr);
	if (rv < 0) {
		printf("error %d [%s]\n", rv, strerror(rv));
		return;
	}
	
	soclose(sp);

	printf("Addresses appear to be assigned correctly...let's check :\n");
}

#define TEST_DATA "Hello World"

static void err(int code, char *msg)
{
	printf("Error: %s: %d [%s]\n", msg, code, strerror(code));
}
		
static void sysctl_test(void)
{
	int mib[5];
	size_t needed;
	char *buf;
	caddr_t lim;
	
	printf ("sysctl test\n"
	        "===========\n");
	
	mib[0] = PF_ROUTE;
	mib[1] = 0;
	mib[2] = 0;
	mib[3] = NET_RT_DUMP;
	mib[4] = 0;
	if (net_sysctl(mib, 5, NULL, &needed, NULL, 0) < 0)	{
		perror("route-sysctl-estimate");
		exit(1);
	}

	printf("estimated to need %ld bytes\n", needed);

	if (needed > 0) {
		if ((buf = malloc(needed)) == 0) {
			printf("out of space\n");
			exit(1);
		}
		if (net_sysctl(mib, 5, buf, &needed, NULL, 0) < 0) {
			perror("sysctl of routing table");
			exit(1);
		}
		lim  = buf + needed;
	}
	printf("got data...\n");
}
	
static void tcp_test(uint32 srv)
{
	void *sp;
	struct sockaddr_in sa;
	int rv;
	char buffer[200];
	struct iovec iov;
	int flags = 0;
	
	printf("\n*** TCP Test ***\n\n");	
	rv = initsocket(&sp);
	if (rv < 0) {
		err(rv, "Couldn't get a 2nd socket!");
		return;
	}
	
	rv = socreate(AF_INET, sp, SOCK_STREAM, 0);
	if (rv < 0) {
		err(rv, "Failed to create a socket to use...");
		return;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons((real_time_clock() & 0xffff));
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_len = sizeof(sa);
	
	rv = sobind(sp, (caddr_t)&sa, sizeof(sa));
	if (rv < 0) {
		err(rv, "Failed to bind!\n");
		return;
	}

	sa.sin_addr.s_addr = htonl(srv);
	sa.sin_port = htons(80);
	
	rv = soconnect(sp, (caddr_t)&sa, sizeof(sa));
	if (rv < 0) {
		err(rv, "Connect failed!!");
		return;
	}
	snooze(500000);

	memset(&buffer, 0, 200);
	strcpy(buffer, "GET / HTTP/1.0\n\n");
	iov.iov_base = &buffer;
	iov.iov_len = 16;

	
	rv = writeit(sp, &iov, flags);
	if (rv < 0) {
		err(rv, "writeit failed!!");
		return;
	}
	printf("writeit wrote %d bytes\n\n", rv);
	
	memset(&buffer, 0, 200);
	iov.iov_len = 200;
	iov.iov_base = &buffer;
	while ((rv = readit(sp, &iov, &flags)) >= 0) {
		if (rv < 0) { 
			err (rv, "readit");
			soclose(sp);
			return;
		} else
			printf("%s", buffer);
		if (rv == 0)
			break;
		/* PITA - have to keep resetting these... */
		iov.iov_len = 200;
		iov.iov_base = &buffer;
		memset(&buffer, 0, 200);
	}
	
	soclose(sp);
	printf("\nTCP socket test completed...\n");
}

int main(int argc, char **argv)
{
	status_t status;
	ifnet *d;
	int qty = 1000;
	/* XXX - change this line to query a different web server! */
	uint32 web_server = 0xc0a80001;
	
	printf("Net Server Test App!\n"
	       "====================\n\n");

	start_stack();

	if (devices == NULL) {
		printf("\nFATAL: no devices configured!\n");
		exit (-1);
	}
	
	printf("\n");

	assign_addresses();
	list_devices();

	create_sockets((void*)&qty);
	tcp_test(web_server);
	sysctl_test();
	
	printf("\n\n Tests completed!\n");
	
	d = devices;
	while (d) {
		printf("device %s : rx_thread = %ld\n", d->if_name, d->rx_thread);
		if (d->rx_thread > 0  && d->if_type == IFT_ETHER) {
			printf("waiting on thread for %s%d\n", d->name, d->if_unit);
			wait_for_thread(d->rx_thread, &status);
		}
		d = d->if_next; 
	}

	return 0;
}

