/*
 * ppp_device.h
 *
 * Each PPP device is created as both a full blown struct ifnet * that is
 * managed/used by the main stack and as one of these structures. 
 * The structure shown below is private to the ppp code and allows the ppp
 * code to control what's going on.
 *
 * Or that's the theory... :)
 *
 */

#ifndef _PPP_PPP_DEVICE_H_
#define _PPP_PPP_DEVICE_H_

#define PPP_DEVNAME   "ppp"     /* We'll call ourselves pppx :) */

/* The ppp device structure... */
struct ppp_softc {
	struct	ifnet sc_if;        /* network-visible interface */
	uint	sc_flags;           /* control/status bits */
	struct  fsm *fsm_list;
	uint8   layers;             /* bit mask of layers we have to deal with */
		
	uint16  sc_mru;             /* max receive unit */
	pid_t	sc_xfer;            /* used in transferring unit */
	struct	ifq *q;             /* interactive output packet q */
	struct	pppstat sc_stats;	/* count of bytes/pkts sent/rcvd */

	struct ppp_device_module_info *child;
	struct ifnet *if_child;     /* The child interface that handles next level down I/O */
};

/* device flags... */
#define PPP_LOG_INPKT          0x00000001  /* log input packets */
#define PPP_LOG_OUTPKT         0x00000002  /* log output packets */


#endif /* _PPP_PPP_DEVICE_H_ */
