/* loop.c - loopback device 
 */

#include<stdio.h>
#include <malloc.h>

#include "net_module.h"
#include "sys/socket.h"
#include "protocols.h"
#include "netinet/in.h"
#include "ipv4/ipv4.h"

static loaded_net_module *net_modules;
static int *prot_table;
static struct ifnet *me = NULL;


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

int loop_input(struct mbuf *buf, int hdrlen)
{
	int fproto = IPPROTO_IP; /* XXX - Dirty hack :( */

	if (fproto >= 0 && net_modules[prot_table[fproto]].mod->input) {
		return net_modules[prot_table[fproto]].mod->input(buf, 0);
	} else {
		printf("Failed to determine a valid protocol fproto = %d\n", fproto);
	}

	m_freem(buf);

	return 0;
}

int loop_init(loaded_net_module *ln, int *pt)
{
	me = (ifnet*)malloc(sizeof(ifnet));

        net_modules = ln;
        prot_table = pt;

	me->devid = -1;
        me->name = "loop";
        me->unit = 0;
        me->if_type = IFT_LOOP;
        me->rxq = NULL;
        me->rx_thread = -1;
        me->txq = NULL;
        me->tx_thread = -1;
        me->if_addrlist = NULL;

	me->input = &loop_input;
	me->output = &loop_output;
	me->ioctl = NULL;

	net_server_add_device(me);

        return 0;
}

int loop_dev_init(ifnet *dev)
{
	if (!dev || dev->if_type != IFT_LOOP)
		return EINVAL;

	dev->if_mtu = 16384; /* can be as large as we want */
       	dev->flags |= (IFF_UP|IFF_RUNNING|IFF_MULTICAST|IFF_LOOPBACK);

	return 0;
}

net_module net_module_data = {
	"Loopback Device Driver",
	NS_LOOP,
	NET_LAYER1,
        0,
        0,
	0,

	&loop_init,
	&loop_dev_init,
	NULL,
	NULL,
	NULL
};

