#ifndef _PPP_IPCP_H
#define _PPP_IPCP_H_

#ifdef _KERNEL_
#define IPCP_MODULE_PATH "network/ppp/protocols/ipcp"
#else
#define IPCP_MODULE_PATH "modules/ppp/protocols/ipcp"
#define std_ops NULL
#endif

/* IPCP Types */
#define IPCP_ADDRESSES     1
#define IPCP_COMPPROT      2
#define IPCP_ADDRESS       3
#define IPCP_MOBILEIP4     4
#define IPCP_PRIDNS       81
#define IPCP_PRINBNS      82
#define IPCP_SECDNS       83
#define IPCP_SECNBNS      84

#define IPCP_MAX_CODE      7
#define MAX_IPCP_OPT_LEN  20
#define IPCP_NAME "IPv4 Control Protocol"

struct ipcpheader {
  uint8  code;          /* Request code */
  uint8  id;            /* Identification */
  uint16 length;        /* Length of packet */
};

struct ipcp_options {
	uint32 address;
	uint16 compression;
	uint8 max_slot;
	uint8 comp_slot;

	uint want_addr:1;
	uint want_comp:1;	
};

struct ipcp_opt {
	uint8  type;
	uint8  len;
	union {
		struct {
			uint16 prot;
			uint8  data[MAX_IPCP_OPT_LEN - 4];
		} comp;
		uint8 dat[MAX_IPCP_OPT_LEN - 2];
	} d;
};
#define comp_prot d.comp.prot
#define comp_data d.comp.data
#define ipcp_data d.dat

#endif