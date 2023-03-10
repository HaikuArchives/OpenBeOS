/*
 * serial_ppp.c - PPP async serial device
 *
 */

#include <kernel/OS.h>
#include <stdio.h>
#include <malloc.h>
#include <kernel/OS.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h> 
#include <errno.h>

//#include "sys/socket.h"
//#include "protocols.h"
//#include "netinet/in.h"
//#include "netinet/ip.h"
//#include "sys/socketvar.h"
//#include "sys/protosw.h"
//#include "sys/domain.h"
#include "sys/sockio.h"
#include "net/ppp_defs.h"
#include "net/if_types.h"

#include "net_malloc.h"
#include "core_module.h"
#include "net_module.h"
#include "core_funcs.h"
#include "../fsm.h"
#include "ppp/ppp_module.h"
#include "ppp/ppp_device_module.h"

#include "../ppp_device.h"
#include "serial_ppp.h"

#include "../ppp_funcs.h"

#ifdef _KERNEL_
#include <KernelExport.h>
#define spawn_thread spawn_kernel_thread
static status_t sppp_ops(int32 op, ...);
#define SPPP_MODULE_PATH "network/ppp/devices/serial_ppp"
#else
#define sppp_ops NULL
#define SPPP_MODULE_PATH "ppp/devices/serial_ppp"
static image_id pppid = -1;
#endif

static struct core_module_info *core = NULL;
static struct ppp_module_info *ppp = NULL;
static int serial_dev = 0;

#define SERIAL_PORT_PATH "/dev/ports"
#define PPP_ALLSTATIONS 0xff     /* All-Stations broadcast address */
#define PPP_UI          0x03     /* Unnumbered Information */
#define PPP_FLAG        0x7e     /* Flag Sequence */
#define PPP_ESCAPE      0x7d     /* Asynchronous Control Escape */
#define PPP_TRANS       0x20     /* Asynchronous transparency modifier */

#define MAX_DUMP_BYTES	128

static char *matches[] = {
	"serial",
	"/dev/ports/serial",
};

static void add_matches(struct ppp_dev_match **existing, 
                        struct ppp_device_module_info *ptr)
{
	struct ppp_dev_match *p, *m;
	int i;
	
	if ((p = *existing) != NULL)
		for (; p->next ; p = p->next)
			continue;
		
	for (i=0; i < sizeof(matches) / sizeof(*matches);i++) {
		m = (struct ppp_dev_match*)malloc(sizeof(*m));
		if (!m)
			return;
		memset(m, 0, sizeof(*m));
		m->mod = ptr;
		m->match = matches[i];
		m->mlen = strlen(matches[i]);
		
		if (p) {
			p->next = m;
			p = p->next;
		} else {
			*existing = m;
			p = m;
		}
	}
}

static void pppdumpm(struct mbuf *m0)
{
    char buf[3*MAX_DUMP_BYTES+4];
    char *bp = buf;
    struct mbuf *m;
    static char digits[] = "0123456789abcdef";

    for (m = m0; m; m = m->m_next) {
		int l = m->m_len;
		u_char *rptr = (u_char *)m->m_data;

		while (l--) {
	    	if (bp > buf + sizeof(buf) - 4)
				goto done;
	    	*bp++ = digits[*rptr >> 4]; /* convert byte to ascii hex */
	    	*bp++ = digits[*rptr++ & 0xf];
		}

		if (m->m_next) {
		    if (bp > buf + sizeof(buf) - 3)
				goto done;
	    	*bp++ = '|';
		} else
	    	*bp++ = ' ';
   	}
done:
    if (m)
	*bp++ = '>';
    *bp = 0;
    printf("%s\n", buf);
}

