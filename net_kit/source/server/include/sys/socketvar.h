/* socketvar.h */


#ifndef SYS_SOCKETVAR_H
#define SYS_SOCKETVAR_H

#include <kernel/OS.h>
#include "mbuf.h"
#include "sys/net_uio.h"
#include "sys/socket.h"

struct  sockbuf {
	uint32  sb_cc;			/* actual chars in buffer */
	uint32  sb_hiwat;		/* max actual char count (high water mark) */
	uint32  sb_mbcnt;		/* chars of mbufs used */
	uint32  sb_mbmax;		/* max chars of mbufs to use */
	int32   sb_lowat;		/* low water mark */
	struct  mbuf *sb_mb;	/* the mbuf chain */
	int16   sb_flags;		/* flags, see below */
	int32   sb_timeo;		/* timeout for read/write */
	sem_id	sb_pop;			/* sem to wait on... */
};

#define SB_MAX          (256*1024)      /* default for max chars in sockbuf */
#define SB_LOCK         0x01            /* lock on data queue */
#define SB_WANT         0x02            /* someone is waiting to lock */
#define SB_WAIT         0x04            /* someone is waiting for data/space */
#define SB_SEL          0x08            /* someone is selecting */
#define SB_ASYNC        0x10            /* ASYNC I/O, need signals */
#define SB_NOINTR       0x40            /* operations not interruptible */
#define SB_KNOTE        0x80            /* kernel note attached */
#define SB_NOTIFY       (SB_WAIT|SB_SEL|SB_ASYNC)

typedef void (*socket_event_callback)(void * socket, uint32 event, void * cookie);

struct socket {
	uint16 so_type;		/* type of socket */
	uint16 so_options;	/* socket options */
	int16 so_linger;	/* dreaded linger value */
	int16 so_state;		/* socket state */
	caddr_t so_pcb;		/* pointer to the control block */
	sem_id so_timeo;         /* our wait channel */

	struct protosw *so_proto; /* pointer to protocol module */

	struct socket *so_head;
	struct socket *so_q0;
	struct socket *so_q;

	int16  so_q0len;
	int16  so_qlen;
	int16  so_qlimit;
	int32  so_error;
//	pid_t  so_pgid;
	uint32 so_oobmark;

	/* our send/recv buffers */
	struct sockbuf so_snd;
	struct sockbuf so_rcv;

	// event callback
	socket_event_callback event_callback;
	void * event_callback_cookie;
};

/*
 * Socket state bits.
 */
#define SS_NOFDREF              0x001   /* no file table ref any more */
#define SS_ISCONNECTED          0x002   /* socket connected to a peer */
#define SS_ISCONNECTING         0x004   /* in process of connecting to peer */
#define SS_ISDISCONNECTING      0x008   /* in process of disconnecting */
#define SS_CANTSENDMORE         0x010   /* can't send more data to peer */
#define SS_CANTRCVMORE          0x020   /* can't receive more data from peer */
#define SS_RCVATMARK            0x040   /* at mark on input */
#define SS_ISDISCONNECTED       0x800   /* socket disconnected from peer */

#define SS_PRIV                 0x080   /* privileged for broadcast, raw... */
#define SS_NBIO                 0x100   /* non-blocking ops */
#define SS_ASYNC                0x200   /* async i/o notify */
#define SS_ISCONFIRMING         0x400   /* deciding to accept connection req */
#define SS_CONNECTOUT           0x1000  /* connect, not accept, at this end */

/* helpful defines... */

/* adjust counters in sb reflecting freeing of m */
#define sbfree(sb, m) { \
        (sb)->sb_cc -= (m)->m_len; \
        (sb)->sb_mbcnt -= MSIZE; \
        if ((m)->m_flags & M_EXT) \
                (sb)->sb_mbcnt -= (m)->m_ext.ext_size; \
}

#define sbspace(sb) \
    ((uint32) min((int)((sb)->sb_hiwat - (sb)->sb_cc), \
         (int)((sb)->sb_mbmax - (sb)->sb_mbcnt)))

/* do we have to send all at once on a socket? */
#define sosendallatonce(so) \
    ((so)->so_proto->pr_flags & PR_ATOMIC)

