/* protosw.h */

#ifndef PROTOSW_H
#define PROTOSW_H

/* every protocol module init's one of these and passes it into
 * the add_protocol() function, defined below */
/* NB The pr_domain pointer will be filled in during the
 * add_protocol() call
 */
struct protosw {
	char *name;
	char *mod_path;
	uint16	pr_type;	/* SOCK_xxx */
	struct domain *pr_domain;
	uint16 pr_protocol;	/* protocol number */
	uint16 pr_flags;	/* flags, PR_xxx */
	int layer;			/* which layer are we? */
	
	/* the functions! */
	void (*pr_init)(void);
	int (*pr_input)(struct mbuf*, int);
	int (*pr_output)(struct mbuf *, struct mbuf *, 
					 struct route *,
					 int, void *);
	
	int (*pr_userreq)(struct socket *, int,
					  struct mbuf *, 
					  struct mbuf *, 
					  struct mbuf *);
	
	struct protosw *pr_next;	/* pointer to next proto structure */
	struct protosw *dom_next;	/* next protosw pointed by the domain */
};

struct protosw *protocols;
void add_protocol(struct protosw *pr, int fam);

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

#endif /* PROTOSW_H */