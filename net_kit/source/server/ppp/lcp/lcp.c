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
#include "lcp.h"

#include "../ppp_module.h"
#include "../ppp_funcs.h"

static struct core_module_info *core;
static struct ppp_module_info *ppp = NULL;

static char *matches[] = {
	"lcp",
};

static void add_matches(struct ppp_proto_match **existing, 
                        struct ppp_module_info *ptr)
{
	struct ppp_proto_match *p, *m;
	int i;
printf("existing = %p (%p)\n", existing, *existing);
	
	if ((p = *existing) != NULL)
		for (; p->next ; p = p->next) {
			printf("%p\n", p);
			continue;
		}
		
	for (i=0; i < sizeof(matches) / sizeof(*matches);i++) {
		m = (struct ppp_proto_match*)malloc(sizeof(*m));
		if (!m)
			return;
		memset(m, 0, sizeof(*m));
		m->mod = ptr;
		m->layer = PPP_MODULE_LAYER1;
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

static void lcp_sconfreq(struct fsm *fsm)
{
	/* OK, so we need to build a request and then send it... */
	struct mbuf *m = m_gethdr(MT_DATA);
	uchar *ptr;
	struct lcp_options *req = (struct lcp_options*)fsm->our_req;
	int rlen = 0;

	if (!m)
		return;

	ptr = mtod(m, uchar*);
	m_reserve(m, 14);
	ptr = mtod(m, uchar*);

	/* Build options... */
	if (req->protocomp) {
		struct lcp_opt *opt = (struct lcp_opt*)ptr;
		opt->id = LCPO_PROTOCOMP;
		opt->len = 2;
		ptr += 2;rlen += 2;
	}
	if (req->acfcomp) {
		struct lcp_opt *opt = (struct lcp_opt*)ptr;
		opt->id = LCPO_ACFCOMP;
		opt->len = 2;
		ptr += 2;rlen += 2;
	}
	
	fsm_Output(fsm, PCC_CONFIGREQ, fsm->reqid, NULL, rlen, m);
}	
	
static void lcp_checkconfig(struct fsm *fsm, struct mbuf *m, struct fsm_decode *dec)
{
	uint8 *cp = mtod(m, uint8 *), *wp;
	struct lcpheader *lcph = mtod(m, struct lcpheader*);
	int len = lcph->length - sizeof(struct lcpheader);
	struct lcp_options *ao = (struct lcp_options *)fsm->allowed;
	struct lcp_options *ho = (struct lcp_options *)fsm->his_req;
	
	/* strip off the header as no longer needed */
	m_adj(m, sizeof(struct lcpheader));
	cp = mtod(m, uint8*);
		
	wp = cp;
	while (len > 0) {
		struct lcp_opt *lo = (struct lcp_opt*)cp;
		printf("LCP_OPT: id     %d [%s]\n", lo->id, protoname(lo->id));
	
		switch (lo->id) {
			case LCPO_MRU: {
				ho->mru = ntohs(*((uint16*)&lo->data[0]));
				printf("\tMRU requested %04x\n", ho->mru);
				break;
			}
			case LCPO_ACCMAP: {
				uint32 map = ntohl(*(uint32*)&lo->data[0]);
				printf("\taccmap = %08lx\n", map);
				break;
			}
			case LCPO_MAGICNUM: {
				ho->magic = ntohl(*(uint32*)&lo->data[0]);
				printf("\tMagic # %08lx\n", ho->magic);
				if (ao->want_magic) {
					memcpy(dec->ackend, cp, lo->len);
					dec->ackend += lo->len;
					printf("accepted magic...\n");
				} else {
					/* check value */
				}
				break;
			}
			case LCPO_PROTOCOMP:
				if (ao->protocomp) {
					ho->protocomp = 1;
					/* accept */
				} else {
					memcpy(dec->rejend, cp, 2);
					dec->rejend += 2;
					/* reject */
					printf("\tPROTOCOMP rejected\n");
				}
				break;
			case LCPO_ACFCOMP:
				if (ao->acfcomp) {
					ho->acfcomp = 1;
					/* accept */
				} else {
					memcpy(dec->rejend, cp, 2);
					dec->rejend += 2;				
					/* reject */
					printf("\tACFCOMP rejected\n");
				}
				break;
			case LCPO_CALLBACK: {
				if (ao->callback) {
					/* accept */
				} else {
					/* reject */
				}
				printf("\tCallback\n");
				break;
			}
			case LCPO_MRRU: {
				uint16 mrru = ntohs(*((uint16*)&lo->data[0]));
				printf("\tMRRU requested %04x\n", mrru);
				break;
			}
			case LCPO_ENDDISC: {	
				char endp[lo->len - 1];
				int i;
				memcpy(&endp, &lo->data, lo->len - 2);
				printf("\tendpoint id ");
				for (i = 0; i < lo->len - 2; i++)
					printf("%02x", endp[i]);
				printf("\n");
				break;
			}
		}

		cp += lo->len;
		len -= lo->len;
	}
}

static int lcp_up(struct fsm *fsm)
{
	/* OK, our lower (physical) link has come up.
	 * Send our initial config request
	 */
	lcp_sconfreq(fsm);
	return 1;
}

static void lcp_Down(struct fsm *fsm)
{
	memset(fsm->his_req, 0, sizeof(struct lcp_options));
}

void lcp_start(struct fsm *fsm)
{
	struct lcp_options *opts = (struct lcp_options*)fsm->our_req;
	
	fsm->failedmagic = 0;
	fsm->reqs = fsm->naks = fsm->rejs = fsm->maxtries * 3;
	opts->mru = 0;
}

void lcp_input(struct fsm *fsm, struct mbuf *bp)
{
	struct lcpheader *lcph = mtod(bp, struct lcpheader*);
#if 0
	printf("LCP: code   = %02x\n", lcph->code);
	printf("LCP: id     = %02x\n", lcph->id);
	printf("LCP: length = %04x\n", ntohs(lcph->length));
#endif

	lcph->length = ntohs(lcph->length);
	if (bp->m_len < lcph->length) {
		printf("LCP: packet is too short! (%ld vs %d)\n", bp->m_len, lcph->length);
		m_freem(bp);
		return;
	}

	printf("LCP: %s\n", codename(lcph->code));
	switch (lcph->code) {
		case PCC_CONFIGREQ:
			//lcp_checkconfig(fsm, bp);
			return;
		default:
			printf("LCP: Not yet able to handle %s\n", codename(lcph->code));
	}
	
	return;
}

static struct ppp_module lcp_protocol = {
	LCP_NAME,
	lcp_up,           /* up */
	lcp_Down,         /* down */
	lcp_start,        /* start */
	NULL,             /* finish */
	NULL,             /* InitRestartTimer */
	lcp_sconfreq,     /* sendconfig */
	lcp_input,        /* input */
	lcp_checkconfig,  /* DecodeConfig */
	NULL              /* SendTerminateAck */
};
 
void lcp_init_fsm(struct fsm *fsm)
{
	struct lcp_options *ao = NULL, *oo = NULL, *ho = NULL;
	fsm->name = LCP_NAME;
	fsm->proto = PPP_LCP;
	fsm->min_code = PCC_CONFIGREQ;
	fsm->max_code = PCC_TIMEREM;
	fsm->layer = PPP_MODULE_LAYER1;
	
	ao = (struct lcp_options*)malloc(sizeof(struct lcp_options));
	oo = (struct lcp_options*)malloc(sizeof(struct lcp_options));
	ho = (struct lcp_options*)malloc(sizeof(struct lcp_options));
	if (!ao || !oo || !ho)
		return;
	
	memset(ao, 0, sizeof(struct lcp_options));
	memset(oo, 0, sizeof(struct lcp_options));
	memset(ho, 0, sizeof(struct lcp_options));

	/* Set allowable options */
	ao->max_mtu = PPP_MAX_MTU;
	ao->max_mru = PPP_MAX_MRU;
	ao->want_magic = 1;
	fsm->allowed = (void*)ao;
	
	/* Set our options */
	oo->mtu = PPP_MTU;
	oo->mru = PPP_MRU;

	fsm->our_req = (void*)oo;
	fsm->his_req = (void*)ho;
	
	fsm->mod = &lcp_protocol;
}

static int lcp_module_init(void *cpp)
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
			LCP_MODULE_PATH,
			0,
			std_ops
		},
		lcp_module_init,
		NULL /*lcp_module_stop*/
	},
#ifndef _KERNEL_
	set_core,
	set_ppp,
#endif
	add_matches,
	lcp_init_fsm,
	
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
