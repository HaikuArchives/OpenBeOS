/* arp_module.h */

#ifndef ARP_MODULE_H
#define ARP_MODULE_H

#ifdef _KERNEL_MODE
#include <module.h>
#define ARP_MODULE_PATH		"network/misc/arp"

#else

#define ARP_MODULE_PATH		"modules/misc/arp"
#endif

#include "net/route.h"
#include "sys/socket.h"

struct arp_module_info {

#ifdef _KERNEL_MODE
	module_info module;
#else
	int (*init)(void);
#endif

	int (*input)(struct mbuf *, int);
	int (*resolve)(struct mbuf *, struct rtentry *,
		struct sockaddr *, void *,
		void (*callback)(int, struct mbuf *));
};

#endif /* ARP_MODULE_H */
