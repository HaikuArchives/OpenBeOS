/* raw_module.h
 */
 
#ifndef RAW_MODULE_H
#define RAW_MODULE_H

#ifdef _KERNEL_MODE

#include <KernelExport.h>
#include <module.h>

struct raw_module_info {
	module_info info;
	int (*input)(struct mbuf *, int);
};

#define RAW_MODULE_PATH	"network/protocol/raw"

#endif /* _KERNEL_MODE */

#endif /* RAW_MODULE_H */
