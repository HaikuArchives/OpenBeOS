/* raw_module.h
 */
 
#ifndef RAW_MODULE_H
#define RAW_MODULE_H

#ifdef _KERNEL_MODE

#include <KernelExport.h>
#include <module.h>
#define RAW_MODULE_PATH	"network/protocol/raw"

#else

#define RAW_MODULE_PATH              "modules/protocols/raw"

#endif

struct raw_module_info {
#ifdef _KERNEL_MODE
	module_info info;
#endif
	void (*input)(struct mbuf *, int);
};

#endif /* RAW_MODULE_H */