static void pppdumpb(char *ptr, int len)
{
    char buf[3*MAX_DUMP_BYTES+4];
    char *bp = buf;
	char *rptr = ptr;
    static char digits[] = "0123456789abcdef";

	while (len--) {
	    if (bp > buf + sizeof(buf) - 4)
			goto done;
    	*bp++ = digits[*rptr >> 4]; /* convert byte to ascii hex */
    	*bp++ = digits[*rptr++ & 0xf];
	}

done:
    *bp = 0;
    printf("%s\n", buf);
}

static int32 sppp_write(void *data)
{
	struct mbuf *m = NULL;
	struct mbuf *m0 = NULL;
	struct serial_ppp_device *dev = (struct serial_ppp_device *)data;
	struct ifnet *ifp = &dev->ifp;
	char buffer[ifp->if_mtu];
	int adds = 0;
	char *bp, *cp;
	uint16 fcs = 0;
	int len, i, rv;
	
	if (!ifp->txq) {
		printf("%s: no transmission q created!\n", ifp->if_name);
		return -1;
	}
	
	while (1) {
		acquire_sem_etc(ifp->txq->pop, 1, B_CAN_INTERRUPT | B_DO_NOT_RESCHEDULE, 0);
		IFQ_DEQUEUE(ifp->txq, m);
		if (m) {
			memset(&buffer, 0, ifp->if_mtu);
			bp = &buffer[0];
			
			for (m0 = m ; m0->m_next ; m0 = m0->m_next) {
				continue;
			}
			
			if (M_TRAILINGSPACE(m0) > 2) {
				m0->m_len += 2;
				if (m0->m_flags & M_PKTHDR)
					m0->m_pkthdr.len += 2;
			}
			/* Prepend space for our framing headers (trailer we don't worry about) */
			m = m_prepend(m, 2);
			if (!m)
				printf("sppp_write: m_prepend failed!\n");
			len = 0;
			for (m0 = m; m0 ; m0 = m0->m_next) {
				len += m0->m_len;
			}

			/* make sure we have a contiguous set of data */
			m = m_pullup(m, len);
			if (!m) {
				printf("spp_write: m_pullup failed!\n");
				continue;
			}
			
			printf(">>> ");
			pppdumpm(m);

			cp = mtod(m, char*);
			*cp++ = 0xff;
			*cp-- = 0x03;
			
			len = m->m_len;

			fcs = 0xffff;
			fcs = pppfcs16(fcs, cp, len - 2);
			fcs ^= 0xffff;
			*(cp + len - 2) = (fcs & 0x00ff);
			*(cp + len - 1) = ((fcs >> 8) & 0x00ff);

			*bp++ = 0x7e;			
			adds = 1;
			for (i = 0; i < len ; i++) {
				if ((uint8)*cp < 0x20 || *cp == 0x7e || *cp == 0x7d) {
					adds++;
					*bp++ = 0x7d;
					*cp ^= PPP_TRANS;
				}
				*bp++ = *cp++;
			}
			len += adds;
			/* Add fcs and trailer byte */
			buffer[len++] = 0x7e;
			rv = write(ifp->devid, &buffer, len);
			m_freem(m);
			printf("sppp_write: write gave %d\n", rv);
		}
	}
	return 0;
}		
		
