/* ipv4_module.h
 * definitions for ipv4 protocol
 */
 
#ifndef IPV4_MODULE_H
#define IPV4_MODULE_H

#ifdef _KERNEL_MODE

#include <KernelExport.h>
#include <module.h>

struct ipv4_module_info {
	module_info info;
	int (*ip_output)(struct mbuf *, 
	                 struct mbuf *, 
	                 struct route *,
	                 int, void *);
	uint16 (*ip_id)(void);
	int (*ctloutput)(int,
	                 struct socket *,
                     int, int,
                     struct mbuf **);
};

#define IPV4_MODULE_PATH	"network/protocol/ipv4"

#endif /* _KERNEL_MODE */

#endif /* IPV4_MODULE_H */
