#ifndef _PPP_LCP_LCP_H
#define _PPP_LCP_LCP_H_

#ifdef _KERNEL_
#define LCP_MODULE_PATH "network/ppp/protocols/lcp"
#else
#define LCP_MODULE_PATH "modules/ppp/protocols/lcp"
#define std_ops NULL
#endif

#define LCP_NAME "Line Control Protocol"

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

struct lcp_options {
	uint32 magic;               /* magic */
	uint32 accmap;              /* Initial ACCMAP value */
	uint32 lqrperiod;           /* LQR frequency (seconds) */

	uint16 mru;                 /* Preferred MRU value */
	uint16 max_mru;             /* Preferred MRU value */
	uint16 mtu;                 /* Preferred MTU */
	uint16 max_mtu;             /* Preferred MTU */


	unsigned want_magic: 1;     /* do we want/allow a magic setting */
	unsigned acfcomp : 1;       /* Address & Control Field Compression neg */
	unsigned protocomp : 1;     /* Protocol field compression */
	unsigned callback: 1;       /* callback ? */
	unsigned lqr : 1;           /* Link Quality Report */
	unsigned pap : 1;           /* Password Authentication protocol */
	unsigned chap05 : 1;        /* Challenge Handshake Authentication proto */
#ifdef HAVE_DES
	unsigned chap80nt : 1;      /* Microsoft (NT) CHAP */
	unsigned chap80lm : 1;      /* Microsoft (LANMan) CHAP */
	unsigned chap81 : 1;        /* Microsoft CHAP v2 */
#endif
	char ident[PPP_MTU - 7];	/* SendIdentification() data */
};

#define	LCP_MAXCODE     PCC_IDENT

#define MAX_LCP_OPT_LEN 20
struct lcp_opt {
  uint8 id;
  uint8 len;
  uchar data[MAX_LCP_OPT_LEN-2];
};

void lcp_init_fsm(struct fsm *fsm);
//void lcp_init(struct lcp *lcp, struct ifnet *ifp);
//void lcp_input(struct mbuf *bp, struct lcp *lcp);

#endif
