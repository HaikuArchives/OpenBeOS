/* socketvar.h */

#ifndef SYS_SOCKETVAR_H
#define SYS_SOCKETVAR_H

struct socket {
	uint16 type;		/* type of socket */
	uint16 so_options;	/* socket options */
	int16 so_linger;	/* dreaded linger value */
	int16 so_state;		/* socket state */

	struct net_module *so_proto; /* pointre to protocol module */

	struct socket *head;
	struct sockaet *so_q0;
	struct sockaet *so_q;

	int16 so_q0len;
	int16 so_qlen;
	int16 so_qlimit;
	int16 so_timeo;
	uint16 so_error;
	pid_t so_pgid;
	uint32 so_oobmark;

	caddr_t so_pcb;		/* pointer to the control block */

	/* XXX - finish me! */
};

#endif /* SYS_SOCKETVAR_H */
