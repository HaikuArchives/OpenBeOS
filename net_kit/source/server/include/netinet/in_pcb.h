/* in_pcb.h
 * internet protcol control blocks
 */

#include "sys/socketvar.h"
#include "sys/socket.h"
#include "net_misc.h"
#include "pools.h"
#include "ipv4/ipv4.h"
#include "netinet/in.h"

#ifndef IN_PCB_H
#define IN_PCB_H

enum {
	INP_HDRINCL	= 0x01,
	INP_RECVOPTS	= 0x02,
	INP_RECVRETOPTS = 0x04,
	INP_RECVSTADDR	= 0x08	/* receive IP destination as control inf. */
};
#define INP_CONTROLOPT (INP_RECVOPTS | INP_RECVRETOPTS | INP_RECVDSTADDR);

struct inpcb {
	struct inpcb *inp_next;
	struct inpcb *inp_prev;
	struct inpcb *inp_head;

	struct in_addr faddr;	/* foreign address */
	uint16  fport;		/* foreign port # */	
	struct in_addr laddr;	/* local address */
	uint16  lport;		/* local port # */

	struct socket *inp_socket;

	ipv4_header inp_ip;	/* header prototype */	
	int inp_flags;		/* flags */
	/* more will be required */
};

int      in_pcballoc (struct socket *, struct inpcb *head);
int      in_pcbbind (struct inpcb *, struct mbuf *);
int      in_pcbconnect (void *, struct mbuf *);
void     in_pcbdetach (struct inpcb *);

/* helpful macro's */
#define     sotoinpcb(so)   ((struct inpcb *)(so)->so_pcb)

#endif /* IN_PCB_H */