static int32 sppp_read(void *data)
{
	struct mbuf *m = NULL;
	struct serial_ppp_device *dev = (struct serial_ppp_device *)data;
	struct ifnet *ifp = &dev->ifp;
	uchar *bytes;
	char rxb[32];
	char tbuff[ifp->if_mtu];
	int rv, complete = 0, i, pktlen = 0, esc_req = 0;
	uint16 fcs;
	struct ifq *pppq;
	
	if (!dev->ppp) {
		printf("%s: not yet attached to a PPP device!\n", ifp->if_name);
		return -1;
	}
	pppq = dev->ppp->sc_if.rxq;
	
	
	while (1) {
recycle:
		pktlen = 0;
		esc_req = 0;
		memset((void*)tbuff, 0, ifp->if_mtu);
		bytes = (uchar*)&tbuff[0];
		while (1) {
			/* inefficient, but this seems to be how the data arrives anyway
			 */
			memset(&rxb, 0, 32);
			do {
				rv = read(ifp->devid, rxb, 32);
			} while (rv == 0);
			/* we should handle read/write that isn't PPP here :) */
			
			if (rv < 0)
				break;

			for (i = 0 ; i < rv; i++) {
				if (esc_req == 1) {
					rxb[i] ^= PPP_TRANS;
					esc_req = 0;
				} else if (rxb[i] == PPP_ESCAPE) {
					if ((rv - i++) == 1) {
						/* we're the last character... set esc_req = 1 */
						esc_req = 1;
						break;
					} else
						rxb[i] ^= PPP_TRANS;					
				} else {
					/* flag can't be escaped... */
					if (rxb[i] == PPP_FLAG) {
						if (bytes != (uchar*)&tbuff[0])
							complete = 1;
					}
				}
				*bytes++ = rxb[i];
				pktlen++;
				if (complete)
					break;
			}
			if (complete)
				break;
		}
		complete = 0;
		/* if incomplete packet, junk it and try again */
		/* ??? not sure if this is correct...think we should maybe wait for more */
		if (*(bytes - 1) != PPP_FLAG) {
			goto recycle;
		}
//		printf("sppp_read: got ppp packet of %d bytes\n", pktlen);
		pktlen -= 2; /* decrease length by header & trailer */
		fcs = pppfcs16(PPP_INITFCS, (uchar*)&tbuff[1], pktlen);
		if (fcs != PPP_GOODFCS) {
			printf("sppp_read: FCS failed!\nFailed packet was :");
			pppdumpb(&tbuff[1], pktlen);
			goto recycle;
		}
		/* pkt -4 as we don't want the address, UI or fcs to be copied over */
		m = NULL;
		m = m_devget(&tbuff[3], pktlen - 4, 0, ifp, NULL);
		if (m) {
			/* we have a valid packet... */
			printf("<<< ");
			pppdumpm(m);
			IFQ_ENQUEUE(pppq, m);
		}
		complete = 0;
	}
	return 0;
}

static int32 connect_thread(void *data)
{
	struct serial_ppp_device *dev = (struct serial_ppp_device *)data;
	struct ifnet *ifp = &dev->ifp;
	int rv;
	struct termios options;
	char path[PATH_MAX];
	char thd_name[B_OS_NAME_LENGTH];
	
	sprintf(path, "%s/%s", SERIAL_PORT_PATH, ifp->if_name);	
	/* sit in a blocking open until we have a connection... */
	ifp->devid = open(path, O_RDWR);
	if (ifp->devid < 0) {
		printf("serial_ppp: failed to open device %s\n", path);
		printf("serial_ppp: error was %d [%s]\n", errno, strerror(errno));
		return -1;
	}
	printf("serial_ppp: connection (%d)!!!\n", ifp->devid);
	
	ifp->if_flags |= IFF_UP;
	
	/* OK, set up the port to use */
	ioctl(ifp->devid, TCGETA, (char *)&options);

	options.c_cflag &= ~CBAUD;
	options.c_cflag |= B19200;
	
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;
	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 10;
	
	rv = ioctl(ifp->devid, TCSETA, &options);
	if (rv != 0) {
		printf("async serial: tcsetattr gave an error!\n");
		return(-1);
	}
	
	printf("async serial: port setup completed\n");
	
	/* very simple, we'll spawn a read and write thread */
	sprintf(thd_name, "%s_write_thread", ifp->if_name);
	ifp->txq = start_ifq();
	ifp->tx_thread = spawn_thread(sppp_write, thd_name, 
	                             B_NORMAL_PRIORITY, ifp);
	if (ifp->tx_thread > 0) {
		resume_thread(ifp->tx_thread);
	}

	/* This is here as the first thing it will do is try to send a
	 * Config Request */
printf("ppp = %p\n",ppp);
	if (ppp) {
		fsm_Open(dev->fsm);
		fsm_Up(dev->fsm);
	}

	sprintf(thd_name, "%s_read_thread", ifp->if_name);
	ifp->rx_thread = spawn_thread(sppp_read, thd_name, 
	                             B_NORMAL_PRIORITY, ifp);
	if (ifp->rx_thread > 0) {
		resume_thread(ifp->rx_thread);
		ifp->if_flags |= IFF_RUNNING;
	}
	
	return 0;
}

