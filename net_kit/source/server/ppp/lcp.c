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
#include "net/ppp_defs.h"
#include "net/if_ppp.h"
#include "lcp.h"

extern struct core_module_info *core;

static const char *codename(int code)
{
	static const char * const codes[] = {
		"Invalid",
		"Configure-Request",
		"Configure-Ack",
		"Configure-Nak",
		"Configure-Reject",
		"Terminate-Request",
		"Terminate-Ack",
		"Code-Reject",
		"Protocol-Reject",
		"Echo-Request",
		"Echo-Reply",
		"Discard-request",
	};
	if (code < 0 || code > sizeof(codes) / sizeof(*codes))
		return "Invalid value";
	return codes[code];
}

static const char *protoname(int proto)
{
	static const char * const cftypes[] = {
		/* Check out the latest ``Assigned numbers'' rfc (1700) */
		NULL,
		"MRU",			/* 1: Maximum-Receive-Unit */
		"ACCMAP",		/* 2: Async-Control-Character-Map */
		"AUTHPROTO",	/* 3: Authentication-Protocol */
		"QUALPROTO",	/* 4: Quality-Protocol */
		"MAGICNUM",		/* 5: Magic-Number */
		"RESERVED",		/* 6: RESERVED */
		"PROTOCOMP",	/* 7: Protocol-Field-Compression */
		"ACFCOMP",		/* 8: Address-and-Control-Field-Compression */
		"FCSALT",		/* 9: FCS-Alternatives */
		"SDP",			/* 10: Self-Describing-Pad */
		"NUMMODE",		/* 11: Numbered-Mode */
		"MULTIPROC",	/* 12: Multi-Link-Procedure */
		"CALLBACK",		/* 13: Callback */
		"CONTIME",		/* 14: Connect-Time */
		"COMPFRAME",	/* 15: Compound-Frames */
		"NDE",			/* 16: Nominal-Data-Encapsulation */
		"MRRU",			/* 17: Multilink-MRRU */
		"SHORTSEQ",		/* 18: Multilink-Short-Sequence-Number-Header */
		"ENDDISC",		/* 19: Multilink-Endpoint-Discriminator */
		"PROPRIETRY",	/* 20: Proprietary */
		"DCEID",		/* 21: DCE-Identifier */
		"MULTIPP",		/* 22: Multi-Link-Plus-Procedure */
		"LDBACP",		/* 23: Link Discriminator for BACP */
	};

	if (proto < 0 || proto > sizeof cftypes / sizeof *cftypes ||
	    cftypes[proto] == NULL)
		return "unknown";

	return cftypes[proto];
}

void lcp_output(struct lcp *lcp, uint code, uint id, char *ptr,
                int count, struct mbuf *m0)
{
	int plen = sizeof(struct lcpheader) + count;
	struct mbuf *m = m0;
	struct lcpheader *lh;
	struct sockaddr sa;
	
	printf("LCP_OUTPUT: %s\n", codename(code));
	
	if (m0) {
		plen = m->m_len;
	} else {
		m = m_get(MT_DATA);
		if (!m)
			return;
		/* copy in data */
		m->m_len = sizeof(struct lcpheader) + count;
		memcpy((void*)(mtod(m, char*) + sizeof(struct lcpheader)),
		       (void*)ptr, count);
	}
	lh = mtod(m, struct lcpheader*);
	lh->code = code;
	lh->id = id;
	lh->length = htons(plen);
	
	sa.sa_len = sizeof(sa);
	sa.sa_family = AF_UNSPEC;
	sa.sa_data[0] = 0xc0;
	sa.sa_data[1] = 0x21;
	
	lcp->ifp->output(lcp->ifp, m, &sa, NULL);
}	

void lcp_setup(struct lcp *lcp, int openmode)
{
	lcp->open_mode = openmode;

	lcp->his_mru = PPP_MRU;
	lcp->his_mrru = 0;
	lcp->his_magic = 0;
	lcp->his_lqrperiod = 0;
	lcp->his_acfcomp = 0;
	lcp->his_auth = 0;
	lcp->his_authtype = 0;
	lcp->his_shortseq = 0;
	lcp->mru_req = 0;

	if ((lcp->want_mru = lcp->cfg.mru) == 0)
		lcp->want_mru = PPP_MRU;
	lcp->want_acfcomp = 0;

	lcp->his_accmap = lcp->want_accmap = 0;
	lcp->his_protocomp = lcp->want_protocomp = 1;
	lcp->want_magic = 0;
	lcp->want_auth = 0;
	lcp->want_authtype = 0;
	lcp->want_lqrperiod = 0;

	lcp->his_reject = lcp->my_reject = 0;
	lcp->auth_iwait = lcp->auth_ineed = 0;
	lcp->LcpFailedMagic = 0;
}

