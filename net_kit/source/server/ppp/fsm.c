#include <kernel/OS.h>
#ifdef USER
#include <stdio.h>
#endif

#include <sys/mbuf.h>
#include <net/if.h>

#include "net/if.h"
#include "net/ppp_stats.h"
#include "net/if_ppp.h"
#include "ppp_device.h"
#include "ppp_defs.h"
#include "fsm.h"
#include "core_module.h"
#include "ppp_module.h"
#include "core_funcs.h"

enum {
	 PARENT_UP = 1,
	 PARENT_START,
	 PARENT_DOWN,
	 PARENT_FINISH
};

extern struct core_module_info *core;

void fsm_Output(struct fsm *fsm, uint code, uint8 id, uchar *ptr, 
                int count, struct mbuf *m)
{
	int plen = sizeof(struct fsmheader) + count;
	struct fsmheader *lh = NULL;
	struct mbuf *m0 = m;
	struct sockaddr sa;

	if (!m0) {
		m0 = m_gethdr(MT_DATA);
		if (!m0) {
			printf("Failed to get an mbuf!\n");
			return;
		}	
	} else {
		/* We have an existing mbuf (good) so let's adjust the lengths
		 * correctly and add space for our header.
		 */
		m0->m_len = count;
		if (m0->m_flags & M_PKTHDR)
			m0->m_pkthdr.len = count;
		m0 = m_prepend(m0, sizeof(struct fsmheader));
		if (!m0) {
			printf("Failed to get an mbuf on m_prepend...\n");
			return;
		}
	}
	
	lh = mtod(m0, struct fsmheader *);
	if (!lh) {
		printf("Got an invalid pointer for header (%p)!\n", lh);
		if (!m)
			m_freem(m0);
		return;
	}
	
	if (fsm->LogLevel > 3) {
		printf("%s: %s: Send %s(%d) state = %s\n",
		       fsm->sc->sc_if.if_name, fsm->name, codename(code), id, state2nam(fsm->state));
	}

	lh->code = code;
	lh->id = id;
	lh->length = htons(plen);
	
	/* Ugh...should make sure we have enough room for this... */
	if (count > 0 && ptr != NULL)
		memcpy((void*)(mtod(m0, char*) + sizeof(struct fsmheader)), ptr, count);

	sa.sa_len = sizeof(sa);
	sa.sa_family = AF_UNSPEC;
	*(uint16*)&sa.sa_data = htons(fsm->proto);
	
	fsm->sc->sc_if.output(&fsm->sc->sc_if, m0, &sa, NULL);
/*
	if (code == PCC_CONFIGREJ)
		lcp_SendIdentification(&fp->link->lcp);
*/
}

static void fsm_newstate(struct fsm *fsm, int newst)
{
	printf("%s: %s: State change %s --> %s\n",
	       fsm->sc->sc_if.if_name, fsm->name, state2nam(fsm->state), state2nam(newst));
/*
	if (fsm->state == STM_STOPPED && fsm->StoppedTimer.state == TIMER_RUNNING)
		timer_Stop(&fp->StoppedTimer);
*/
	fsm->state = newst;
/*
	if ((newst >= STM_INITIAL && newst <= ST_STOPPED) || (newst == ST_OPENED)) {
		timer_Stop(&fsm->FsmTimer);
		if (newst == STM_STOPPED && fsm->StoppedTimer.load) {
			timer_Stop(&fp->StoppedTimer);
			fp->StoppedTimer.func = StoppedTimeout;
			fp->StoppedTimer.arg = (void *) fp;
			timer_Start(&fp->StoppedTimer);
		}
	}
*/
}

static void fsm_sconfreq(struct fsm *fsm)
{
	if (fsm->reqs-- > 0 && fsm->restart-- > 0) {
		if (fsm->mod && fsm->mod->SendConfigReq) 
			(*fsm->mod->SendConfigReq)(fsm);
		//timer_Start(&fp->FsmTimer);		/* Start restart timer */
	} else {
		if (fsm->reqs < 0)
			printf("%s:%s:  Too many REQs sent - abandoning "
			       "negotiation\n",
			       fsm->sc->sc_if.if_name, fsm->name);
//		lcp_SendIdentification(&fp->link->lcp);
		fsm_Close(fsm);
	}
}

/* XXX - This can be improved by using the same code and flags 
 * for the various combinations of up/down etc, but
 * this is just a quick trial to make sure it works :)
 */
