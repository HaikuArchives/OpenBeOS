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

static int loop_output(struct mbuf *m, int prot, struct sockaddr *tgt)
{
	/* turn it straight back... */
	/* This is lame as we should be detecting the protocol, but it gets
	 * us working.
	 * XXX - fix me!
	 */
	ipv4_header *ip;
	struct in_addr ia;

	ip = mtod(m, ipv4_header *);
	ia = ip->dst;
	ip->dst = ip->src;
	ip->src = ia;

	IFQ_ENQUEUE(m->m_pkthdr.rcvif->rxq, m);
}

static int loop_input(struct mbuf *buf)
{
	int fproto = IPPROTO_IP; /* XXX - Dirty hack :( */
	if (fproto >= 0 && net_modules[prot_table[fproto]].mod->input) {
		return net_modules[prot_table[fproto]].mod->input(buf);
	} else {
		printf("Failed to determine a valid protocol fproto = %d\n", fproto);
	}

	m_freem(buf);
	return 0;
}

int loop_init(loaded_net_module *ln, int *pt)
{
	ifnet *ifn = (ifnet*)malloc(sizeof(ifnet));

        net_modules = ln;
        prot_table = pt;

	ifn->devid = -1;
	ifn->name = "loop";
	ifn->unit = 0;
        ifn->if_type = IFT_LOOP;
	ifn->rxq = NULL;
        ifn->rx_thread = -1;
        ifn->tx_thread = -1;
        ifn->txq = NULL;
        ifn->if_addrlist = NULL;

	ifn->input = &loop_input;
	ifn->output = &loop_output;
	ifn->ioctl = NULL;

	net_server_add_device(ifn);

        return 0;
}

int loop_dev_init(ifnet *dev)
{
	if (!dev)
		return EINVAL;

	if (dev->if_type != IFT_LOOP)
		return EINVAL;

	dev->if_mtu = 16384; /* can be as large as we want */
       	dev->flags |= (IFF_UP|IFF_RUNNING|IFF_MULTICAST);

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
	&loop_input,
	&loop_output,
	NULL,
	NULL,
	NULL
};

