/* net_module.h
 * fundtions and staructures for all modules using the
 * net_server
 */

#include <kernel/OS.h>
#include <image.h>

#include "mbuf.h"
#include "if.h"
#include "net_misc.h"
#include "net/route.h"
#include "sys/socketvar.h"

#ifndef OBOS_NET_MODULE_H
#define OBOS_NET_MODULE_H

#ifdef _KERNEL_MODE
#include <module.h>
#endif

typedef struct loaded_net_module 	loaded_net_module;

struct device_info {
	char *name;
	int (*init)(void);
};

struct protocol_info {
	char *name;
	void (*init)(void);
};

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
	int 	(*dev_init) (ifnet *);
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


struct protosw *pffindproto(int domain, int protocol, int type);
struct protosw *pffindtype(int domain, int type);
void add_protosw(struct protosw *[], int layer);
struct in_ifaddr *get_primary_addr(void);

#endif /* OBOS_NET_MODULE_H */

