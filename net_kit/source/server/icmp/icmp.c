/* icmp.c
 */
 
#include <stdio.h>

#include "mbuf.h"
#include "icmp.h"
#include "ipv4/ipv4.h"
#include "net_module.h"
#include "protocols.h"
#include "sys/socket.h"
#include "sys/protosw.h"
#include "sys/domain.h"

#ifdef _KERNEL_MODE
#include <KernelExport.h>
#include "net_server/core_module.h"

#define m_freem             core->m_freem
static struct core_module_info *core = NULL;
#define ICMP_MODULE_PATH	"network/protocol/icmp"
#else
#define ICMP_MODULE_PATH	"modules/protocol/icmp"
#endif

struct protosw* proto[IPPROTO_MAX];

static struct route icmprt;

#if SHOW_DEBUG
static void dump_icmp(struct mbuf *buf)
{
	ipv4_header *ip = mtod(buf, ipv4_header *);
	icmp_header *ic =  (icmp_header*)((caddr_t)ip + (ip->hl * 4));

        printf("ICMP: ");
        switch (ic->type) {
		case ICMP_ECHO_RQST:
			printf ("Echo request\n");
			break;
		case ICMP_ECHO_RPLY:
			printf("echo reply\n");
			break;
		default:
			printf("?? type = %d\n", ic->type);
	}
}
#endif

int icmp_input(struct mbuf *buf, int hdrlen)
{
	ipv4_header *ip = mtod(buf, ipv4_header *);	
	icmp_header *ic;
	int icl = ip->length - hdrlen;
	struct route rt;
	int error = 0;
	struct in_addr tmp;

	ic = (icmp_header*)((caddr_t)ip + hdrlen);
	rt.ro_rt = NULL;

#if SHOW_DEBUG
	dump_icmp(buf);
#endif

	if (in_cksum(buf, icl, hdrlen) != 0) {
		printf("Checksum failed!\n");
		m_freem(buf);
		return 0;
	}	

	switch (ic->type) {
		case ICMP_ECHO_RQST: {
			icmp_echo *ie = (icmp_echo *)ic;
			ie->hdr.type = ICMP_ECHO_RPLY;

			ie->hdr.cksum = 0;
			ie->hdr.cksum = in_cksum(buf, icl, hdrlen);
			tmp = ip->src;
			ip->src = ip->dst;
			ip->dst = tmp;

			if (satosin(&icmprt.ro_dst)->sin_addr.s_addr == ip->src.s_addr)
				rt = icmprt;
			else {
				/* fill in target structure... */
				satosin(&rt.ro_dst)->sin_family = AF_INET;
				satosin(&rt.ro_dst)->sin_len = sizeof(rt.ro_dst);
				satosin(&rt.ro_dst)->sin_addr = ip->src;
			}
			error = proto[IPPROTO_IP]->pr_output(buf, NULL, &rt, 0, NULL);
			if (error == 0)
				icmprt = rt;
			return error;
			break;
		}
		case ICMP_ECHO_RPLY:
			break;
		default:
	}
	m_freem(buf);
	return 0;
}

static void icmp_init(void)
{
	memset(&icmprt, 0, sizeof(icmprt));

	memset(proto, 0, sizeof(struct protosw *) * IPPROTO_MAX);
#ifndef _KERNEL_MODE
	add_protosw(proto, NET_LAYER2);
#else
	core->add_protosw(proto, NET_LAYER2);
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
	NULL,//&icmp_output,
	NULL,
	
	NULL,
	NULL
};

#ifndef _KERNEL_MODE

static void icmp_protocol_init(void)
{
	add_domain(NULL, AF_INET);
	add_protocol(&my_proto, AF_INET);
}

struct protocol_info protocol_info = {
	"ICMP (v4) Module",
	&icmp_protocol_init
};

#else

static status_t icmp_ops(int32 op, ...)
{
	switch(op) {
		case B_MODULE_INIT:
			if (!core)
				get_module(CORE_MODULE_PATH, (module_info**)&core);
			if (!core)
				return B_ERROR;
				
			core->add_domain(NULL, AF_INET);
			core->add_protocol(&my_proto, AF_INET);			
			return B_OK;
		case B_MODULE_UNINIT:
			break;
		default:
			return B_ERROR;
	}
	return B_OK;
}

static module_info my_module = {
	ICMP_MODULE_PATH,
	B_KEEP_LOADED,
	icmp_ops
};

_EXPORT module_info *modules[] = {
	&my_module,
	NULL
};

#endif

