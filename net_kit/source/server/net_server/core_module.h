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
#include "net/if.h"

struct core_module_info {
	module_info	module;

	int (*start)(void);
	int (*stop)(void);
	void (*add_domain)(struct domain *, int);
	void (*add_protocol)(struct protosw *, int);
	void (*add_protosw)(struct protosw *prt[], int layer);
	void (*start_rx_thread)(ifnet *dev);
	void (*start_tx_thread)(ifnet *dev);
	
	/* pool functions */
	status_t (*pool_init)(pool_ctl **p, size_t sz);
	char *(*pool_get)(pool_ctl *p);
	void (*pool_put)(pool_ctl *p, void *ptr);
	void (*pool_destroy)(pool_ctl *p);

	/* socket functions - called "internally" */
	int (*soreserve)(struct socket *, uint32, uint32);
	int (*sbappendaddr)(struct sockbuf *, struct sockaddr *, 
		 				struct mbuf *, struct mbuf *);
	void (*sowakeup)(struct socket *, struct sockbuf *);
	void (*soisconnected)(struct socket *);
	void (*soisdisconnected)(struct socket *);
	void (*socantsendmore)(struct socket *);
	
	/* pcb options */
	int (*in_pcballoc)(struct socket *, struct inpcb *);
	void (*in_pcbdetach)(struct inpcb *); 
	int (*in_pcbbind)(struct inpcb *, struct mbuf *);
	int (*in_pcbconnect)(struct inpcb *, struct mbuf *);
	int (*in_pcbdisconnect)(struct inpcb *);
	struct inpcb * (*in_pcblookup)(struct inpcb *, 
	    struct in_addr, uint16, struct in_addr, uint16, int);
	int (*in_control)(struct socket *, int, caddr_t,
	    struct ifnet *);
	
	/* mbuf routines... */
	struct mbuf * (*m_gethdr)(int);
	void (*m_adj)(struct mbuf*, int);
	struct mbuf * (*m_prepend)(struct mbuf*, int);
	struct mbuf *(*m_pullup)(struct mbuf *, int);
	void (*m_copyback)(struct mbuf *, int, int, caddr_t);
	void (*m_copydata)(struct mbuf *, int, int, caddr_t);
	struct mbuf *(*m_copym)(struct mbuf *, int, int);
	struct mbuf * (*m_free)(struct mbuf *);
	void (*m_freem)(struct mbuf *);


	/* module control routines... */
	void (*add_device)(ifnet *);
	struct ifnet *(*get_interfaces)(void);
	
	/* routing */
	void (*rtalloc)(struct route *ro);
	struct rtentry *(*rtalloc1)(struct sockaddr *, int);
	void (*rtfree)(struct rtentry *);
	int	(*rtrequest)(int, struct sockaddr *,
                     struct sockaddr *, struct sockaddr *, int,
                     struct rtentry **);
	struct radix_node *(*rn_addmask)(void *, int, int);
	struct radix_node *(*rn_head_search)(void *argv_v);
	struct radix_node_head **(*get_rt_tables)(void);
	int	(*rt_setgate)(struct rtentry *, struct sockaddr *,
	                  struct sockaddr *);

	/* ifnet functions */
	struct ifaddr *(*ifa_ifwithdstaddr)(struct sockaddr *addr);
	struct ifaddr *(*ifa_ifwithnet)(struct sockaddr *addr);
	void (*if_attach)(struct ifnet *ifp);
	struct ifaddr *(*ifa_ifwithaddr)(struct sockaddr *);
	struct ifaddr *(*ifa_ifwithroute)(int, struct sockaddr *,
                                       struct sockaddr *);
	struct ifaddr *(*ifaof_ifpforaddr)(struct sockaddr *, struct ifnet *);
	void (*ifafree)(struct ifaddr *);

	struct in_ifaddr *(*get_primary_addr)(void);	

	/* socket functions - used by socket driver */
	int (*initsocket)(void **);
	int (*socreate)(int, void *, int, int);
	int (*soclose)(void *);
	int (*sobind)(void *, caddr_t, int);
	int (*solisten)(void *, int);
	int (*soconnect)(void *, caddr_t, int);
	int (*recvit)(void *, struct msghdr *, caddr_t, int *);
	int (*sendit)(void *, struct msghdr *, int, int *);
	int (*soo_ioctl)(void *, int, caddr_t);
	int (*net_sysctl)(int *, uint, void *, size_t *, void *, size_t);
	int (*writeit)(void *, struct iovec *, int);
	int (*readit)(void*, struct iovec *, int *);
	int (*soselect)(void *, uint8, uint32, void *);
	int (*sosetopt)(void *, int, int, const void *, size_t);
	int (*sogetopt)(void *, int, int, void *, size_t *);

};

#ifdef _KERNEL_MODE
#define CORE_MODULE_PATH	"network/core"
#endif

#endif /* OBOS_CORE_MODULE_H */

