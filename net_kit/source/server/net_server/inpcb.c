/* inpcb.c
 *
 * implementation of internet control blocks code
 */

#include <stdio.h>

#ifdef _KERNEL_MODE
#include <KernelExport.h>
#endif

#include "pools.h"
#include "mbuf.h"
#include "sys/socketvar.h"
#include "netinet/in.h"
#include "netinet/in_pcb.h"
#include "net/if.h"
#include "net_module.h"
#include "netinet/in_var.h"
#include "sys/protosw.h"

static struct pool_ctl *pcbpool = NULL;
static struct in_addr zeroin_addr;

int inpcb_init(void)
{
	in_ifaddr = NULL;
	
	if (!pcbpool)
		pool_init(&pcbpool, sizeof(struct inpcb));

	if (!pcbpool) {
		printf("inpcb_init: ENOMEM\n");
		return ENOMEM;
	}

	zeroin_addr.s_addr = 0;
	return 0;
}

int in_pcballoc(struct socket *so, struct inpcb *head)
{
	struct inpcb *inp;

	inp = (struct inpcb *)pool_get(pcbpool);

	if (!inp) {
		printf("in_pcballoc: ENOMEM\n");
		return ENOMEM;
	}

	memset(inp, 0, sizeof(*inp));

	inp->inp_head = head;
	/* insert new pcb at head of queue */
	inp->inp_prev = head->inp_next->inp_prev;
	inp->inp_next = head->inp_next;
	head->inp_next->inp_prev = inp;
	head->inp_next = inp;

	/* associate ourselves with the socket */
	inp->inp_socket = so;
	so->so_pcb = (caddr_t)inp;
	return 0;
}

void in_pcbdetach(struct inpcb *inp) 
{
	struct socket *so = inp->inp_socket;

	so->so_pcb = NULL;
	/* BSD sockets would call sofree here - we can't.
	 * The first thing that sofree does in BSD is check whether
	 * there are still file system references to the socket
	 * (SS_NOFDREF) and if there are it doesn't free. We don't have
	 * the same relationship to our sockets, as we use the socket for
	 * the kernel cookie, and freeing it here would lead to real problems,
	 * so we leave the socket until we call soclose()
	 * This may need to be reviewed and an extra layer of abstraction
	 * added at some point if we find it's using too much system resource.
	 */

	if (inp->inp_options)
		m_free(inp->inp_options);
	if (inp->inp_route.ro_rt)
		rtfree(inp->inp_route.ro_rt);
	
	inp->inp_prev->inp_next = inp->inp_next;
	inp->inp_next->inp_prev = inp->inp_prev;

	pool_put(pcbpool, inp);
}

