/* 
 * serial_ppp.h
 *
 * the serial_ppp device...
 */
 
#ifndef _PPP_SERIAL_PPP_H
#define _PPP_SERIAL_PPP_H

struct serial_ppp_device {
	struct ifnet ifp;
	char *path;
	thread_id cx;
	struct ppp_softc *ppp;
	struct fsm *fsm;
};

#endif /* _PPP_SERIAL_PPP_H */