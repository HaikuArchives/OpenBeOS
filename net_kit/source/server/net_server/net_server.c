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

struct ifnet *devices = NULL;
struct ifnet *pdevices = NULL;		/* pseudo devices - loopback etc */
int ndevs = 0;
sem_id dev_lock;

static int32 if_thread(void *data)
{
	ifnet *i = (ifnet *)data;
	status_t status;
	char buffer[i->if_mtu];
	size_t len = i->if_mtu;
	int count = 0;

	while ((status = read(i->devid, buffer, len)) >= B_OK) {
		struct mbuf *mb = m_devget(buffer, status, 0, i, NULL);
		/* Hmm, not sure about this... */
		if ((i->if_flags & IFF_UP) == 0) {
			m_freem(mb);
			continue;
		}
		
		if (i->input)
			i->input(mb);
		atomic_add(&i->if_ipackets, 1);
		count++;
		len = i->if_mtu;
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

		if ((i->if_flags & IFF_UP) == 0) {
			m_freem(m);
			continue;
		}
		
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

		if (len > i->if_mtu) {
			printf("%s%d: tx_thread: packet was too big!\n", i->name, i->unit);
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

	sprintf(dname, "%s%d", ifn->name, ifn->unit);
	printf("adding device %s\n", dname);
	ifn->if_name = strdup(dname);

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
	printf("add_device: added %s\n", ifn->if_name);	
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
	dev->tx_thread = spawn_thread(tx_thread, "net_tx_thread", priority, dev);
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
		printf("%2d  %s%d       %4ld ", i++, d->name, d->unit, d->if_mtu);
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
			printf("\t\t Addresses:\n");
			while (ifa) {
				dump_sockaddr(ifa->ifa_addr);
				printf("\n");
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
#if SHOW_MALLOC_USAGE
	printf("core.c: add_domain: malloc(%ld)\n", sizeof(*ndm));
#endif
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
	
	printf("add_protocol: %s\n", pr->name);
	
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

int start_stack(void)
{
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
	
	printf("*** assign sockets test! ***\n"
	       "Have you chnaged these to match your card and network?\n");

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

static void big_socket_test(void)
{
	void *sp[100];
	int i, rv;
	int qty = 10;
	struct sockaddr_in sa;
	int32 nsocks = 0;
	
	sa.sin_family = AF_INET;
	sa.sin_port = 0;
	sa.sin_addr.s_addr = INADDR_LOOPBACK;
	sa.sin_len = sizeof(sa);
	memset(&sa.sin_zero, 0, sizeof(sa.sin_zero));
	
	printf("\n*** big socket test (stupid name!) ***\n");
	while (qty < 100) {
		for (i=0;i<qty;i++) {
			rv = initsocket(&sp[i]);
			if (rv < 0) {
				err(rv, "Failed to create a socket");
				break;
			}	
			rv = socreate(AF_INET, sp[i], SOCK_DGRAM, 0);
			if (rv < 0) {
				err(rv, "Failed to create #2 a socket");
				break;
			}
		}
		for (i=0;i < qty;i++) {
			rv = sobind(sp[i], (caddr_t)&sa, sizeof(sa));
			if (rv < 0) {
				err(rv, "Failed to bind!");
				break;
			}
		}	
		for (i=0;i<qty;i++) {
			soclose(sp[i]);
			nsocks++;
		}
		qty += 10;
	}
	printf("Big socket test completed: %ld sockets created/bound and then closed\n",
	       nsocks);
}
			
static void bind_test(void)
{
	void *sp = NULL, *sq = NULL; /* socket pointer... */
	int rv, rw;
	struct sockaddr_in sa;
	struct msghdr mh;
	struct iovec iov;
	char msg[20];

	printf("\n*** bind test (UDP sockets) ***\n\n");		
	rv = initsocket(&sp);
	if (rv < 0) {
		err(rv, "Couldn't get a socket!");
		return;
	}
	rv = initsocket(&sq);
	if (rv < 0) {
		err(rv, "Couldn't get a 2nd socket!");
		return;
	}
	
	rv = socreate(AF_INET, sp, SOCK_DGRAM, 0);
	if (rv < 0) {
		err(rv, "Failed to create a socket to use...");
		return;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(7772);
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_len = sizeof(sa);
	
	rv = sobind(sp, (caddr_t)&sa, sizeof(sa));
	if (rv < 0) {
		err(rv, "Failed to bind!\n");
	}

	rv = socreate(AF_INET, sq, SOCK_DGRAM, 0);
	if (rv < 0) {
		err(rv, "Failed to create a socket to use...");
		return;
	}
	sa.sin_addr.s_addr = INADDR_LOOPBACK;
	sa.sin_port = htons(7773);
	
	rv = sobind(sq, (caddr_t)&sa, sizeof(sa));
	if (rv < 0) {
		err(rv, "Failed to bind!");
		return;
	}
	memcpy(msg, TEST_DATA, strlen(TEST_DATA));
	
	mh.msg_name = (caddr_t)&sa;
	mh.msg_namelen = sizeof(sa);
	mh.msg_flags = 0;
	mh.msg_control = NULL;
	mh.msg_controllen = 0;
	iov.iov_base = &msg;
	iov.iov_len = strlen(TEST_DATA);
	mh.msg_iov = &iov;
	mh.msg_iovlen = 1;
	sa.sin_len = sizeof(sa);
	
	printf("Trying to send %ld bytes data\n", strlen(TEST_DATA));
	
	rv = sendit(sp, &mh, 0, &rw);
	if (rv < 0) {
		printf("Failed to send data!\n");
		goto out;
	}
		
	iov.iov_base = &msg;
	iov.iov_len = 20;
	mh.msg_namelen = sizeof(sa);
	mh.msg_name = (caddr_t)&sa;
	memset(&msg, 0, 20);
	memset(&sa, 0, sizeof(sa));
	sa.sin_len = sizeof(sa);
	
	rv = recvit(sq, &mh, (caddr_t)&mh.msg_namelen, &rw);
	printf("recvit gave %d, rw = %d\n", rv, rw);
	if (rw > 0) {
		printf("got %d bytes\n[%s]\n", rw, msg);
		printf("Came from %08lx:%d\n", ntohl(sa.sin_addr.s_addr), 
			ntohs(sa.sin_port));
	}
out:
	soclose(sp);
	soclose(sq);
}

int main(int argc, char **argv)
{
	status_t status;
	ifnet *d;
	thread_id t;
	int qty = 15000;

	printf("Net Server Test App!\n"
	       "====================\n\n");

	start_stack();

	if (net_init_timer() < B_OK)
		printf("timer service won't work!\n");

	if (devices == NULL) {
		printf("\nFATAL: no devices configured!\n");
		exit (-1);
	}

	assign_addresses();
	list_devices();

	t = spawn_thread(create_sockets, "socket creation test",
		B_NORMAL_PRIORITY, &qty);
	if (t >= 0)
		resume_thread(t);

	/* try to get better output! */
	wait_for_thread(t, &status);
	
	bind_test();
	big_socket_test();
	
	printf("\n\n Tests completed!\n");
	
	d = devices;
	while (d) {
		printf("device %s : rx_thread = %ld\n", d->if_name, d->rx_thread);
		if (d->rx_thread > 0  && d->if_type == IFT_ETHER) {
			printf("waiting on thread for %s%d\n", d->name, d->unit);
			wait_for_thread(d->rx_thread, &status);
		}
		d = d->if_next; 
	}

	return 0;
}