int sppp_output(struct ifnet *ifp, struct mbuf *m, struct sockaddr *sa,
			struct rtentry *rt)
{
	return 0;
}
					
static int sppp_dev_stop(struct ifnet *dev)
{
	dev->if_flags &= ~IFF_UP;

	if (dev->rx_thread > 0)
		kill_thread(dev->rx_thread);
	if (dev->tx_thread > 0)
		kill_thread(dev->tx_thread);

	close(dev->devid);
	
	dev->if_flags &= ~IFF_RUNNING;
	
	return 0;
}

static int sppp_ioctl(struct ifnet *ifp, int cmd, caddr_t data)
{
	int error = 0;

	switch(cmd) {
		case SIOCSIFADDR:
			ifp->if_flags |= (IFF_UP | IFF_RUNNING);
			if (ifp->rx_thread < 0)
				start_rx_thread(ifp);
			if (ifp->tx_thread < 0)
				start_tx_thread(ifp);
			break;
		default:
			error = EINVAL;
	}
	return error;
}



static struct ifnet *sppp_create(char *data, int dlen)
{
	struct serial_ppp_device *dev = (struct serial_ppp_device *)
	                                malloc(sizeof(*dev));
	if (!dev)
		return NULL;
	if (dlen < 1) {
		free(dev);
		return NULL;
	}

	memset(dev, 0, sizeof(*dev));
	dev->ifp.devid = -1;
	dev->ifp.name = "serial";
	dev->ifp.if_unit = serial_dev++;
	dev->ifp.if_type = IFT_RS232; /* not exactly... */
	dev->ifp.rx_thread = -1;
	dev->ifp.tx_thread = -1;
	dev->ifp.if_addrlen = 0;
	dev->ifp.if_hdrlen = 0;
	dev->ifp.if_flags = IFF_POINTOPOINT;
	dev->ifp.if_mtu = 1500;
	dev->ifp.stop = &sppp_dev_stop;
	dev->ifp.ioctl = &sppp_ioctl;

	/* This should be safer than strdup as apparently it
	 * can be nasty in some cases.
	 */
	dev->path = (char*)malloc(strlen(data));
	memcpy(dev->path, data, strlen(data));
	
	if_attach(&dev->ifp);

	return &dev->ifp;	
}

static int sppp_start(struct ifnet *ifp)
{
	struct serial_ppp_device *dev = (struct serial_ppp_device*)ifp;
	char thd_name[B_OS_NAME_LENGTH];
	
	/* If we're here it's because the following are true...
	 *
	 * we're being told the port is ready
	 * the port is setup correctly
	 * the port is ready (and waiting) for ppp
	 */

	/* Open as non-blocking so we just open */
	ifp->devid = open(dev->path, O_RDWR | O_NONBLOCK);
 	if (ifp->devid < 0) {
 		printf("serial_ppp: Unable to open %s\n", dev->path);
 		return -1;
 	}
 	/* As we appear to have opened OK, turn on blocking again */
 	fcntl(ifp->devid, F_SETFL, 0);
	ifp->if_flags |= IFF_UP;
 		
	/* very simple, we'll spawn a write thread */
	sprintf(thd_name, "%s_write_thread", ifp->if_name);
	ifp->txq = start_ifq();
	ifp->tx_thread = spawn_thread(sppp_write, thd_name, 
	                             B_NORMAL_PRIORITY, ifp);
	if (ifp->tx_thread > 0) {
		resume_thread(ifp->tx_thread);
	}

	/* This is here as the first thing it will do is try to send a
	 * Config Request.
	 * Hopefully LCP was in Closed state so this will bring it to the
	 * ReqSent state and cause a ConfigRequest to be sent, thus starting off
	 * the LCP negotiation.
	 */
	if (ppp)
		fsm_Open(dev->fsm);

	/* now we spawn a read thread */
	sprintf(thd_name, "%s_read_thread", ifp->if_name);
	ifp->rx_thread = spawn_thread(sppp_read, thd_name, 
	                             B_NORMAL_PRIORITY, ifp);
	if (ifp->rx_thread > 0) {
		resume_thread(ifp->rx_thread);
		ifp->if_flags |= IFF_RUNNING;
	}
	
	return 0;
}	

