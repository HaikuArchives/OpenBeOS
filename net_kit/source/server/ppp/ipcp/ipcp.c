#ifdef USER
#include <stdio.h>
#endif

#include <sys/param.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <sys/socket.h>

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "core_module.h"
#include "core_funcs.h"
#include "../ppp_defs.h"
#include "net/ppp_stats.h"
#include "net/if_ppp.h"
#include "../fsm.h"
#include "ipcp.h"

#include "../ppp_module.h"
#include "../ppp_funcs.h"

static struct core_module_info *core;
static struct ppp_module_info *ppp = NULL;

static char *matches[] = {
	"ipcp",
};

static void add_matches(struct ppp_proto_match **existing, 
                        struct ppp_module_info *ptr)
{
	struct ppp_proto_match *p, *m;
	int i;
	
	if ((p = *existing) != NULL)
		for (; p->next ; p = p->next)
			continue;
		
	for (i=0; i < sizeof(matches) / sizeof(*matches);i++) {
		m = (struct ppp_proto_match*)malloc(sizeof(*m));
		if (!m)
			return;
		memset(m, 0, sizeof(*m));
		m->mod = ptr;
		m->layer = PPP_MODULE_LAYER3;
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

static const char *ipcp_protoname(int proto)
{
	static const char * const cftypes[] = {
		NULL,
		"IP Addresses",
		"IP Compression",
		"IP Address",
	};

	if (proto < 0 || proto > sizeof cftypes / sizeof *cftypes ||
	    cftypes[proto] == NULL)
		return "unknown";

	return cftypes[proto];
}

static char *ip_ntoa(uint32 ipaddr)
{
    static char b[64];

    ipaddr = ntohl(ipaddr);

    sprintf(b, "%d.%d.%d.%d",
	    (u_char)(ipaddr >> 24),
	    (u_char)(ipaddr >> 16),
	    (u_char)(ipaddr >> 8),
	    (u_char)(ipaddr));
    return b;
}

static void ipcp_sconfreq(struct fsm *fsm)
{
	/* OK, so we need to build a request and then send it... */
	struct mbuf *m = m_gethdr(MT_DATA);
	uchar *ptr;
	struct ipcp_options *req = (struct ipcp_options*)fsm->our_req;
	struct ipcpheader *lh;
	int len = 0;
printf("ipcp_sconfreq\n");
	if (!m)
		return;

	ptr = mtod(m, uchar*);
	m_reserve(m, 14);
	ptr = mtod(m, uchar*);

	/* XXX - add others :) */
	if (req->want_addr) {
		struct ipcp_opt *opt = (struct ipcp_opt*) ptr;
		opt->type = IPCP_ADDRESS;
		opt->len = 6;
		if (req->address != 0) {
			/* ??? - ordering??? */
			memcpy(ptr + 2, &req->address, 4);
		}
		ptr += 6;
		len += 6;
	}

	m->m_len = len;
	m->m_pkthdr.len = len;
	
	fsm_Output(fsm, PCC_CONFIGREQ, fsm->reqid, NULL, len, m);
}	

static void ipcp_checkconfig(struct fsm *fsm, struct mbuf *m, struct fsm_decode *dec)
{
	uint8 *cp = mtod(m, uint8 *), *wp;
	struct ipcpheader *lcph = mtod(m, struct ipcpheader*);
	int len = lcph->length - sizeof(struct ipcpheader);
	struct ipcp_options *ao = (struct ipcp_options *)fsm->allowed;
	struct ipcp_options *ho = (struct ipcp_options *)fsm->his_req;

printf("ipcp_checkconfig\n");	
	/* strip off the header as no longer needed */
	m_adj(m, sizeof(struct ipcpheader));
	cp = mtod(m, uint8*);
		
	wp = cp;
	while (len > 0) {
		struct ipcp_opt *lo = (struct ipcp_opt*)cp;
		printf("IPCP_OPT: type   %d [%s]\n", lo->type, ipcp_protoname(lo->type));

		switch (lo->type) {
			case IPCP_ADDRESSES:
				/* old, shouldn't be used, so just fall through */
			case IPCP_ADDRESS:
				if (ao->want_addr) {
					ho->address = *((uint32*)&lo->ipcp_data);
					memcpy(dec->ackend, cp, lo->len);
					dec->ackend += lo->len;
				} else {
					memcpy(dec->rejend, cp, lo->len);
					dec->rejend += lo->len;
				}
				break;
			case IPCP_COMPPROT:
				/* we don't currently do compression, so just reject */
				memcpy(dec->rejend, cp, lo->len);
				dec->rejend += lo->len;
				break;
		}
		cp += lo->len;
		len -= lo->len;
	}
}

/* Just to show what's going on... */
static void ipcp_input(struct fsm *fsm, struct mbuf *m)
{
	/* we ignore ConfigReq messages when in the following states... */
	uint8 *cp = mtod(m, uint8 *), *wp;
	struct ipcpheader *lcph = mtod(m, struct ipcpheader*);
	int len = ntohs(lcph->length) - sizeof(struct ipcpheader);

	if (lcph->code > IPCP_MAX_CODE) {
		printf("** Invalid code for IPCP!\n");
		/* XXX - should send a CodeReject */
		m_freem(m);
		return;
	}

	/* skip the header */
	m_adj(m, sizeof(struct ipcpheader));
	cp = mtod(m, uint8*);
	
	wp = cp;
	while (len > 0) {
		struct ipcp_opt *io = (struct ipcp_opt*)cp;
		printf("IPCP_OPT: id     %d [%s]\n", io->type, ipcp_protoname(io->type));
	
		switch (io->type) {
			case IPCP_ADDRESSES:
				printf("\tIP_ADDRESSES has been deprecated! ???\n");
			case IPCP_ADDRESS:
				printf("\tAddress: %s\n", ip_ntoa(*((uint32*)&io->ipcp_data[0])));
				break;
			case IPCP_COMPPROT: {
				uint16 comp = ntohs(io->comp_prot);
				printf("\tProtocol requested was %04x\n", comp);
				printf("\tMax  slot Id : %02x\n", io->comp_data[0]);
				printf("\tComp slot Id : %02x\n", io->comp_data[1]);
				break;
			}
			default:
				//printf("Unknown type for IPCP\n");
		}
		cp += io->len;
		len -= io->len;
	}
}

static int ipcp_up(struct fsm *fsm)
{
	/* OK, a lower layer has come up.
	 * Send our initial config request
	 */
	ipcp_sconfreq(fsm);
	return 1;
}

static void ipcp_start(struct fsm *fsm)
{
//	struct ipcp_options *opts = (struct ipcp_options*)fsm->our_req;
	/* nothing to do at present */	
}

static struct ppp_module ipcp_protocol = {
	IPCP_NAME,
	ipcp_up,           /* up */
	NULL,              /* down */
	ipcp_start,        /* start */
	NULL,              /* finish */
	NULL,              /* InitRestartTimer */
	ipcp_sconfreq,     /* sendconfig */
	ipcp_input,        /* input */
	ipcp_checkconfig,  /* DecodeConfig */
	NULL               /* SendTerminateAck */
};

void ipcp_init_fsm(struct fsm *fsm)
{
	struct ipcp_options *ao = NULL, *oo = NULL, *ho = NULL;
	fsm->name = IPCP_NAME;
	fsm->proto = PPP_IPCP;
	fsm->min_code = PCC_CONFIGREQ;
	fsm->max_code = PCC_CODEREJ;
	fsm->layer = PPP_MODULE_LAYER3;
	
	ao = (struct ipcp_options*)malloc(sizeof(struct ipcp_options));
	oo = (struct ipcp_options*)malloc(sizeof(struct ipcp_options));
	ho = (struct ipcp_options*)malloc(sizeof(struct ipcp_options));
	if (!ao || !oo || !ho)
		return;
	
	memset(ao, 0, sizeof(struct ipcp_options));
	memset(oo, 0, sizeof(struct ipcp_options));
	memset(ho, 0, sizeof(struct ipcp_options));

	/* Set allowable options */
	ao->want_addr = oo->want_addr = 1;
	ao->want_comp = oo->want_comp = 0;
	fsm->allowed = (void*)ao;
	
	/* Set our options */
	oo->address = htonl(0xc0a80020);

	fsm->our_req = (void*)oo;
	fsm->his_req = (void*)ho;
	
	fsm->mod = &ipcp_protocol;
}

static int ipcp_module_init(void *cpp)
{
	if (cpp)
		core = cpp;

	return 0;
}

#ifndef _KERNEL_
void set_core(struct core_module_info *cp)
{
	core = cp;
}

void set_ppp(struct ppp_module_info *pp)
{
	ppp = pp;
}
#endif


_EXPORT struct ppp_module_info protocol_info = {
	{
		{
			IPCP_MODULE_PATH,
			0,
			std_ops
		},
		ipcp_module_init,
		NULL /*lcp_module_stop*/
	},
#ifndef _KERNEL_
	set_core,
	set_ppp,
#endif
	add_matches,
	ipcp_init_fsm,
	
	NULL,
	NULL,
	NULL
};

#ifdef _KERNEL_
static status_t std_ops(int32 op, ...)
{
	switch(op) {
		case B_MODULE_INIT:
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
