/* ipv4_module.h
 * definitions for ipv4 protocol
 */
 
#ifndef IPV4_MODULE_H
#define IPV4_MODULE_H

#ifdef _KERNEL_MODE
#include <KernelExport.h>
#include <module.h>
#define IPV4_MODULE_PATH	"network/protocol/ipv4"

#else /* _KERNEL_MODE */

#define IPV4_MODULE_PATH	"modules/protocol/ipv4"

#endif /* _KERNEL_MODE */

struct ipv4_module_info {
#ifdef _KERNEL_MODE
	module_info info;
#endif
	int (*output)(struct mbuf *, 
	                 struct mbuf *, 
	                 struct route *,
	                 int, void *);
	uint16 (*ip_id)(void);
	int (*ctloutput)(int,
	                 struct socket *,
                     int, int,
                     struct mbuf **);
	struct mbuf *(*ip_srcroute)(void);
	void (*ip_stripoptions)(struct mbuf *, 
	                        struct mbuf *);

};

#endif /* IPV4_MODULE_H */