int in_pcbbind(struct inpcb *inp, struct mbuf *nam)
{
	struct socket *so = inp->inp_socket;
	struct inpcb *head = inp->inp_head;
	struct sockaddr_in *sin;
	uint16 lport = 0;
	int wild = 0;
	int reuseport = (so->so_options & SO_REUSEPORT);

	if (inp->lport || inp->laddr.s_addr != INADDR_ANY) {
		printf("in_pcbbind: EINVAL\n");
		return EINVAL;
	}

	/* XXX - yuck! Try to format this better */
	/* This basically checks all the options that might be set that allow
	 * us to use wildcard searches.
	 */
	if (((so->so_options & (SO_REUSEADDR | SO_REUSEPORT)) == 0) &&
	    ((so->so_proto->pr_flags & PR_CONNREQUIRED) == 0 ||
	    (so->so_options & SO_ACCEPTCONN) == 0))
		wild = INPLOOKUP_WILDCARD;

	if (nam) {
		sin = mtod(nam, struct sockaddr_in *);
		if (nam->m_len != sizeof(*sin)) {
			printf("in_pcbind: EINVAL\n");
			/* whoops, too much data! */
			return EINVAL;
		}

		/* Apparently this may not be correctly 
		 * in older programs, so this may need
		 * to be commented out...
		 */
		if (sin->sin_family != AF_INET) {
			printf("in_pcbbind: EAFNOSUPPORT\n");
			return EAFNOSUPPORT;
		}

		lport = sin->sin_port;

		if (IN_MULTICAST(ntohl(sin->sin_addr.s_addr))) {
			/* need special case for multicast. We'll
			 * allow the complete binding to be duplicated if
			 * SO_REUSEPORT is set, or if we have 
			 * SO_REUSEADDR set and both sockets have
			 * a multicast address bound.
			 * We'll exit with the reuseport variable set
			 * correctly.
			 */
			if (so->so_options & SO_REUSEADDR)
				reuseport = SO_REUSEADDR | SO_REUSEPORT;
		} else if (sin->sin_addr.s_addr != INADDR_ANY) {
			sin->sin_port = 0; /* must be zero for next step */
			if (ifa_ifwithaddr((struct sockaddr*)sin) == NULL) {
				printf("in_pcbbind: EADDRNOTAVAIL\n");
				return EADDRNOTAVAIL;
			}

		}
		if (lport) {
			struct inpcb *t;
			/* we have something to work with... */
			/* XXX - reserved ports have no meaning for us */
			/* XXX - fix me if we ever have multi-user */
			t = in_pcblookup(head, zeroin_addr, 0,
					 sin->sin_addr, lport, wild);
			if (t && (reuseport & t->inp_socket->so_options) == 0) {
				printf("in_pcbbind: EADDRINUSE\n");
				return EADDRINUSE;
			}
		}
		inp->laddr = sin->sin_addr;
	}
	/* if we have an ephemereal port, find a suitable port to use */
	if (lport == 0) {
		/* ephemereal port!! */
		do {
			if (head->lport++ < IPPORT_RESERVED ||
		  	    head->lport > IPPORT_USERRESERVED) {
				head->lport = IPPORT_RESERVED;
			}
			lport = htons(head->lport);
		} while (in_pcblookup(head, zeroin_addr, 0, 
					inp->laddr, lport, wild));
	}

	inp->lport = lport;
	return 0;
}

struct inpcb *in_pcblookup(struct inpcb *head, struct in_addr faddr,
			   uint16 fport_a, struct in_addr laddr,
			   uint16 lport_a, int flags)
{
	struct inpcb *inp;
	struct inpcb *match = NULL;
	int matchwild = 3;
	int wildcard;
	uint16 fport = fport_a;
	uint16 lport = lport_a;

	for (inp = head->inp_next; inp != head; inp = inp->inp_next) {
		if (inp->lport != lport)
			continue; /* local ports don't match */
		wildcard = 0;
		/* Here we try to find the best match. wildcard is set to 0
		 * and bumped by one every time we find something that doesn't match
		 * so we can have a suitable match at the end
		 */
		if (inp->laddr.s_addr != INADDR_ANY) {
			if (laddr.s_addr == INADDR_ANY)
				wildcard++;
			else if (inp->laddr.s_addr != laddr.s_addr)
				continue;
		} else {
			if (laddr.s_addr != INADDR_ANY)
				wildcard++;
		}
		
		if (inp->faddr.s_addr != INADDR_ANY) {
			if (faddr.s_addr == INADDR_ANY)
				wildcard++;
			else if (inp->faddr.s_addr != faddr.s_addr ||
				 inp->fport != fport)
				continue;
		} else {
			if (faddr.s_addr != INADDR_ANY)
				wildcard++;
		}
 		if (wildcard && ((flags & INPLOOKUP_WILDCARD) == 0)) {
			continue; /* wildcard match is not allowed!! */
		}
		
		if (wildcard < matchwild) {
			match = inp;
			matchwild = wildcard;
			if (matchwild == 0)
				break; /* exact match!! */
		}
	}
	return match;
}

