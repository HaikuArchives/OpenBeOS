/* inpcb.c
 *
 * implementation of internet control blocks code
 */

#include <stdio.h>

#include "pools.h"
#include "mbuf.h"
#include "sys/socketvar.h"
#include "netinet/in.h"
#include "netinet/in_pcb.h"
#include "if.h"
 
static struct pool_ctl *pcbpool;
static struct in_addr zeroin_addr;

/* XXX - Debug hack */
#undef NET_DEBUG
#define NET_DEBUG 1

int inpcb_init(void)
{
	if (!pcbpool)
		pool_init(&pcbpool, sizeof(struct inpcb));

	if (!pcbpool) {
		printf("Doh! Failed to create an inpcb pool!\n");
		return -1;
	}

	zeroin_addr.s_addr = 0;
	return 0;
}

int in_pcballoc(struct socket *so, struct inpcb *head)
{
	struct inpcb *inp;

	inp = (struct inpcb *)pool_get(pcbpool);

	if (!inp)
		return ENOMEM;

	memset(inp, 0, sizeof(*inp));

	inp->inp_head = head;
	/* insert new pcb at head of queue */
	inp->inp_prev = head;
	inp->inp_next = head->inp_next;
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
	/* free socket! */

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

	if (inp->lport || inp->laddr.s_addr != INADDR_ANY)
		return EINVAL;

	if (nam) {
		sin = mtod(nam, struct sockaddr_in *);
		if (nam->m_len != sizeof(*sin))
			/* whoops, too much data! */
			return EINVAL;

		/* Apparently this may not be correctly 
		 * in older programs, so this may need
		 * to be commented out...
		 */
		if (sin->sin_family != AF_INET)
			return EAFNOSUPPORT;

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
			if (! is_address_local(&sin->sin_addr.s_addr, 4))
				return EADDRNOTAVAIL;
		}
		if (lport) {
			struct inpcb *t;
			/* we have something to work with... */
			/* XXX - reserved ports have no meaning for us */
			/* XXX - fix me if we ever have multi-user */
			t = in_pcblookup(head, zeroin_addr, 0,
					 sin->sin_addr, lport, wild);
			if (t && (reuseport & t->inp_socket->so_options) == 0)
				return EADDRINUSE;
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

#ifdef NET_DEBUG
	/* if we're using an ephemereal port we don't set it back into
	 * the sockaddr structure, so this is the onyl way we have of 
	 * checking that we're assigning correctly...
	 */
	printf("bound to port %d\n", htons(lport));
#endif

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
			continue; /* local portsdon't match */
		wildcard = 0;
		if (inp->laddr.s_addr == INADDR_ANY) {
			if (laddr.s_addr == INADDR_ANY)
				wildcard++;
			else if (inp->laddr.s_addr != laddr.s_addr)
				continue;
		} else {
			if (laddr.s_addr != INADDR_ANY)
				wildcard++;
		}
		
		if (inp->faddr.s_addr == INADDR_ANY) {
			if (faddr.s_addr == INADDR_ANY)
				wildcard++;
			else if (inp->faddr.s_addr != faddr.s_addr ||
				 inp->fport != fport)
				continue;
		} else {
			if (faddr.s_addr != INADDR_ANY)
				wildcard++;
		}
 		if (wildcard && (flags & INPLOOKUP_WILDCARD) == 0)
			continue; /* wildcard match is not allowed!! */

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
	struct sockaddr_in ifaddr;
	struct sockaddr_in *sin = mtod(nam, struct sockaddr_in *);

	if (nam->m_len != sizeof(*sin))
		return EINVAL;
	if (sin->sin_family != AF_INET) {
		return EAFNOSUPPORT;
	}
	if (sin->sin_port == 0) {
		return EADDRNOTAVAIL;
	}
	
	/* foreign IP address */
	if (sin->sin_addr.s_addr == INADDR_ANY)
		/* assign primary if address */
		sin->sin_addr.s_addr = *(uint32*)&in_ifaddr->sa_data;
	
	if (inp->laddr.s_addr == INADDR_ANY) {
		/* we're sending from INADDR_ANY, so here we need
		 * to find a suitable interface to use!
		 * XXX - This needs to be reviewed once we have routing in place.
		 */
		ifaddr.sin_addr.s_addr = *(uint32*)&in_ifaddr->sa_data;
		ifaddr.sin_family = in_ifaddr->sa_family;
		ifaddr.sin_len = in_ifaddr->sa_len;
	}

	if (in_pcblookup(inp->inp_head, sin->sin_addr, sin->sin_port, 
			 inp->laddr.s_addr ? inp->laddr : ifaddr.sin_addr,
			 inp->lport, 0)) {
		return EADDRINUSE;
	}


	if (inp->laddr.s_addr == INADDR_ANY) {
		if (inp->lport == 0)
			in_pcbbind(inp, NULL);
		inp->laddr = ifaddr.sin_addr;
	}
	inp->faddr = sin->sin_addr;
	inp->fport = sin->sin_port;
	return 0;
}

