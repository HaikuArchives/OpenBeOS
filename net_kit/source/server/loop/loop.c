/* loop.c - loopback device 
 */

#include <stdio.h>
#include <malloc.h>

#ifdef _KERNEL_MODE
#include <module.h>
#include "net_server/core_module.h"
#include "net_device.h"
#include "loop_module.h"

struct core_module_info *core = NULL;
#endif

#include "net_module.h"
#include "sys/socket.h"
#include "protocols.h"
#include "netinet/in.h"
#include "ipv4/ipv4.h"
#include "sys/protosw.h"
#include "sys/domain.h"

#ifdef USE_DEBUG_MALLOC
#define malloc dbg_malloc
#define free   dbg_free
#endif

static struct protosw *proto[IPPROTO_MAX];

int loop_output(ifnet *ifp, struct mbuf *m, struct sockaddr *sa,
			struct rtentry *rt)
{
	/* turn it straight back... */
	/* This is lame as we should be detecting the protocol, but it gets
	 * us working.
	 * XXX - fix me!
	 */
	ipv4_header *ip = mtod(m, ipv4_header *);

	ip->dst = ip->src;
	ip->src.s_addr = INADDR_LOOPBACK;

	IFQ_ENQUEUE(ifp->rxq, m);
}

int loop_input(struct mbuf *buf)
{
	int error = 0;

	if (proto[IPPROTO_IP] && proto[IPPROTO_IP]->pr_input)
		error = proto[IPPROTO_IP]->pr_input(buf, 0);
	else
		printf("No input tourtine found for IP\n");

#ifndef _KERNEL_MODE
	m_freem(buf);
#else
	core->m_freem(buf);
#endif

	return error;
}

static int loop_dev_stop(ifnet *dev)
{
	if (!dev || dev->if_type != IFT_LOOP)
		return EINVAL;

	dev->flags &= ~IFF_UP;

	if (dev->rx_thread > 0)
		kill_thread(dev->rx_thread);
	if (dev->tx_thread > 0)
		kill_thread(dev->tx_thread);

	dev->flags &= ~IFF_RUNNING;
	
	return 0;
}

static int loop_dev_start(ifnet *dev)
{
	if (!dev || dev->if_type != IFT_LOOP)
		return EINVAL;

	dev->if_mtu = 16384; /* can be as large as we want */
	dev->flags |= (IFF_UP|IFF_MULTICAST|IFF_LOOPBACK);
	dev->stop = &loop_dev_stop;
	
	if (dev->rx_thread < 0)
		start_rx_thread(dev);
	if (dev->tx_thread < 0)
		start_tx_thread(dev);

	if (dev->rx_thread > 0 && dev->tx_thread > 0)
		dev->flags |= IFF_RUNNING;	
	return 0;
}

static int loop_init(void)
{
	struct ifnet *me = (ifnet*)malloc(sizeof(ifnet));
	memset(me, 0, sizeof(*me));

	memset(proto, 0, sizeof(struct protosw *) * IPPROTO_MAX);

	me->devid = -1;
	me->name = "loop";
	me->unit = 0;
	me->if_type = IFT_LOOP;
	me->rx_thread = -1;
	me->tx_thread = -1;
	me->if_addrlen = 0;
	me->if_hdrlen = 0;
	me->flags = IFF_LOOPBACK | IFF_MULTICAST;
	me->start = &loop_dev_start;
	me->input = &loop_input;
	me->output = &loop_output;
	/* XXX - add ioctl */
	
#ifndef _KERNEL_MODE
	add_protosw(proto, NET_LAYER1);
	if_attach(me);
#else
	core->if_attach(me);
	core->add_protosw(proto, NET_LAYER1);

#endif

	return 0;
}


struct device_info device_info = {
	"Loopback Device Driver",
	&loop_init
};

#ifdef _KERNEL_MODE

static status_t loop_ops(int32 op, ...)
{
	switch(op) {
		case B_MODULE_INIT:
			if (get_module(CORE_MODULE_PATH, 
					(module_info **)&core) != B_OK) {
				dprintf("loop_module: failed to get core ptr\n");
				return B_ERROR;
			}
			break;
		case B_MODULE_UNINIT:
			put_module(CORE_MODULE_PATH);
			break;
		default:
			return B_ERROR;
	}
	return B_OK;
}

static struct device_module_info my_module = {
	{
		LOOP_MODULE_PATH,
		B_KEEP_LOADED,
		loop_ops
	},

	loop_init
};

_EXPORT module_info *modules[] = {
	(module_info*) &my_module,
	NULL
};

#endif