/* XXX - This needs a LOT of work! */
int in_pcbconnect(struct inpcb *inp, struct mbuf *nam)
{
	struct in_ifaddr *ia = NULL;
	struct sockaddr_in *ifaddr = NULL;
	struct sockaddr_in *sin = mtod(nam, struct sockaddr_in *);

	if (nam->m_len != sizeof(*sin)) {
		printf("in_pcbconnect: EINVAL\n");
		return EINVAL;
	}
	if (sin->sin_family != AF_INET) {
		printf("in_pcbconnect: EAFNOSUPPORT\n");
		return EAFNOSUPPORT;
	}
	if (sin->sin_port == 0) {
		printf("in_pcbconnect: EADDRNOTAVAIL\n");
		return EADDRNOTAVAIL;
	}
	
	if (in_ifaddr) {
		if (sin->sin_addr.s_addr == INADDR_ANY)
			sin->sin_addr = IA_SIN(in_ifaddr)->sin_addr;

		/* we need to handle INADDR_BROADCAST here as well */
		
	}
	
	if (inp->laddr.s_addr == INADDR_ANY) {
		struct route *ro;

		ro = &inp->inp_route;

		if (ro && ro->ro_rt &&
		    (satosin(&ro->ro_dst)->sin_addr.s_addr != sin->sin_addr.s_addr
		    || inp->inp_socket->so_options & SO_DONTROUTE)) {
			RTFREE(ro->ro_rt);
			ro->ro_rt = NULL;
		}
		if ((inp->inp_socket->so_options & SO_DONTROUTE) == 0
		    && (ro->ro_rt == NULL 
			|| ro->ro_rt->rt_ifp == NULL)) {
			/* we don't have a route, try to get one */
			ro->ro_dst.sa_family = AF_INET;
			ro->ro_dst.sa_len = sizeof(struct sockaddr_in);
			((struct sockaddr_in*)&ro->ro_dst)->sin_addr = sin->sin_addr;
			rtalloc(ro);
		}
		/* did we find a route?? */
		if (ro->ro_rt && (ro->ro_rt->rt_ifp->if_flags & IFF_LOOPBACK))
			ia = ifatoia(ro->ro_rt->rt_ifa);

		if (ia == NULL) {
			uint16 fport = sin->sin_port;

			sin->sin_port = 0;
			ia = ifatoia(ifa_ifwithdstaddr(sintosa(sin)));
			if (ia == NULL)
				ia = ifatoia(ifa_ifwithnet(sintosa(sin)));
			sin->sin_port = fport;
			if (ia == NULL)
				ia = in_ifaddr;
			if (ia == NULL) {
				printf("in_pcbconnect: EADDRNOTAVAIL\n");
				return EADDRNOTAVAIL;
			}
		}
		/* XXX - handle multicast */
		ifaddr = (struct sockaddr_in*) &ia->ia_addr;
	}

	if (in_pcblookup(inp->inp_head, sin->sin_addr, sin->sin_port, 
			 inp->laddr.s_addr ? inp->laddr : ifaddr->sin_addr,
			 inp->lport, 0)) {
		printf("in_pcbconnect: EADDRINUSE\n"); 
		return EADDRINUSE;
	}


	if (inp->laddr.s_addr == INADDR_ANY) {
		if (inp->lport == 0)
			in_pcbbind(inp, NULL);
		inp->laddr = ifaddr->sin_addr;
	}
	inp->faddr = sin->sin_addr;
	inp->fport = sin->sin_port;
	return 0;
}

int in_pcbdisconnect(struct inpcb *inp)
{
	inp->faddr.s_addr = INADDR_ANY;
	inp->fport = 0;

	/* BSD sockets are fd's and as such their lifetimes are
	 * controlled by the system. On traditional BSD systems we'd
	 * check to see if we still had references to the socket in the
	 * fd table, but we can't do that. As the socket is the kernel cookie
	 * the fact we're here implies there is still a reference.
	 */
	in_pcbdetach(inp);
	return 0;
}

