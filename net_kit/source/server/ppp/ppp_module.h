#ifndef _PPP_PPP_MODULE_H_
#define _PPP_PPP_MODULE_H_

#include "net_module.h"

#ifdef _KERNEL_
#include <KernelExport.h>
#define PPP_MODULE_PATH	"network/protocol/ppp"
#else 
#define PPP_MODULE_PATH	"modules/protocol/ppp"
#endif

#define PPP_MODULE_LAYER1   0x01   /* physical / LCP */
#define PPP_MODULE_LAYER2   0x02   /* Authentication protocols */
#define PPP_MODULE_LAYER3   0x04   /* network protocols */

struct ppp_module {
	char *name;

	int  (*Up) (struct fsm *);                      /* Layer is now up (tlu) */
	void (*Down) (struct fsm *);                    /* About to come down (tld) */
	void (*Start) (struct fsm *);                   /* Layer about to start up (tls) */
	void (*Finish) (struct fsm *);                  /* Layer now down (tlf) */

	void (*InitRestartCounter) (struct fsm *);      /* Set fsm timer load */
	void (*SendConfigReq) (struct fsm *);           /* Send REQ please */
	void (*Input) (struct fsm *, struct mbuf *);

	void (*DecodeConfig)(struct fsm *, struct mbuf *, struct fsm_decode *);
	
//	void (*SentTerminate) (struct fsm *);           /* Term REQ just sent */
	void (*SendTerminateAck) (struct fsm *, uint8); /* Send Term ACK please */
                                                    /* Deal with incoming data */
//	int (*RecvResetReq) (struct fsm *fp);           /* Reset output */
//	void (*RecvResetAck) (struct fsm *fp, u_char);  /* Reset input */
};

/* List of strings a module matches */
struct ppp_proto_match {
	struct ppp_proto_match *next;
	char *match;
	int mlen;
	uint8 layer;
	struct ppp_module_info *mod;
};

/* Kernel level module definition */
struct ppp_module_info {
	struct kernel_net_module_info info;
#ifndef _KERNEL_
	void (*set_core)(struct core_module_info *);
	void (*set_ppp)(struct ppp_module_info*);
#endif
	void (*add_matches)(struct ppp_proto_match **, 
                        struct ppp_module_info *);
	void (*init_fsm)(struct fsm *);
	
	/* FSM functions */
	void (*fsm_Up)(struct fsm *);
	void (*fsm_Open)(struct fsm*);
	void (*fsm_Down)(struct fsm*);
	void (*fsm_Close)(struct fsm*);
	void (*fsm_Output)(struct fsm *, uint, uint8, uchar *, 
	                   int, struct mbuf *);
};

#ifdef _NETWORK_STACK

#endif

#endif /* _PPP_PPP_MODULE_H_ */
