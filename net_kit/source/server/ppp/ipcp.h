#ifndef _PPP_IPCP_H
#define _PPP_IPCP_H_

/* IPCP Types */
#define IPCP_ADDRESSES     1
#define IPCP_COMPPROT      2
#define IPCP_ADDRESS       3

#define IPCP_MAX_CODE      7

struct ipcp_opt {
	uint8  type;
	uint8  len;
	union {
		struct {
			uint16 prot;
			uint8  data[MAX_LCP_OPT_LEN - 4];
		} comp;
		uint8 dat[MAX_LCP_OPT_LEN - 2];
	} d;
};
#define comp_prot d.comp.prot
#define comp_data d.comp.data
#define ipcp_data d.dat

void ipcp_input(struct mbuf *m);

#endif