/*
 * ppp_device_module.h
 *
 * Structures we need for a PPP device module
 */
 
#ifndef _PPP_PPP_DEVICE_MODULE_H_
#define _PPP_PPP_DEVICE_MODULE_H_

struct ppp_dev_match {
	struct ppp_dev_match *next;
	char *match;
	int mlen;
	struct ppp_device_module_info *mod;
};

struct ppp_device_module_info {
	char *name;
#ifndef _KERNEL_
	void (*set_core)(struct core_module_info *);
	void (*set_ppp)(struct ppp_module_info*);
#endif
	void (*add_matches)(struct ppp_dev_match **, struct ppp_device_module_info *);
	struct ifnet * (*create_device)(char *, int);
	void (*attach)(struct ifnet *, struct ppp_softc *);
	void (*attach_fsm)(struct ifnet *, struct fsm *);
	int (*start)(struct ifnet *);
	int (*stop)(struct ifnet *);
	int (*destroy_device)(struct ifnet *);
};


#endif  /* _PPP_PPP_DEVICE_MODULE_H_ */

