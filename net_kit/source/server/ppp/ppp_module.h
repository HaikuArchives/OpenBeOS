#ifndef PPP_MODULE_H
#define PPP_MODULE_H

#include "net_module.h"

#ifdef _KERNEL_
#include <KernelExport.h>
#define PPP_MODULE_PATH	"network/interface/ppp"
#else /* _KERNEL_ */
#define PPP_MODULE_PATH	"modules/interface/ppp"
#endif /* _KERNEL_ */

struct ppp_module_info {
	struct kernel_net_module_info info;
#ifndef _KERNEL_
	void (*set_core)(struct core_module_info *);
#endif

	struct ifnet *(*connection)(void);
};

#endif /* IPV4_MODULE_H */