void lcp_init(struct lcp *lcp, struct ifnet *ifp)
{
  /* Initialise ourselves */
	lcp->ifp = ifp;
	lcp->cfg.mru = 0;
	lcp->cfg.max_mru = MAX_MRU;
	lcp->cfg.mtu = 0;
	lcp->cfg.max_mtu = MAX_MTU;
	lcp->cfg.accmap = 0;
	lcp->cfg.openmode = 1;
	lcp->cfg.lqrperiod = 0;
	lcp->cfg.acfcomp = 0;
	lcp->cfg.chap05 = 0;
	lcp->cfg.lqr = 0;
	lcp->cfg.pap = 0;
	lcp->cfg.protocomp = 0;
	*lcp->cfg.ident = '\0';

	lcp_setup(lcp, lcp->cfg.openmode);
}


static void lcp_checkconfigreq(struct mbuf *m, struct lcp *lcp)
{
	/* we ignore ConfigReq messages when in the following states... */
	uint8 *cp = mtod(m, uint8 *), *wp;
	struct lcpheader *lcph = mtod(m, struct lcpheader*);
	struct lcp_decode dec;
	char *ackend = &dec.ack[0],/* *nakend = &dec.nak[0],*/ *rejend = &dec.rej[0];
	int len = lcph->length - sizeof(struct lcpheader);
	int resplen = 0;
	
	/* Should check state here... */
		
	/* skip the header */
	cp += sizeof(struct lcpheader);
	wp = cp;
	while (len > 0) {
		struct lcp_opt *lo = (struct lcp_opt*)cp;
		printf("LCP_OPT: id     %d [%s]\n", lo->id, protoname(lo->id));
	
		switch (lo->id) {
			case LCPO_MRU: {
				uint16 mru = ntohs(*((uint16*)&lo->data[0]));
				printf("\tMRU requested %04x\n", mru);
				break;
			}
			case LCPO_ACCMAP: {
				uint32 map = ntohl(*(uint32*)&lo->data[0]);
				printf("\taccmap = %08lx\n", map);
				break;
			}
			case LCPO_MAGICNUM: {
				uint32 magic = ntohl(*(uint32*)&lo->data[0]);
				printf("\tMagic # %08lx\n", magic);
				/* it's a request! */
				if (lcp->want_magic == magic) {
					/* it's invalid */
					memcpy(rejend, cp, 6);
					rejend += 6;
				} else {
					lcp->his_magic = magic;
					lcp->LcpFailedMagic = 0;
					memcpy(ackend, cp, 6);
					ackend += 6;
				}
				break;
			}
			case LCPO_PROTOCOMP:
				/* we don't do this yet! */
				memcpy(rejend, cp, 2);
				rejend += 2;
				printf("\tPROTOCOMP rejected\n");
				break;
			case LCPO_ACFCOMP:
				/* we don't do this yet! */
				memcpy(rejend, cp, 2);
				rejend += 2;
				printf("\tACFCOMP rejected\n");
				break;
			case LCPO_CALLBACK: {
				uint8 ctype = lo->data[0];
				memcpy(rejend, cp, lo->len);
				rejend += lo->len;
				printf("\tCallback: %d\n", ctype);
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
	/* Did we reject any option requests */
	if ((resplen = rejend - (char*)&dec.rej[0]) != 0) {
		m_adj(m, m->m_len - resplen - sizeof(struct lcpheader));
		memcpy((void*)wp, &dec.rej[0], resplen);
		lcp_output(lcp, CODE_CONFIGREJ, lcph->id, NULL, 0, m);
		return;
	}
	
	/* XXX - add NAK code :) */
	
	/* We accept his requested config ... tell him */
	if ((resplen = ackend - (char*)&dec.ack[0]) != 0) {
		m_adj(m, m->m_len - resplen - sizeof(struct lcpheader));
		memcpy((void*)wp, &dec.ack[0], resplen);
		lcp_output(lcp, CODE_CONFIGACK, lcph->id, NULL, 0, m);
	}
	{
		lcp_output(lcp, CODE_CONFIGREQ, lcph->id++, NULL, 0, NULL);
	}
}
	


void lcp_input(struct mbuf *bp, struct lcp *lcp)
{
	struct lcpheader *lcph = mtod(bp, struct lcpheader*);
#if 0
	printf("LCP: code   = %02x\n", lcph->code);
	printf("LCP: id     = %02x\n", lcph->id);
	printf("LCP: length = %04x\n", lcph->length);
#endif

	lcph->length = ntohs(lcph->length);
	if (bp->m_len < lcph->length) {
		printf("LCP: packet is too short! (%ld vs %d)\n", bp->m_len, lcph->length);
		m_freem(bp);
		return;
	}

	printf("LCP: %s\n", codename(lcph->code));
	switch (lcph->code) {
		case CODE_CONFIGREQ:
			lcp_checkconfigreq(bp, lcp);
			return;
		default:
			printf("LCP: Not yet able to handle %s\n", codename(lcph->code));
	}
	
	return;
}