static void sppp_attach(struct ifnet *ifp, struct ppp_softc *ppp)
{
	struct serial_ppp_device *dev = (struct serial_ppp_device*)ifp;
	dev->ppp = ppp;
}

static void sppp_attach_fsm(struct ifnet *ifp, struct fsm *fsm)
{
	struct serial_ppp_device *dev = (struct serial_ppp_device*)ifp;
	dev->fsm = fsm;
	/* as we're attached...signal an Up event to move the LCP to
	 * the Closed state
	 */
	if (ppp)
		fsm_Up(dev->fsm);
	/* If we get an Open event from the user the LCP will move to ReqSent and
	 * start things off...
	 */
}
	
/* remove a device */
int sppp_remove(struct ifnet *ifp)
{
	printf("sppp_remove\n");
	if_detach(ifp);
	return 0;
}
	
#ifdef USER
static void set_core(struct core_module_info *cp)
{
	if (!core)
		core = cp;
}

static void set_ppp(struct ppp_module_info *cp)
{
	if (!ppp)
		ppp = cp;
}

#endif

static int sppp_module_init(void *cpp)
{
	if (cpp)
		core = cpp;

printf("sppp_module_init\n");
#ifndef _KERNEL_
	if (!ppp) {
		char path[PATH_MAX];
		getcwd(path, PATH_MAX);
		strcat(path, "/" PPP_MODULE_PATH);

		pppid = load_add_on(path);
		if (pppid > 0) {
			status_t rv = get_image_symbol(pppid, "protocol_info",
								B_SYMBOL_TYPE_DATA, (void**)&ppp);
			if (rv < 0) {
				printf("Failed to get access to PPP information!\n");
				return -1;
			}
		} else { 
			printf("Failed to load the PPP module...\n");
			return -1;
		}
		ppp->set_core(cpp);
	}
#else
	if (!ppp)
		get_module(PPP_MODULE_PATH, (module_info**)&ppp);
#endif
	if (!ppp)
		printf("Failed to get the PPP module pointer!\n");
	
	return 0;
}

_EXPORT struct ppp_device_module_info ppp_device = {
	"Serial PPP device",
#ifdef USER
	set_core,
	set_ppp,
#endif
	add_matches,
	sppp_create,
	sppp_attach,
	sppp_attach_fsm,
	sppp_start,
	NULL,                /* sppp_stop */
	sppp_remove
};

_EXPORT struct kernel_net_module_info device_info = {
	{
		SPPP_MODULE_PATH,
		0,
		sppp_ops
	},
	sppp_module_init,
	NULL,
};

#ifdef _KERNEL_
static status_t sppp_ops(int32 op, ...)
{
	switch(op) {
		case B_MODULE_INIT:
			get_module(CORE_MODULE_PATH, (module_info **)&core);
			if (!core)
				return B_ERROR;
			return B_OK;
		case B_MODULE_UNINIT:
			break;
		default:
			return B_ERROR;
	}
	return B_OK;
}

_EXPORT module_info *modules[] = {
	(module_info*) &device_info,
	NULL
};

#endif