void fsm_parent_start(struct fsm *fsm)
{
	struct ppp_softc *sc = fsm->sc;
	struct fsm *mods = sc->fsm_list;
	uint8 tgt_layer = fsm->layer << 1;
printf("fsm_parent_start\n");	

	if (! sc->layers & tgt_layer)
		tgt_layer <<= 1;
		if (!sc->layers & tgt_layer)
			return;

	for (; mods; mods = mods->fsm_next)
		if (mods->layer == tgt_layer)
			fsm_Open(mods);
}

/* XXX - This can be improved by using the same code and flags 
 * for the various combinations of up/down etc, but
 * this is just a quick trial to make sure it works :)
 */
void fsm_parent_up(struct fsm *fsm)
{
	struct ppp_softc *sc = fsm->sc;
	struct fsm *mods = sc->fsm_list;
	uint8 tgt_layer = fsm->layer << 1;
printf("fsm_parent_up\n");	
	if (! sc->layers & tgt_layer)
		tgt_layer <<= 1;
		if (!sc->layers & tgt_layer)
			return;
	for (; mods; mods = mods->fsm_next)
		if (mods->layer == tgt_layer)
			fsm_Up(mods);
}
	
	
void fsm_Up(struct fsm *fsm)
{
printf("fsm_Up (%s)\n", fsm->name);
	switch (fsm->state) {
		case STM_INITIAL:
			printf("FSM: Using \"%s\" as a transport\n",
			       fsm->sc->if_child->if_name);
			fsm_newstate(fsm, STM_CLOSED);
			break;
		case STM_STARTING:
			printf("STM_STARTING\n");
//			FsmInitRestartCounter(fp, FSM_REQ_TIMER);
			fsm_sconfreq(fsm);
			fsm_newstate(fsm, STM_REQSENT);
			break;
		default:
			printf("%s:%s: Oops, Up at %s\n",
			       fsm->sc->sc_if.if_name, fsm->name, state2nam(fsm->state));
			break;
	}
}

static void fsm_opennow(struct fsm *fsm)
{
//  timer_Stop(&fp->OpenTimer);
	if (fsm->state <= STM_STOPPED) {
		if (fsm->state != STM_STARTING) {
			/*
			 * In practice, we're only here in ST_STOPPED (when delaying the
			 * first config request) or ST_CLOSED (when openmode == 0).
			 *
			 * The ST_STOPPED bit is breaking the RFC already :-(
			 *
			 * According to the RFC (1661) state transition table, a TLS isn't
			 * required for an Open event when state == Closed, but the RFC
			 * must be wrong as TLS hasn't yet been called (since the last TLF)
			 * ie, Initial gets an `Up' event, Closing gets a RTA etc.
			 */
			if (fsm->mod && fsm->mod->Start)
				(*fsm->mod->Start)(fsm);
			fsm_parent_start(fsm);
		}
//		FsmInitRestartCounter(fp, FSM_REQ_TIMER);
		fsm_sconfreq(fsm);
		fsm_newstate(fsm, STM_REQSENT);
  }
}

void fsm_Open(struct fsm *fsm)
{
	printf("fsm_Open %s\n", fsm->name);
	switch (fsm->state) {
		case STM_INITIAL:
			fsm_newstate(fsm, STM_STARTING);
			if (fsm->mod && fsm->mod->Start)
				(*fsm->mod->Start)(fsm);
			fsm_parent_start(fsm);
			break;
		case STM_CLOSED:
			printf("STM_CLOSED\n");
			if (fsm->open_mode == OPEN_PASSIVE) {
				fsm_newstate(fsm, STM_STOPPED);		/* XXX: This is a hack ! */
			} else if (fsm->open_mode > 0) {
				if (fsm->open_mode > 1)
					printf("%s: Entering STOPPED state for %d seconds\n",
					fsm->sc->sc_if.if_name, fsm->open_mode);
				fsm_newstate(fsm, STM_STOPPED);		/* XXX: This is a not-so-bad hack ! */
/*
				timer_Stop(&fp->OpenTimer);
				fp->OpenTimer.load = fp->open_mode * SECTICKS;
				fp->OpenTimer.func = FsmOpenNow;
				fp->OpenTimer.arg = (void *)fp;
				timer_Start(&fp->OpenTimer);
*/
			} else
				fsm_opennow(fsm);
			break;
		case STM_STOPPED:		/* XXX: restart option */
		case STM_REQSENT:
		case STM_ACKRCVD:
		case STM_ACKSENT:
		case STM_OPENED:		/* XXX: restart option */
			break;
		case STM_CLOSING:		/* XXX: restart option */
		case STM_STOPPING:		/* XXX: restart option */
			fsm_newstate(fsm, STM_STOPPING);
			break;
	}
}