/* adjust counters in sb reflecting allocation of m */
#define sballoc(sb, m) { \
        (sb)->sb_cc += (m)->m_len; \
        (sb)->sb_mbcnt += MSIZE; \
        if ((m)->m_flags & M_EXT) \
                (sb)->sb_mbcnt += (m)->m_ext.ext_size; \
}

#define soreadable(so) \
	((so)->so_rcv.sb_cc >= (so)->so_rcv.sb_lowat || \
	 ((so)->so_state & SS_CANTRCVMORE) || \
	 (so)->so_qlen || (so)->so_error)

#define sowriteable(so) \
	((sbspace(&(so)->so_snd) >= (so)->so_snd.sb_lowat) && \
	 (((so)->so_state & SS_ISCONNECTED) || \
	  (((so)->so_proto->pr_flags & PR_CONNREQUIRED) == 0) || \
	 ((so)->so_state & SS_CANTSENDMORE) || (so)->so_error))

#define sorwakeup(so)   sowakeup((so), &(so)->so_rcv)
/* we don't handle upcall for sockets */
#define sowwakeup(so)   sowakeup((so), &(so)->so_snd)

#ifdef _NETWORK_STACK

uint32 sb_max;

/* Function prototypes */
int	soreserve (struct socket *so, uint32 sndcc, uint32 rcvcc);

int     initsocket(void **spp);
int     socreate(int dom, void *aso, int type, int proto);

int     sobind(void *so, caddr_t, int);
int     solisten(void *sp, int backlog);
int     soconnect(void *sp, caddr_t, int);
int     sosysctl (int *, uint, void *, size_t *, void *, size_t);
int     soshutdown(void *sp, int how);
int     writeit(void *, struct iovec *, int);
int     readit(void *, struct iovec *, int *);

int     sendit(void *, struct msghdr *, int, int *);
int     recvit(void *, struct msghdr *, caddr_t, int *);

int	    sosend(struct socket *so, struct mbuf *addr, struct uio *uio, 
               struct mbuf *top, struct mbuf *control, int flags);
int     sosetopt(void *, int, int, const void *, size_t);
int     sogetopt(void *, int, int, void *, size_t *);

struct socket *sonewconn(struct socket *head, int connstatus);
int 	set_socket_event_callback(void *, socket_event_callback, void *);

void    sbrelease (struct sockbuf *sb);
int     sbreserve (struct sockbuf *sb, uint32 cc);
void	sbdrop (struct sockbuf *sb, int len);
void    sbdroprecord (struct sockbuf *sb);
void    sbflush (struct sockbuf *sb);
int     sbwait (struct sockbuf *sb);
void	sbappend (struct sockbuf *sb, struct mbuf *m);
int     sbappendaddr (struct sockbuf *sb, struct sockaddr *asa,
            struct mbuf *m0, struct mbuf *control);
int     sbappendcontrol (struct sockbuf *sb, struct mbuf *m0,
            struct mbuf *control);
void    sbappendrecord (struct sockbuf *sb, struct mbuf *m0);
void    sbcheck (struct sockbuf *sb);
void    sbcompress (struct sockbuf *sb, struct mbuf *m, struct mbuf *n);
int     soreceive (struct socket *so, struct mbuf **paddr, struct uio *uio,
            struct mbuf **mp0, struct mbuf **controlp, int *flagsp);
void    sowakeup(struct socket *so, struct sockbuf *sb);
int     sbwait(struct sockbuf *sb);

int     soo_ioctl(void *sp, int cmd, caddr_t data);
int     soclose(void *sp);
int     sodisconnect(struct socket *);
void    sofree(struct socket *);

void    sohasoutofband(struct socket *so);
void    socantsendmore(struct socket *so);
void    socantrcvmore(struct socket *so);
void    soisconnected (struct socket *so);
void    soisconnecting (struct socket *so);
void    soisdisconnected (struct socket *so);
void    soisdisconnecting (struct socket *so);
void    soqinsque (struct socket *head, struct socket *so, int q);
int     soqremque (struct socket *so, int q);
int     sorflush(struct socket *so);

int     nsleep(sem_id chan, char *msg, int timeo);
void    wakeup(sem_id chan);

#endif /* _NETWORK_STACK */

#endif /* SYS_SOCKETVAR_H */
