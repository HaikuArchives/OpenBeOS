/* icmp.c
 */
 
#include <stdio.h>

#include "net_misc.h"
#include "sys/socket.h"
#include "netinet/in_systm.h"
#include "netinet/ip.h"
#include "netinet/ip_icmp.h"
#include "protocols.h"
#include "sys/protosw.h"
#include "sys/domain.h"
#include "netinet/icmp_var.h"

#include "core_module.h"
#include "net_module.h"
#include "core_funcs.h"
#include "raw/raw_module.h"

#ifdef _KERNEL_MODE
#include <KernelExport.h>
static status_t icmp_ops(int32 op, ...);
#define ICMP_MODULE_PATH	"network/protocol/icmp"
#else
#define icmp_ops NULL
#define ICMP_MODULE_PATH	"modules/protocol/icmp"
#endif

static struct core_module_info *core = NULL;
static struct raw_module_info *raw = NULL;

struct protosw* proto[IPPROTO_MAX];

static struct route icmprt;

#if SHOW_DEBUG
static void dump_icmp(struct mbuf *buf)
{
	struct ip *ip = mtod(buf, struct ip *);
	struct icmp *ic =  (struct icmp*)((caddr_t)ip + (ip->hl * 4));

	printf("ICMP: ");
	switch (ic->icmp_type) {
		case ICMP_ECHORQST:
			printf ("Echo request\n");
			break;
		case ICMP_ECHORPLY:
			printf("echo reply\n");
			break;
		default:
			printf("?? type = %d\n", ic->type);
	}
}
#endif

void icmp_input(struct mbuf *buf, int hdrlen)
{
	struct ip *ip = mtod(buf, struct ip *);	
	struct icmp *ic;
	int icl = ip->ip_len;
	int i;
	struct route rt;
	struct in_addr tmp;

	if (icl < ICMP_MINLEN) {
		icmpstat.icps_tooshort++;
		goto freeit;
	}
	i = hdrlen + min(icl, ICMP_ADVLENMIN);
	if (buf->m_len < i && (buf = m_pullup(buf, i)) == NULL) {
		icmpstat.icps_tooshort++;
		return;
	}
	ip = mtod(buf, struct ip*);		
	ic = (struct icmp*)((caddr_t)ip + hdrlen);
	rt.ro_rt = NULL;

#if SHOW_DEBUG
	dump_icmp(buf);
#endif

	if (in_cksum(buf, icl, hdrlen) != 0) {
		printf("icmp_input: checksum failed!\n");
		m_freem(buf);
		return;
	}	

	switch (ic->icmp_type) {
		case ICMP_ECHO: {	
			ic->icmp_type = ICMP_ECHOREPLY;

			ic->icmp_cksum = 0;
			ic->icmp_cksum = in_cksum(buf, icl, hdrlen);
			tmp = ip->ip_src;
			ip->ip_src = ip->ip_dst;
			ip->ip_dst = tmp;
			ip->ip_len += hdrlen;
			
			proto[IPPROTO_IP]->pr_output(buf, NULL, NULL, 0, NULL);
			return;
			break;
		}
		case ICMP_ECHOREPLY:
			break;
		default:
			break;
	}

raw:
	if (raw)
		return raw->input(buf, 0);

freeit:
	m_freem(buf);
	return;
}

static void icmp_init(void)
{
	memset(&icmprt, 0, sizeof(icmprt));

	memset(proto, 0, sizeof(struct protosw *) * IPPROTO_MAX);
	add_protosw(proto, NET_LAYER2);
#ifdef _KERNEL_MODE
	if (!raw)
		get_module(RAW_MODULE_PATH, (module_info**)&raw);

#endif
}

struct protosw my_proto = {
	"ICMP (v4)",
	ICMP_MODULE_PATH,
	0,
	NULL,
	IPPROTO_ICMP,
	PR_ATOMIC | PR_ADDR,
	NET_LAYER2,
	
	&icmp_init,
	&icmp_input,
	NULL,         /* pr_output, */
	NULL,
	NULL,         /* pr_sysctl */
	NULL,         /* pr_ctloutput */
	
	NULL,
	NULL
};

static int icmp_protocol_init(void *cpp)
{
	/* we will never call this with anything but NULL when in kernel,
	 * so this should be safe.
	 */
	if (cpp)
		core = (struct core_module_info *)cpp;
	add_domain(NULL, AF_INET);
	add_protocol(&my_proto, AF_INET);
	return 0;
}

static int icmp_protocol_stop(void)
{
	remove_protocol(&my_proto);
	return 0;
}

struct kernel_net_module_info protocol_info = {
	{
		ICMP_MODULE_PATH,
		0,
		icmp_ops
	},
	icmp_protocol_init,
	icmp_protocol_stop
};

#ifdef _KERNEL_MODE
static status_t icmp_ops(int32 op, ...)
{
	switch(op) {
		case B_MODULE_INIT:
			if (!core)
				get_module(CORE_MODULE_PATH, (module_info**)&core);
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
	(module_info *)&protocol_info,
	NULL
};
#endif