/* arp_module.h */

#ifndef ARP_MODULE_H
#define ARP_MODULE_H

#include <module.h>

#include "net/route.h"
#include "sys/socket.h"

#define ARP_MODULE_PATH		"network/protocol/arp"

struct arp_module_info {
	module_info module;

	int (*input)(struct mbuf *, int);
	int (*resolve)(struct mbuf *, struct rtentry *,
		struct sockaddr *, void *,
		void (*callback)(int, struct mbuf *));
};

#endif /* ARP_MODULE_H */
