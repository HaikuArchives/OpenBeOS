/* loop_module.h */

#ifndef LOOP_MODULE_H
#define LOOP_MODULE_H

#include <module.h>
#include "mbuf.h"
#include "if.h"
#include "net_module.h"

#define LOOP_MODULE_PATH "network/interface/loop"

struct loop_module {
	module_info module;
	
	void (*module_info)(net_module *);

	int (*init)(loaded_net_module *, int *);
	int (*dev_init)(ifnet *);

	int (*input)(struct mbuf *);
};

#endif

