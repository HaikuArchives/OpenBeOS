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

typedef struct loaded_net_module 	loaded_net_module;

/* each net_module MUST define eaxctaly ONE of these, completely 
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

struct loaded_net_module {
	struct loaded_net_module *next;
        struct net_module *mod;
        image_id iid;
        int32 ref_count;
};

enum {
	NET_LAYER1	= 1, /* link layer */
	NET_LAYER2,	/* network layer */
	NET_LAYER3,	/* transport layer */
	NET_LAYER4	/* socket layer */
};

/*
 * Defines for the userreq function req field are below.
 * 
 *      (*usrreq)(up, req, m, nam, opt);
 *
 * up is a (struct socket *)
 * req is one of these requests,
 * m is a optional mbuf chain containing a message,
 * nam is an optional mbuf chain containing an address,
 * opt is a pointer to a socketopt structure or nil.
 *
 * The protocol is responsible for disposal of the mbuf chain m,
 * the caller is responsible for any space held by nam and opt.
 * A non-zero return from usrreq gives an
 * UNIX error number which should be passed to higher level software.
 */
#define PRU_ATTACH              0       /* attach protocol to up */
#define PRU_DETACH              1       /* detach protocol from up */
#define PRU_BIND                2       /* bind socket to address */
#define PRU_LISTEN              3       /* listen for connection */
#define PRU_CONNECT             4       /* establish connection to peer */
#define PRU_ACCEPT              5       /* accept connection from peer */
#define PRU_DISCONNECT          6       /* disconnect from peer */
#define PRU_SHUTDOWN            7       /* won't send any more data */
#define PRU_RCVD                8       /* have taken data; more room now */
#define PRU_SEND                9       /* send this data */
#define PRU_ABORT               10      /* abort (fast DISCONNECT, DETATCH) */
#define PRU_CONTROL             11      /* control operations on protocol */
#define PRU_SENSE               12      /* return status into m */
#define PRU_RCVOOB              13      /* retrieve out of band data */
#define PRU_SENDOOB             14      /* send out of band data */
#define PRU_SOCKADDR            15      /* fetch socket's address */
#define PRU_PEERADDR            16      /* fetch peer's address */
#define PRU_CONNECT2            17      /* connect two sockets */
/* begin for protocols internal use */
#define PRU_FASTTIMO            18      /* 200ms timeout */
#define PRU_SLOWTIMO            19      /* 500ms timeout */
#define PRU_PROTORCV            20      /* receive from below */
#define PRU_PROTOSEND           21      /* send to below */
#define PRU_PEEREID             22      /* get local peer eid */

#define PRU_NREQ                22

net_module *pffindproto(int domain, int protocol, int type);
net_module *pffindtype(int domain, int type);

#endif /* OBOS_NET_MODULE_H */

