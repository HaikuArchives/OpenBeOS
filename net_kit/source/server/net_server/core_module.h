/* core_module.h
 * definitions needed by the core networking module
 */

#ifndef OBOS_CORE_MODULE_H
#define OBOS_CORE_MODULE_H

#include <module.h>
#include "mbuf.h"

struct core_module_info {
	module_info	module;

	/* Add the required functions here... */
	int (*soo_ioctl)(void *, int, caddr_t);

	/* socket functions... */
	int (*initsocket)(void **);
	int (*socreate)(int, void *, int, int);
	int (*soclose)(void *);
	int (*sobind)(void *, struct mbuf *);
};

#define CORE_MODULE_PATH	"network/core"

#endif /* OBOS_CORE_MODULE_H */

