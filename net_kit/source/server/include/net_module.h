/* net_module.h
 * fundtions and staructures for all modules using the
 * net_server
 */

#ifndef OBOS_NET_MODULE_H
#define OBOS_NET_MODULE_H

#include <kernel/OS.h>
#include <image.h>

#include "sys/mbuf.h"
#include "net_misc.h"
#include "sys/socketvar.h"
#include "net/if.h"
#include "net/route.h"

#ifdef _KERNEL_MODE
#include <module.h>

#endif

struct device_info {
	char *name;
	int (*init)(struct core_module_info *);
};

struct protocol_info {
	char *name;
	void (*init)(struct core_module_info *);
};

typedef struct loaded_net_module   loaded_net_module;

/* each net_module MUST define exactly ONE of these, completely 
 * filled in, and it MUST be called net_module_data
 */
typedef struct net_module {
	char *name;
	int proto;
	int layer;
	int domain;	/* AF_INET and so on */
	int sock_type;	/* SOCK_STREAM or SOCK_DGRAM at present */
	int flags;

	int 	(*init) (loaded_net_module *, int *pt);
	int 	(*dev_init) (struct ifnet *);
	int 	(*input) (struct mbuf *, int);
	int 	(*output) (struct mbuf *, struct mbuf *, struct route *, int, void *);
	int     (*resolve) (struct mbuf *, struct rtentry *, struct sockaddr *, void *,
				void (*callback)(int, struct mbuf *));
	int	(*userreq) (struct socket *, int, struct mbuf*, struct mbuf*,
			    struct mbuf*);
} net_module;

#ifndef _KERNEL_MODE

struct loaded_net_module {
	struct loaded_net_module *next;
	struct net_module *mod;
	image_id iid;
	int32 ref_count;
};

#else

struct loaded_net_module {
	struct loaded_net_module *next;
	struct net_module *mod;
	module_info *ptr;
	char *path;
};

#endif /* _KERNEL_MODE */

enum {
	NET_LAYER1	= 1, /* link layer */
	NET_LAYER2,	/* network layer */
	NET_LAYER3,	/* transport layer */
	NET_LAYER4	/* socket layer */
};

#endif /* OBOS_NET_MODULE_H */

