/* inpcb.c
 *
 * implementation of internet control blocks code
 */

#include <stdio.h>

#include "pools.h"
#include "mbuf.h"
#include "sys/socketvar.h"
#include "netinet/in_pcb.h"

static struct pool_ctl *pcbpool;

int inpcb_init(void)
{
	if (!pcbpool)
		pool_init(&pcbpool, sizeof(struct inpcb));

	if (!pcbpool) {
		printf("Doh! Failed to create an inpcb pool!\n");
		return -1;
	}

	return 0;
}

int in_pcballoc(struct socket *so, struct inpcb *head)
{
	struct inpcb *inp;

	inp = (struct inpcb *)pool_get(pcbpool);

	if (!inp)
		return B_NO_MEMORY;

	memset(inp, 0, sizeof(*inp));

	inp->inp_head = head;
	/* insert new pcb at head of queue */
	inp->inp_next = head;
	if (head) {
		inp->inp_prev = head->inp_prev;
	} else {
		inp->inp_prev = head;
	}
	/* associate ourselves with the socket */
	inp->inp_socket = so;
	so->so_pcb = (caddr_t)inp;
	return 0;
}

void in_pcbdetach(struct inpcb *inp) 
{
	struct socket *so = inp->inp_socket;

	so->so_pcb = 0;
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

		if (sin->sin_family != AF_INET)
			return EAFNOSUPPORT;

		lport = sin->sin_port;

		printf("trying to bind to port %d\n", lport);
		/* XXX - finish me! */
	}
	return 0;
}

