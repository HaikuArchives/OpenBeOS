/* core_module.h
 * definitions needed by the core networking module
 */

#ifndef OBOS_CORE_MODULE_H
#define OBOS_CORE_MODULE_H

#include <module.h>
#include "mbuf.h"
#include "sys/protosw.h"
#include "sys/socketvar.h"
#include "netinet/in_pcb.h"

struct core_module_info {
	module_info	module;

	int (*start)(void);
	int (*stop)(void);
	void (*add_domain)(struct domain *, int);
	void (*add_protocol)(struct protosw *, int);
	void (*add_protosw)(struct protosw *prt[], int layer);
	
	/* Add the required functions here... */
	int (*soo_ioctl)(void *, int, caddr_t);

	/* socket functions... */
	int (*initsocket)(void **);
	int (*socreate)(int, void *, int, int);
	int (*soclose)(void *);
	int (*sobind)(void *, struct mbuf *);
	int (*soreserve)(struct socket *, uint32, uint32);
	int (*sbappendaddr)(struct sockbuf *, struct sockaddr *, 
		 				struct mbuf *, struct mbuf *);
	void (*sowakeup)(struct socket *, struct sockbuf *);
		 
	/* pcb options */
	int (*in_pcballoc)(struct socket *, struct inpcb *);
	void (*in_pcbdetach)(struct inpcb *); 
	int (*in_pcbbind)(struct inpcb *, struct mbuf *);
	int (*in_pcbconnect)(struct inpcb *, struct mbuf *);
	int (*in_pcbdisconnect)(struct inpcb *);
	struct inpcb * (*in_pcblookup)(struct inpcb *, 
		struct in_addr, uint16, struct in_addr, uint16, int);	
	
	/* mbuf routines... */
	struct mbuf * (*m_free)(struct mbuf *);
	void (*m_freem)(struct mbuf *);
	struct mbuf * (*m_gethdr)(int);
	void (*m_adj)(struct mbuf*, int);
	struct mbuf * (*m_prepend)(struct mbuf*, int);

	/* module control routines... */
	void (*add_device)(ifnet *);

	/* routing */
	void (*rtalloc)(struct route *ro);
	struct rtentry *(*rtalloc1)(struct sockaddr *, int);
	void (*rtfree)(struct rtentry *);

	/* ifnet functions */
	struct ifaddr *(*ifa_ifwithdstaddr)(struct sockaddr *addr);
	struct ifaddr *(*ifa_ifwithnet)(struct sockaddr *addr);
	
	struct in_ifaddr * (*get_primary_addr)(void);	
};

#ifdef _KERNEL_MODE
#define CORE_MODULE_PATH	"network/core"
#endif

#endif /* OBOS_CORE_MODULE_H */