void fsm_Down(struct fsm *fsm)
{
	switch (fsm->state) {
		case STM_CLOSED:
			fsm_newstate(fsm, STM_INITIAL);
			break;
		case STM_CLOSING:
			/* This TLF contradicts the RFC (1661), which ``misses it out'' ! */
			if (fsm->mod && fsm->mod->Finish)
				(*fsm->mod->Finish)(fsm);
			fsm_newstate(fsm, STM_INITIAL);
//			(*fp->parent->LayerFinish)(fp->parent->object, fp);
			break;
		case STM_STOPPED:
			fsm_newstate(fsm, STM_STARTING);
			if (fsm->mod && fsm->mod->Start)
				(*fsm->mod->Start)(fsm);
			fsm_parent_start(fsm);
			break;
		case STM_STOPPING:
		case STM_REQSENT:
		case STM_ACKRCVD:
		case STM_ACKSENT:
			fsm_newstate(fsm, STM_STARTING);
			break;
		case STM_OPENED:
			if (fsm->mod && fsm->mod->Down)
				(*fsm->mod->Down)(fsm);
			fsm_newstate(fsm, STM_STARTING);
//			(*fp->parent->LayerDown)(fp->parent->object, fp);
			break;
	}
}

void fsm_Close(struct fsm *fsm)
{
	switch (fsm->state) {
		case STM_STARTING:
			if (fsm->mod && fsm->mod->Finish)
				(*fsm->mod->Finish)(fsm);
			fsm_newstate(fsm, STM_INITIAL);
//			(*fp->parent->LayerFinish)(fp->parent->object, fp);
			break;
		case STM_STOPPED:
			fsm_newstate(fsm, STM_CLOSED);
			break;
		case STM_STOPPING:
			fsm_newstate(fsm, STM_CLOSING);
			break;
		case STM_OPENED:
			if (fsm->mod && fsm->mod->Down)
				(*fsm->mod->Down)(fsm);
			if (fsm->state == STM_OPENED) {
//				FsmInitRestartCounter(fp, FSM_TRM_TIMER);
//				FsmSendTerminateReq(fsm);
				fsm_newstate(fsm, STM_CLOSING);
//				(*fp->parent->LayerDown)(fp->parent->object, fp);
			}
			break;
		case STM_REQSENT:
		case STM_ACKRCVD:
		case STM_ACKSENT:
//			FsmInitRestartCounter(fsm, FSM_TRM_TIMER);
//			FsmSendTerminateReq(fsm);
			fsm_newstate(fsm, STM_CLOSING);
			break;
	}
}

static void fsm_invalid(struct fsm *fsm, struct mbuf *bp)
{
	printf("%s: %s: received an invalid code.\n",
	       fsm->sc->sc_if.if_name, fsm->name);
}

