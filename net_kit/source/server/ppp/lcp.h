#ifndef _PPP_IPCP_H
#define _PPP_IPCP_H_

/*
 *  Possible states of LCP connection...
 */
#define	ST_INITIAL	0
#define	ST_STARTING	1
#define	ST_CLOSED	2
#define	ST_STOPPED	3
#define	ST_CLOSING	4
#define	ST_STOPPING	5
#define	ST_REQSENT	6
#define	ST_ACKRCVD	7
#define	ST_ACKSENT	8
#define	ST_OPENED	9

#define	ST_MAX		10
#define	ST_UNDEF	-1


#define	MODE_REQ	1
#define	MODE_NAK	3
#define	MODE_REJ	2
#define	MODE_NOP	3
#define	MODE_ACK	2	/* pseudo mode for ccp negotiations */

#define	CODE_CONFIGREQ	1
#define	CODE_CONFIGACK	2
#define	CODE_CONFIGNAK	3
#define	CODE_CONFIGREJ	4
#define	CODE_TERMREQ	5
#define	CODE_TERMACK	6
#define	CODE_CODEREJ	7
#define	CODE_PROTOREJ	8
#define	CODE_ECHOREQ	9	/* Used in LCP */
#define	CODE_ECHOREP	10	/* Used in LCP */
#define	CODE_DISCREQ	11
#define	CODE_IDENT	12	/* Used in LCP Extension */
#define	CODE_TIMEREM	13	/* Used in LCP Extension */
#define	CODE_RESETREQ	14	/* Used in CCP */
#define	CODE_RESETACK	15	/* Used in CCP */

/* LCP Options */
#define	LCPO_MRU         1	/* Maximum-Receive-Unit */
#define	LCPO_ACCMAP      2	/* Async-Control-Character-Map */
#define	LCPO_AUTHPROTO   3	/* Authentication-Protocol */
#define	LCPO_QUALPROTO   4	/* Quality-Protocol */
#define	LCPO_MAGICNUM    5	/* Magic-Number */
#define	LCPO_RESERVED    6	/* RESERVED */
#define	LCPO_PROTOCOMP   7	/* Protocol-Field-Compression */
#define	LCPO_ACFCOMP     8	/* Address-and-Control-Field-Compression */
#define	LCPO_FCSALT      9	/* FCS-Alternatives */
#define	LCPO_SDP        10	/* Self-Describing-Padding */
#define	LCPO_CALLBACK   13	/* Callback */
#define	LCPO_CFRAMES    15	/* Compound-frames */
#define	LCPO_MRRU       17	/* Max Reconstructed Receive Unit (MP) */
#define	LCPO_SHORTSEQ   18	/* Want short seqs (12bit) please (see mp.h) */
#define	LCPO_ENDDISC    19	/* Endpoint discriminator */

/* callback::opmask values */
#define CALLBACK_AUTH		(0)
#define CALLBACK_DIALSTRING	(1)	/* Don't do this */
#define CALLBACK_LOCATION	(2)	/* Don't do this */
#define CALLBACK_E164		(3)
#define CALLBACK_NAME		(4)	/* Don't do this */
#define CALLBACK_CBCP		(6)
#define CALLBACK_NONE		(14)	/* No callback is ok */
#define CALLBACK_BIT(n) ((n) < 0 ? 0 : 1 << (n))

#define MAX_MRU           2048
#define MAX_MTU        MAX_MRU
#define DEF_LQRPERIOD       30

struct lcpheader {
  uint8  code;          /* Request code */
  uint8  id;            /* Identification */
  uint16 length;        /* Length of packet */
};

/* used to store details of how our request did */
struct lcp_decode {
  u_char ack[100], *ackend;
  u_char nak[100], *nakend;
  u_char rej[100], *rejend;
};

struct lcp {
	int open_mode;                  /* Delay before config REQ (-1 forever) */
	int state;                      /* State of the LCP connection */
	uint8 reqid;                    /* Next request id */

	uint16 his_mru;                 /* Peers maximum packet size */
	uint16 his_mrru;                /* Peers maximum reassembled packet size (MP) */
	uint32 his_accmap;              /* Peeers async char control map */
	uint32 his_magic;               /* Peers magic number */
	uint32 his_lqrperiod;           /* Peers LQR frequency (100ths of seconds) */
	uint16 his_auth;                /* Peer wants this type of authentication */
	uint8 his_authtype;             /* Fifth octet of REQ/NAK/REJ */
	unsigned his_shortseq : 1;      /* Peer would like only 12bit seqs (MP) */
	unsigned his_protocomp : 1;     /* Does peer do Protocol field compression */
	unsigned his_acfcomp : 1;       /* Does peer do addr & cntrl fld compression */
	unsigned mru_req : 1;           /* Has the peer requested an MRU */

	uint16 want_mru;                /* Our maximum packet size */
	uint16 want_mrru;               /* Our maximum reassembled packet size (MP) */
	uint32 want_accmap;             /* Our async char control map */
	uint32 want_magic;              /* Our magic number */
	uint32 want_lqrperiod;          /* Our LQR frequency (100ths of seconds) */
	uint16 want_auth;               /* We want this type of authentication */
	uint8 want_authtype;            /* Fifth octet of REQ/NAK/REJ */
	unsigned want_shortseq : 1;     /* I'd like only 12bit seqs (MP) */
	unsigned want_protocomp : 1;    /* Do we do protocol field compression */
	unsigned want_acfcomp : 1;      /* Do we do addr & cntrl fld compression */

	uint32 his_reject;              /* Request codes rejected by peer */
	uint32 my_reject;               /* Request codes I have rejected */

	uint16 auth_iwait;              /* I must authenticate to the peer */
	uint16 auth_ineed;              /* I require that the peer authenticates */

	int LcpFailedMagic;             /* Number of `magic is same' errors */

	struct {
		uint16 mru;                 /* Preferred MRU value */
		uint16 max_mru;             /* Preferred MRU value */
		uint16 mtu;                 /* Preferred MTU */
		uint16 max_mtu;             /* Preferred MTU */
		uint32 accmap;              /* Initial ACCMAP value */
		int openmode;               /* when to start CFG REQs */
		uint32 lqrperiod;           /* LQR frequency (seconds) */
		unsigned acfcomp : 2;       /* Address & Control Field Compression neg */
		unsigned chap05 : 2;        /* Challenge Handshake Authentication proto */
#ifdef HAVE_DES
		unsigned chap80nt : 2;      /* Microsoft (NT) CHAP */
		unsigned chap80lm : 2;      /* Microsoft (LANMan) CHAP */
		unsigned chap81 : 2;        /* Microsoft CHAP v2 */
#endif
		unsigned lqr : 2;           /* Link Quality Report */
		unsigned pap : 2;           /* Password Authentication protocol */
		unsigned protocomp : 2;     /* Protocol field compression */
		char ident[PPP_MTU - 7];	/* SendIdentification() data */
	} cfg;
	
	struct ifnet *ifp;
};

#define	LCP_MAXCODE     CODE_IDENT
#define	LCP_MINMPCODE   CODE_CODEREJ

#define MAX_LCP_OPT_LEN 20
struct lcp_opt {
  u_char id;
  u_char len;
  u_char data[MAX_LCP_OPT_LEN-2];
};

void lcp_init(struct lcp *lcp, struct ifnet *ifp);
void lcp_input(struct mbuf *bp, struct lcp *lcp);

#endif