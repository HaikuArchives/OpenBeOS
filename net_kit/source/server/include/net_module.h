/* net_module.h
 * fundtions and staructures for all modules using the
 * net_server
 */

#include <image.h>

#include "mbuf.h"
#include "if.h"
#include "net_misc.h"

#ifndef OBOS_NET_MODULE_H
#define OBOS_NET_MODULE_H

typedef struct loaded_net_module 	loaded_net_module;

/* each net_module MUST define eaxctaly ONE of these, completely 
 * filled in, and it MUST be called net_module_data
 */
typedef struct net_module {
	char *name;
	int proto;
	int layer;

	int 			(*init) (loaded_net_module *, int *pt);
	int 			(*dev_init) (ifnet *);
	int 			(*input) (struct mbuf *);
	int 			(*output) (struct mbuf *, int, ifnet *, 
						struct sockaddr *);
	struct sockaddr* 	(*lookup) (struct sockaddr *, struct sockaddr *);
} net_module;

struct loaded_net_module {
        struct net_module *mod;
        image_id iid;
        int32 ref_count;
};

enum {
	NET_LAYER1	= 1,
	NET_LAYER2,
	NET_LAYER3
};

#endif /* OBOS_NET_MODULE_H */