static void fsm_rconfreq(struct fsm *fsm, struct mbuf *bp)
{
	struct fsm_decode dec;
	int ackaction = 0;
	struct fsmheader *lh = mtod(bp, struct fsmheader *);

	/* Check and process easy case */
	switch (fsm->state) {
		case STM_INITIAL:
			/* Drop through */
		case STM_STARTING:
			printf("%s:%s: RCR in %s\n", fsm->sc->sc_if.if_name,
			      fsm->name, state2nam(fsm->state));
			m_freem(bp);
			return;
		case STM_CLOSED:
			if (fsm->mod && fsm->mod->SendTerminateAck)
				(*fsm->mod->SendTerminateAck)(fsm, lh->id);
			m_freem(bp);
			return;
		case STM_CLOSING:
			printf("%s:%s: RCR in %s\n", fsm->sc->sc_if.if_name,
			      fsm->name, state2nam(fsm->state));
		case STM_STOPPING:
			m_freem(bp);
			return;
		case STM_OPENED:
			(*fsm->mod->Down)(fsm);
			break;
	}

	/* XXX - need to sort out what happens when we cross mbuf boundaries */
	bp = m_pullup(bp, lh->length);
	if (!bp) {
		printf("m_pullup failed!???\n");
		return;
	}
	
	dec.ackend = dec.ack;
	dec.nakend = dec.nak;
	dec.rejend = dec.rej;
	if (fsm->mod && fsm->mod->DecodeConfig)
		(*fsm->mod->DecodeConfig)(fsm, bp, &dec);
/*
  if (flen < sizeof(struct fsmconfig))
    log_Printf(fp->LogLevel, "  [EMPTY]\n");
*/
	if (dec.nakend == dec.nak && dec.rejend == dec.rej)
		ackaction = 1;

	switch (fsm->state) {
		case STM_STOPPED:
			/* Start timer ?? */
			/* Fall through */
		case STM_OPENED:
			fsm_sconfreq(fsm);
			break;
	}

	if (dec.rejend != dec.rej)
		fsm_Output(fsm, PCC_CONFIGREJ, lh->id, dec.rej, dec.rejend - dec.rej, bp);
	if (dec.nakend != dec.nak)
		fsm_Output(fsm, PCC_CONFIGNAK, lh->id, dec.nak, dec.nakend - dec.nak, bp);
	if (ackaction)
		fsm_Output(fsm, PCC_CONFIGACK, lh->id, dec.ack, dec.ackend - dec.ack, bp);

	switch (fsm->state) {
		case STM_STOPPED:
			/*
			 * According to the RFC (1661) state transition table, a TLS isn't
			 * required for a RCR when state == ST_STOPPED, but the RFC
			 * must be wrong as TLS hasn't yet been called (since the last TLF)
			 */
			if (fsm->mod && fsm->mod->Start)
				(*fsm->mod->Start)(fsm);
			fsm_parent_start(fsm);
			/* Fall through */
		case STM_OPENED:
			if (ackaction)
				fsm_newstate(fsm, STM_ACKSENT);
			else
				fsm_newstate(fsm, STM_REQSENT);
//			(*fsm->parent->Down)(fp->parent->object, fp);
			break;
		case STM_REQSENT:
			if (ackaction)
				fsm_newstate(fsm, STM_ACKSENT);
			break;
		case STM_ACKRCVD:
			if (ackaction) {
				fsm_newstate(fsm, STM_OPENED);
				if ((*fsm->mod->Up)(fsm)) {
					fsm_parent_up(fsm);
				} else {
					if (fsm->mod && fsm->mod->Down)
						(*fsm->mod->Down)(fsm);
//					FsmInitRestartCounter(fp, FSM_TRM_TIMER);
//					FsmSendTerminateReq(fsm);
					fsm_newstate(fsm, STM_CLOSING);
//					lcp_SendIdentification(&fsm->link->lcp);
				}
			}
			break;
		case STM_ACKSENT:
			if (!ackaction)
				fsm_newstate(fsm, STM_REQSENT);
			break;
	}
	m_freem(bp);

	if (dec.rejend != dec.rej && --fsm->rejs <= 0) {
		printf("%s: Too many %s REJs sent - abandoning negotiation\n",
               fsm->sc->sc_if.if_name, fsm->name);
//		lcp_SendIdentification(&fsm->link->lcp);
		fsm_Close(fsm);
	}

	if (dec.nakend != dec.nak && --fsm->naks <= 0) {
		printf("%s: Too many %s NAKs sent - abandoning negotiation\n",
               fsm->sc->sc_if.if_name, fsm->name);
//		lcp_SendIdentification(&fsm->link->lcp);
		fsm_Close(fsm);
	}
}


static void fsm_rconfack(struct fsm *fsm, struct mbuf *bp)
/* RCA */
{
	struct fsmheader *lh = mtod(bp, struct fsmheader *);
	printf("rcv ConfAck\n");
	switch (fsm->state) {
		case STM_CLOSED:
		case STM_STOPPED:
			if (fsm->mod && fsm->mod->SendTerminateAck)
				(*fsm->mod->SendTerminateAck)(fsm, lh->id);
			break;
		case STM_CLOSING:
		case STM_STOPPING:
			break;
		case STM_REQSENT:
//			FsmInitRestartCounter(fp, FSM_REQ_TIMER);
			fsm_newstate(fsm, STM_ACKRCVD);
			break;
		case STM_ACKRCVD:
			fsm_sconfreq(fsm);
			fsm_newstate(fsm, STM_REQSENT);
			break;
		case STM_ACKSENT:
//			FsmInitRestartCounter(fp, FSM_REQ_TIMER);
			fsm_newstate(fsm, STM_OPENED);
			/* ??? - risky... */
			if ((*fsm->mod->Up)(fsm)) {
				fsm_parent_up(fsm);
			} else {
				if (fsm->mod && fsm->mod->Down)
					(*fsm->mod->Down)(fsm);
//				FsmInitRestartCounter(fp, FSM_TRM_TIMER);
//				FsmSendTerminateReq(fsm);
				fsm_newstate(fsm, STM_CLOSING);
//				lcp_SendIdentification(&fp->link->lcp);
			}
			break;
		case STM_OPENED:
			if (fsm->mod && fsm->mod->Down)
				(*fsm->mod->Down)(fsm);
//			FsmSendConfigReq(fsm);
			fsm_newstate(fsm, STM_REQSENT);
//			(*fp->parent->LayerDown)(fp->parent->object, fp);
			break;
	}
	m_freem(bp);
}

