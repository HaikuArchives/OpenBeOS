/* in_pcb.h
 * internet protcol control blocks
 */

#include "sys/socketvar.h"
#include "sys/socket.h"
#include "net_misc.h"

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

	ipv4_addr faddr;	/* foreign address */
	uint16 fport;		/* foreign port # */	
	ipv4_addr laddr;	/* local address */
	uint16 lport;		/* local port # */

	struct socket *inp_socket;

	int inp_flags;		/* flags */
	/* more will be required */
};

void init_inpcb(void);


#endif /* IN_PCB_H */

