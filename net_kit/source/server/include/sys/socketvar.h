/* socketvar.h */

#include "mbuf.h"

#ifndef SYS_SOCKETVAR_H
#define SYS_SOCKETVAR_H

struct  sockbuf {
	uint32  sb_cc;          /* actual chars in buffer */
	uint32  sb_hiwat;       /* max actual char count */
	uint32  sb_mbcnt;       /* chars of mbufs used */
	uint32  sb_mbmax;       /* max chars of mbufs to use */
	int32   sb_lowat;       /* low water mark */
	struct  mbuf *sb_mb;    /* the mbuf chain */
	/* XXX - add info for select here ! */
	int16   sb_flags;       /* flags, see below */
	int16   sb_timeo;       /* timeout for read/write */
};

#define SB_MAX          (256*1024)      /* default for max chars in sockbuf */
#define SB_LOCK         0x01            /* lock on data queue */
#define SB_WANT         0x02            /* someone is waiting to lock */
#define SB_WAIT         0x04            /* someone is waiting for data/space */
#define SB_SEL          0x08            /* someone is selecting */
#define SB_ASYNC        0x10            /* ASYNC I/O, need signals */
#define SB_NOINTR       0x40            /* operations not interruptible */
#define SB_KNOTE        0x80            /* kernel note attached */

struct socket {
	uint16 so_type;		/* type of socket */
	uint16 so_options;	/* socket options */
	int16 so_linger;	/* dreaded linger value */
	int16 so_state;		/* socket state */

	struct net_module *so_proto; /* pointer to protocol module */

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

	/* our send/recv buffers */
	struct sockbuf so_snd;
	struct sockbuf so_rcv;

	caddr_t so_pcb;		/* pointer to the control block */

	/* XXX - finish me! */
};

/* helpful defines... */

/* adjust counters in sb reflecting freeing of m */
#define sbfree(sb, m) { \
        (sb)->sb_cc -= (m)->m_len; \
        (sb)->sb_mbcnt -= MSIZE; \
        if ((m)->m_flags & M_EXT) \
                (sb)->sb_mbcnt -= (m)->m_ext.ext_size; \
}

/* Functions */


int     soreserve (struct socket *so, uint32 sndcc, uint32 rcvcc);
int     socreate (int dom, struct socket **aso, int type, int proto);
int     sobind (struct socket *so, struct mbuf *nam);

void    sbrelease (struct sockbuf *sb);
int     sbreserve (struct sockbuf *sb, uint32 cc);
void	sbdrop (struct sockbuf *sb, int len);
void    sbflush (struct sockbuf *sb);



#endif /* SYS_SOCKETVAR_H */