void fsm_Init(struct fsm *fsm)
{
	/* Set initial default values into a new fsm */
	/* XXX - No idea if these are right and should be configurable */
	fsm->reqs = DEF_FSMTRIES;
	fsm->naks = DEF_FSMTRIES;
	fsm->rejs = DEF_FSMTRIES;
	fsm->state = STM_INITIAL;
	fsm->maxtries = DEF_FSMTRIES;
	fsm->restart = 1;
}
	
typedef void (recvfn)(struct fsm *, struct mbuf *);
static const struct fsm_codeopts {
	recvfn *recv;
	unsigned check_reqid : 1;
	unsigned inc_reqid : 1;
	const char *name;
} fsm_functions[] = {
	{ fsm_invalid, 0, 0 }, 
	{ fsm_rconfreq, 0, 0 },
	{ fsm_rconfack, 1, 1 }
};
/*
  { FsmRecvConfigNak, 1, 1, "ConfigNak"    },
  { FsmRecvConfigRej, 1, 1, "ConfigRej"    },
  { FsmRecvTermReq,   0, 0, "TerminateReq" },
  { FsmRecvTermAck,   1, 1, "TerminateAck" },
  { FsmRecvCodeRej,   0, 0, "CodeRej"      },
  { FsmRecvProtoRej,  0, 0, "ProtocolRej"  },
  { FsmRecvEchoReq,   0, 0, "EchoRequest"  },
  { FsmRecvEchoRep,   0, 0, "EchoReply"    },
  { FsmRecvDiscReq,   0, 0, "DiscardReq"   },
  { FsmRecvIdent,     0, 1, "Ident"        },
  { FsmRecvTimeRemain,0, 0, "TimeRemain"   },
  { FsmRecvResetReq,  0, 0, "ResetReq"     },
  { FsmRecvResetAck,  0, 1, "ResetAck"     }
};
*/

void fsm_Input(struct fsm *fsm, struct mbuf *bp)
{
	struct fsmheader *lh;
	int len = 0;
	struct mbuf *mp = bp;
	struct fsm_codeopts codep;
	
	/* Get the total length */
	for (; mp ; mp = mp->m_next) 
		len += mp->m_len;
	
	/* make sure we have a contiguous piece of data */
	if (bp->m_len < sizeof(struct fsmheader)) {
		if ((bp = m_pullup(bp, sizeof(struct fsmheader))) == NULL) {
			m_freem(bp);
			return;
		}
	}
	
	lh = mtod(bp, struct fsmheader *);
	codep = fsm_functions[lh->code];
	lh->length = ntohs(lh->length);
	if (lh->length > len) {
		m_freem(bp);
		return;
	}

	if (lh->code < fsm->min_code || lh->code > fsm->max_code ||
	    lh->code > MAX_FSM_CODE) {
		/* Reject the packet as it's a code we don't know.
		 * Use a private id.  This is really a response-type packet, but we
		 * MUST send a unique id for each REQ....
		 */
		static u_char id;
		fsm_Output(fsm, PCC_CODEREJ, id++, NULL, 0, bp);
    	return;
  	}

	/* Check we have a valid ID passed */
	if (lh->id != fsm->reqid) {
		/* not fatal until we get rest of code in place. */
		printf("fsm_input: id wasn't what we expected...\n");
	}		

	if (codep.inc_reqid && lh->id == fsm->reqid)
		fsm->reqid++;	
    /* That's the end of that ``exchange''.... */

	(*codep.recv)(fsm, bp);
}
