/*
 * ppp_defs.h
 *
 * Definitions for PPP modules
 */


#ifndef _PPP_PPP_DEFS_H_
#define _PPP_PPP_DEFS_H_

/* general defines */
#define PPP_DEVICENAME "ppp"
#define PPP_MTU         1500
#define PPP_MAX_MTU    65535
#define PPP_MRU         1500
#define PPP_MAX_MRU    65535
#define PPP_HDRLEN         8


/* FCS Values */
#define PPP_INITFCS16          0xffff	/* Initial FCS value */
#define PPP_GOODFCS16          0xf0b8	/* Good final FCS value */
#define PPP_INITFCS32      0xffffffff
#define PPP_GOODFCS32      0xdebb20e3

/* Protocol Control Codes */
enum {
	PCC_CONFIGREQ      = 1,           /* config request */
	PCC_CONFIGACK,                    /* config acknowledgement */
	PCC_CONFIGNAK,                    /* config NOT acknowledged */
	PCC_CONFIGREJ,                    /* config rejected */
	PCC_TERMREQ,                      /* terminate request */
	PCC_TERMACK,                      /* terminate acknowledgement */
	PCC_CODEREJ,                      /* reject the code */
	PCC_PROTOREJ,                     /* reject the protocol */
	PCC_ECHOREQ,                      /* ECHO request */
	PCC_ECHOREPLY,                    /* ECHO reply */
	PCC_DISCREQ,                      
	PCC_IDENT,                        /* Identification */
	PCC_TIMEREM,                      /* Time remaining */
	PCC_RESETREQ,
	PCC_RESETACK
};

#define MAX_FSM_CODE   PCC_RESETACK

/* Protocol Configuration Options */
enum {
	PCO_MRU = 1,                      /* MRU */
	PCO_ACCMAP,                       /* Async Control Character Map */
	PCO_AUTHPROTO,                    /* Authentication Protocol */
	PCO_QUALPROTO,                    /* Quality protocol */
	PCO_MAGIC,                        /* Magic Number */
	PCO_RESERVED,                     /* Reserved */
	PCO_PFCOMP,                       /* Protocol Field compression */
	PCO_ACFCOMP,                      /* Address & Control Field Compression */
	PCO_FCSALT,                       /* FCS Aletrnative (RFC1570) */
	PCO_SDP,                          /* Self describing Padding (RFC1570) */
	PCO_CALLBACk = 13,                /* Callback (RFC1570) */
	PCO_CFRAMES = 15,                 /* Compuind Frames (RFC1570) */
	PCO_MRRU = 17,                    /* Max reconstructed receive unit */
	PCO_SHORTSEQ,                     /* Short segments */
	PCO_ENDDISC                       /* Endpoint discriminator */
};

/* FCS Types (RFC 1570) */
#define FCS_NULL                1     /* NULL FCS */
#define FCS_16BIT               2     /* 16-bit (standard) */
#define FCS_32BIT               4     /* 32-bit */

/* Callback Operation values (RFC1570) */
#define CALLBACK_AUTH		    0     /* number determined by user authentication */
#define CALLBACK_DIALSTRING	    1	  /* Dialling string to callback */
#define CALLBACK_LOCATION	    2	  /* Location ID, normally used with user auth to get number */
#define CALLBACK_E164		    3     /* E.164 number */
#define CALLBACK_NAME		    4     /* Distinguished Name ? */

/* State machine states */
enum {
	STM_INITIAL  = 0,
	STM_STARTING,
	STM_CLOSED,
	STM_STOPPED,
	STM_CLOSING,
	STM_STOPPING,
	STM_REQSENT,
	STM_ACKRCVD,
	STM_ACKSENT,
	STM_OPENED
};
#define	STM_MAX		10
#define	STM_UNDEF	-1

#define PPP_MODULE_LAYER1   0x01   /* physical / LCP */
#define PPP_MODULE_LAYER2   0x02   /* Authentication protocols */
#define PPP_MODULE_LAYER3   0x04   /* network protocols */

/* PPP Protocol field values */
#define PPP_IP              0x21	/* Internet Protocol */
#define PPP_XNS             0x25	/* Xerox NS */
#define PPP_AT              0x29	/* AppleTalk Protocol */
#define PPP_IPX             0x2b	/* Internetwork Packet Exchange */
#define PPP_VJC_COMP	    0x2d	/* VJ compressed TCP */
#define PPP_VJC_UNCOMP	    0x2f	/* VJ uncompressed TCP */
#define PPP_IPV6            0x57	/* Internet Protocol Version 6 */
#define PPP_COMP            0xfd	/* compressed packet */

#define PPP_IPCP          0x8021	/* IP Control Protocol */
#define PPP_ATCP          0x8029	/* AppleTalk Control Protocol */
#define PPP_IPXCP         0x802b	/* IPX Control Protocol */
#define PPP_IPV6CP        0x8057	/* IPv6 Control Protocol */
#define PPP_CCP           0x80fd	/* Compression Control Protocol */

#define PPP_LCP           0xc021	/* Link Control Protocol */
#define PPP_PAP           0xc023	/* Password Authentication Protocol */
#define PPP_LQR           0xc025	/* Link Quality Report protocol */
#define PPP_CHAP          0xc223	/* Cryptographic Handshake Auth. Protocol */
#define PPP_CBCP          0xc029	/* Callback Control Protocol */

#define PPP_PROTOCOL(p)	  ((((u_char *)(p))[0] << 8) + ((u_char *)(p))[1])

/* Line Quality reporting */
#define DEF_LQRPERIOD        30

/* Other defines we need */
#define OPEN_PASSIVE -1
#ifdef _NETWORK_STACK
uint16 pppfcs16(uint16 fcs, u_char *cp, int len);
uint32 pppfcs32(uint32 fcs, u_char *cp, int len); /* XXX - not yet implemented!!! */
const char *codename(int code);
const char *protoname(int proto);
const char *state2nam(uint state);
#endif

#endif /* _PPP_PPP_DEFS_H_ */
