
#include "ppp_device.h"

/* Some defines, specific to FSM :) */
#define DEF_FSMTRIES   5      /* By default we try 5 times... */

struct fsm_decode {
  u_char ack[100], *ackend;
  u_char nak[100], *nakend;
  u_char rej[100], *rejend;
};

/* Finite State Machine
 *
 * Basically a single structure we use to keep track of all
 * the modules we use, their state and to control their
 * configuration.
 */
struct fsm {
	uint state;
	const char *name;		/* Name of protocol */
	uint16 proto;           /* Protocol number */
	uint8  layer;           /* which layer are we? */
	uint16 min_code;        /* >= */
	uint16 max_code;        /* <= */
	uint8 reqid;			/* Next request id */
	int8 restart;			/* Do we restart upon failure? 1 = yes, 0 = no */
	int8 open_mode;         /* how do we open? */
	int8 failedmagic;       /* # of times we fail to negotiate a magic number */
	int8 maxtries;           /* Max # of tries we make before giving up */
	
	void *allowed;          /* pointer to protocol specific allowable options */
	void *our_req;
	void *his_req;
	
	int reqs;             /* No. of times we will tx a conf request */
	int naks;               /* How many NAK's before a close */
	int rejs;               /* Rejections until we close() */
	
	int LogLevel;

	struct ppp_module *mod;
	struct ppp_softc *sc;
	struct fsm *fsm_next;    /* pointer to next structure */
};

struct fsmheader {
  uint8  code;			/* Request code */
  uint8  id;			/* Identification */
  uint16 length;		/* Length of packet */
};

/* Functions exported from the core PPP module */
void fsm_Init(struct fsm *);
void fsm_Up(struct fsm *);
void fsm_Open(struct fsm *);
void fsm_Down(struct fsm *);
void fsm_Close(struct fsm *);
void fsm_Input(struct fsm *fsm, struct mbuf *bp);
void fsm_Output(struct fsm *, uint, uint8, u_char *, int, struct mbuf *);

/*
extern void fsm_Init(struct fsm *, const char *, u_short, int, int, int,
                     struct bundle *, struct link *, const  struct fsm_parent *,
                     struct fsm_callbacks *, const char * const [3]);
extern void fsm_Input(struct fsm *, struct mbuf *);
extern int fsm_NullRecvResetReq(struct fsm *);
extern void fsm_NullRecvResetAck(struct fsm *, u_char);
extern void fsm_Reopen(struct fsm *);
extern void fsm2initial(struct fsm *);
extern const char *State2Nam(u_int);
*/