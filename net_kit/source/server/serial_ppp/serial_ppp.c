/* serial_ppp.c - async serial device
 *
 * This is intended to be used for PPP testing purposes only 
 */

#include <kernel/OS.h>
#include <stdio.h>
#include <malloc.h>
#include <kernel/OS.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "sys/socket.h"
#include "protocols.h"
#include "netinet/in.h"
#include "netinet/ip.h"
#include "sys/socketvar.h"
#include "sys/protosw.h"
#include "sys/domain.h"
#include "sys/sockio.h"
#include "net/ppp_defs.h"
#include "net/if_types.h"

#include "net_malloc.h"
#include "core_module.h"
#include "net_module.h"
#include "core_funcs.h"
#include "ppp/ppp_module.h"

#ifdef _KERNEL_
#include <KernelExport.h>
#define spawn_thread spawn_kernel_thread
static status_t sppp_ops(int32 op, ...);
#define ASERIAL_MODULE_PATH "network/interface/serial_ppp"
#else
#define sppp_ops NULL
#define ASERIAL_MODULE_PATH "interface/serial_ppp"
static image_id pppid = -1;
#endif

static struct core_module_info *core = NULL;
static struct ppp_module_info *ppp = NULL;
static struct protosw *proto[IPPROTO_MAX];
static struct ifnet *me = NULL, *pppif = NULL;
static struct ifq *pppq = NULL;
static struct ifq *ppptq = NULL;
static thread_id my_thread;

#define SERIAL_DEV "/dev/ports/serial2"

#define MAX_DUMP_BYTES	128

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
	struct ifnet *ifp = (struct ifnet *)data;
	char buffer[ifp->if_mtu];
	int adds = 0;
	char *bp, *cp;
	uint16 fcs = 0;
	int len, i, rv;
	
	while (1) {
		acquire_sem_etc(ppptq->pop, 1, B_CAN_INTERRUPT | B_DO_NOT_RESCHEDULE, 0);
		IFQ_DEQUEUE(ppptq, m);
		if (m) {
			memset(&buffer, 0, ifp->if_mtu);
			bp = &buffer[0];
			
			for (m0 = m ; m0->m_next ; m0 = m0->m_next)
				continue;
			if (M_TRAILINGSPACE(m0) > 2) {
				m0->m_len += 2;
				if (m0->m_flags & M_PKTHDR)
					m0->m_pkthdr.len += 2;
			}
			/* Prepend space for our framing headers (trailer we don't worry about) */
			m = m_prepend(m, 2);
			len = 0;
			for (m0 = m; m0 ; m0 = m0->m_next)
				len += m0->m_len;

			/* make sure we have a contiguous set of data */
			m = m_pullup(m, len);
			if (!m) {
				printf("spp_write: m_pullup failed!\n");
				continue;
			}
			
//			pppdumpm(m);
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
//			printf("sppp_write: write gave %d\n", rv);
		}
	}
	return 0;
}		
		
static int32 sppp_read(void *data)
{
	struct mbuf *m = NULL;
	struct ifnet *ifp = (struct ifnet *)data;
	uchar *bytes;
	char rxb[32];
	char tbuff[ifp->if_mtu];
	int rv, complete = 0, i, pktlen = 0, esc_req = 0;
	uint16 fcs;
	
	while (1) {
		m = m_gethdr(MT_HEADER);
		if (!m)
			break;
		m_reserve(m, 5);
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
			printf("FCS failed!\n");
			pppdumpb(&tbuff[1], pktlen);
			goto recycle;
		}
		/* pkt -4 as we don't want the address, UI or fcs to be copied over */
		m = m_devget(&tbuff[3], pktlen - 4, 0, ifp, NULL);

		/* we have a valid packet... */
//		pppdumpm(m);
		IFQ_ENQUEUE(pppq, m);
		complete = 0;
	}
	return 0;
}

static int32 connect_thread(void *data)
{
	int rv;
	struct termios options;
	status_t status;
	
	/* sit in a blocking open until we have a connection... */
	me->devid = open(SERIAL_DEV, O_RDWR);
	if (me->devid < 0) {
		printf("async serial: failed to open device %s\n", SERIAL_DEV);
		return -1;
	}
	printf("async serial: connection (%d)!!!\n", me->devid);
	
	me->if_flags |= IFF_UP;
	
	/* OK, set up the port to use */
	ioctl(me->devid, TCGETA, (char *)&options);

	options.c_cflag &= ~CBAUD;
	options.c_cflag |= B19200;
	
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;
	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 10;
	
	rv = ioctl(me->devid, TCSETA, &options);
	if (rv != 0) {
		printf("async serial: tcsetattr gave an error!\n");
		return(-1);
	}
	
	if (ppp)
		pppif = ppp->connection();
	if (pppif) {
		pppq = pppif->rxq;
		ppptq = pppif->txq;
	}
	
	printf("async serial: port setup completed\n");
	
	/* very simple, we'll spawn a read and write thread */
	me->rx_thread = spawn_thread(sppp_read, "sppp_read", 
	                             B_NORMAL_PRIORITY, me);
	if (me->rx_thread > 0) {
		resume_thread(me->rx_thread);
		me->if_flags |= IFF_RUNNING;
	}
	me->tx_thread = spawn_thread(sppp_write, "sppp_write", 
	                             B_NORMAL_PRIORITY, me);
	if (me->tx_thread > 0) {
		resume_thread(me->tx_thread);
	}
	
	wait_for_thread(me->rx_thread, &status);	
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
	
static int sppp_init(void)
{
	me = (struct ifnet*)malloc(sizeof(struct ifnet));
	if (!me)
		return -1;
		
	memset(me, 0, sizeof(*me));
	memset(proto, 0, sizeof(struct protosw *) * IPPROTO_MAX);

	me->devid = -1;
	me->name = "sppp";
	me->if_unit = 0;
	me->if_type = IFT_RS232; /* not exactly... */
	me->rx_thread = -1;
	me->tx_thread = -1;
	me->if_addrlen = 0;
	me->if_hdrlen = 0;
	me->if_flags = IFF_POINTOPOINT;
	me->if_mtu = 1500;
	me->stop = &sppp_dev_stop;
	me->ioctl = &sppp_ioctl;
	
	add_protosw(proto, NET_LAYER1);
	if_attach(me);

	my_thread = spawn_thread(connect_thread, "sppp_connect_thread",
	                         B_NORMAL_PRIORITY, NULL);
	if (my_thread > 0)
		resume_thread(my_thread);

	return 0;
}

static int sppp_module_init(void *cpp)
{
	if (cpp)
		core = cpp;

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
	
	sppp_init();
	
	return 0;
}

_EXPORT struct kernel_net_module_info device_info = {
	{
		ASERIAL_MODULE_PATH,
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


