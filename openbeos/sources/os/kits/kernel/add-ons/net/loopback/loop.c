/* loop.c - loopback device 
 */

#include <stdio.h>
#include <memheap.h>
#include <ktypes.h>
#include <module.h>
#include <Errors.h>
#include <debug.h>

/*#include "sys/socket.h"
#include "protocols.h"
#include "netinet/in.h"
#include "netinet/ip.h"
#include "sys/socketvar.h"
#include "sys/protosw.h"
#include "sys/domain.h"
#include "sys/sockio.h"

#include "net_malloc.h"
#include "core_module.h"
#include "net_module.h"
#include "core_funcs.h"

#ifdef _KERNEL_MODE
#include <KernelExport.h>
#include <module.h>

#include "net_device.h"
#include "loop_module.h"

#endif
*/

#define LOOP_MODULE_PATH "network/interface/loop"

/*
static struct core_module_info *core = NULL;

static struct protosw *proto[IPPROTO_MAX];
static struct ifnet *me = NULL;

int loop_output(struct ifnet *ifp, struct mbuf *m, struct sockaddr *sa,
			struct rtentry *rt)
{
	struct ip *ip = mtod(m, struct ip *);

	ip->ip_dst = ip->ip_src;
	ip->ip_src.s_addr = INADDR_LOOPBACK;

	IFQ_ENQUEUE(ifp->rxq, m);
	return 0;
}

void loop_input(struct mbuf *buf)
{
	if (!buf)
		return;

	buf->m_pkthdr.rcvif = me;
	
	if (proto[IPPROTO_IP] && proto[IPPROTO_IP]->pr_input)
		return proto[IPPROTO_IP]->pr_input(buf, 0);
	else
		printf("No input tourtine found for IP\n");

	m_freem(buf);
	return;
}

static int loop_dev_stop(struct ifnet *dev)
{
	if (!dev || dev->if_type != IFT_LOOP)
		return EINVAL;

	dev->if_flags &= ~IFF_UP;

	if (dev->rx_thread > 0)
		kill_thread(dev->rx_thread);
	if (dev->tx_thread > 0)
		kill_thread(dev->tx_thread);

	dev->if_flags &= ~IFF_RUNNING;
	
	return 0;
}

static int loop_ioctl(struct ifnet *ifp, int cmd, caddr_t data)
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
*/	
static int loop_init(void)
{
dprintf("LOOPBACK: init!\n");
/*	me = (struct ifnet*)malloc(sizeof(struct ifnet));
	if (!me)
		return -1;
		
	memset(me, 0, sizeof(*me));

	memset(proto, 0, sizeof(struct protosw *) * IPPROTO_MAX);

	me->devid = -1;
	me->name = "loop";
	me->if_unit = 0;
	me->if_type = IFT_LOOP;
	me->rx_thread = -1;
	me->tx_thread = -1;
	me->if_addrlen = 0;
	me->if_hdrlen = 0;
	me->if_flags = IFF_LOOPBACK | IFF_MULTICAST;
	me->if_mtu = 16384;
	me->input = &loop_input;
	me->output = &loop_output;
	me->stop = &loop_dev_stop;
	me->ioctl = &loop_ioctl;
	
	add_protosw(proto, NET_LAYER1);
	if_attach(me);
*/
	return 0;
}

static status_t loop_ops(int32 op, ...)
{
	switch(op) {
		case B_MODULE_INIT:
			break;
		case B_MODULE_UNINIT:
			break;
		default:
			return B_ERROR;
	}
	return 0;
}

/* XXX - temp hack */
struct device_mi {
	module_info info;
	int (*init)(void);
};

static struct device_mi my_module = {
	{
		LOOP_MODULE_PATH,
		0,
		loop_ops
	},

	loop_init
};

module_info *modules[] = {
	(module_info*) &my_module,
	NULL
};
