/* socket "server" */

#include <stdio.h>
#include <kernel/OS.h>
#include <iovec.h>
#include <string.h>
#include <errno.h>

#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/sockio.h"
#include "sys/protosw.h"
#include "pools.h"
#include "net/if.h"
#include "netinet/in.h"
#include "netinet/in_pcb.h"
#include "net_module.h"
#include "net_misc.h"
#include "protocols.h"
#include "mbuf.h"
#include "sys/net_uio.h"
#ifdef _KERNEL_MODE
#include <KernelExport.h>
#include "core_module.h"
#endif

static pool_ctl *spool;
static benaphore sockets_lock;

/* OpenBSD sets this at 128??? */
static int somaxconn = SOMAXCONN;

/* for now - should be moved to be_error.h */
#define EDESTADDRREQ EINVAL

int sockets_init(void)
{
	if (!spool)
		pool_init(&spool, sizeof(struct socket));

	if (!spool)
		return ENOMEM;
	
	INIT_BENAPHORE(sockets_lock, "sockets_lock");
	return CHECK_BENAPHORE(sockets_lock);
}


void sockets_shutdown(void)
{
	pool_destroy(spool);
	
	UNINIT_BENAPHORE(sockets_lock);
}


/* uiomove! */

int uiomove(caddr_t cp, int n, struct uio *uio)
{
	struct iovec *iov;
	uint cnt;
	int error = 0;
	void *ptr;

	while (n > 0 && uio->uio_resid) {
		iov = uio->uio_iov;
		cnt = iov->iov_len;

		if (cnt == 0) {
			uio->uio_iov++;
			uio->uio_iovcnt--;
			continue;
		}
		if (cnt > n)
			cnt = n;

		switch (uio->uio_segflg) {
			case UIO_USERSPACE:
				if (uio->uio_rw == UIO_READ)
					ptr = memcpy(iov->iov_base, cp, cnt);
				else
					ptr = memcpy(cp, iov->iov_base, cnt);

				if (!ptr)
					return (errno);
				break;

			case UIO_SYSSPACE:
				if (uio->uio_rw == UIO_READ)
					ptr = memcpy(cp, iov->iov_base, cnt);
				else
					ptr = memcpy(iov->iov_base, cp, cnt);
				if (!ptr)
					return(errno);
		}
		iov->iov_base = (caddr_t)iov->iov_base + cnt;
		iov->iov_len -= cnt;
		uio->uio_resid -= cnt;
		uio->uio_offset += cnt;
		cp += cnt;
		n -= cnt;
	}
	return (error);
}

int initsocket(void **sp)
{
	struct socket *so = (struct socket*)pool_get(spool);

	if (so == NULL) {
		printf("initsocket - no memory!!\n");
		return ENOMEM;
	}

	memset(so, 0, sizeof(*so));

	*sp = so;

	return 0;
}

int socreate(int dom, void *sp, int type, int proto)
{
	struct protosw *prm = NULL; /* protocol module */
	struct socket *so = (struct socket*)sp;
	int error;

	if (so == NULL) {
		printf("socket passed in was NULL!\n");
		return EINVAL;
	}
	
	if (proto)
		prm = pffindproto(dom, proto, type);
	else
		prm = pffindtype(dom, type);

	if (!prm || !prm->pr_userreq)
		return EPROTONOSUPPORT;
	
	if (prm->pr_type != type)
		return EPROTOTYPE;
	
	so->so_type = type;
	so->so_proto = prm;
	so->so_rcv.sb_pop = create_sem(0, "so_rcv sem");
	so->so_timeo      = create_sem(0, "so_timeo");

#ifdef _KERNEL_MODE
	set_sem_owner(so->so_rcv.sb_pop, B_SYSTEM_TEAM);
	set_sem_owner(so->so_timeo, B_SYSTEM_TEAM);
#endif

	error = prm->pr_userreq(so, PRU_ATTACH, NULL, (struct mbuf *)proto, NULL);

	return error;
}


int soreserve(struct socket *so, uint32 sndcc, uint32 rcvcc)
{
	if (sbreserve(&so->so_snd, sndcc) == 0)
		goto bad;
	if (sbreserve(&so->so_rcv, rcvcc) == 0)
		goto bad2;

	if (so->so_rcv.sb_lowat == 0)
		so->so_rcv.sb_lowat = 1;
	if (so->so_snd.sb_lowat == 0)
		so->so_snd.sb_lowat = MCLBYTES;
	if (so->so_snd.sb_lowat > so->so_snd.sb_hiwat)
		so->so_snd.sb_lowat = so->so_snd.sb_hiwat;

	return (0);

bad2:
	sbrelease(&so->so_snd);
bad:
	return (ENOBUFS);
}


int sobind(void *sp, caddr_t data, int len)
{
	int error;
	struct mbuf *nam = m_get(MT_DATA);
	struct socket *so = (struct socket*)sp;

	if (!nam)
		return ENOMEM;

	nam->m_len = len;
	memcpy(mtod(nam, char*), data, len);

/* xxx - locking! */
	error = (*so->so_proto->pr_userreq) (so, PRU_BIND, NULL, nam, NULL);
	
	m_freem(nam);
	
	return error;
}

int solisten(void *sp, int backlog)
{
	struct socket *so = (struct socket *)sp;
	int error;

        error = so->so_proto->pr_userreq(so, PRU_LISTEN, NULL, NULL, NULL);
        if (error)
                return error;
        
        if (so->so_q == 0)
                so->so_options |= SO_ACCEPTCONN;
        if (backlog < 0 || backlog > somaxconn)
                backlog = somaxconn;
	/* OpenBSD defines a minimum of 80...hmmm... */
        if (backlog < 0)
                backlog = 0;
        so->so_qlimit = backlog;
        return 0;
}

int soconnect(void *sp, struct mbuf *nam)
{
	struct socket *so = (struct socket *)sp;
	int error;

	if (so->so_options & SO_ACCEPTCONN)
		return (EOPNOTSUPP);
	/*
	 * If protocol is connection-based, can only connect once.
	 * Otherwise, if connected, try to disconnect first.
	 * This allows user to disconnect by connecting to, e.g.,
	 * a null address.
	 */
	if (so->so_state & (SS_ISCONNECTED|SS_ISCONNECTING) &&
	    ((so->so_proto->pr_flags & PR_CONNREQUIRED) ||
	    (error = sodisconnect(so))))
		error = EISCONN;
	else
		error = so->so_proto->pr_userreq(so, PRU_CONNECT,
		                                 NULL, nam, NULL);
	return error;
}


int sendit(void *sp, struct msghdr *mp, int flags, int *retsize)
{
	struct socket *so = (struct socket *)sp;
	struct uio auio;
	struct iovec *iov;
	int i;
	struct mbuf *to;
	struct mbuf *control;
	int len;
	int error;

	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_WRITE;
	auio.uio_offset = 0;
	auio.uio_resid = 0;
	iov = mp->msg_iov;

	/* Make sure we don't exceed max size... */
	for (i=0;i < mp->msg_iovlen;i++, iov++) {
		if (iov->iov_len > SSIZE_MAX ||
		    (auio.uio_resid += iov->iov_len) > SSIZE_MAX)
			return EINVAL;
	}
	if (mp->msg_name) {
		/* stick msg_name into an mbuf */
		to = m_get(MT_SONAME);
		to->m_len = mp->msg_namelen;
		memcpy(mtod(to, char*), mp->msg_name, mp->msg_namelen);
	} else
		to = NULL;
	
	if (mp->msg_control) {
		if (mp->msg_controllen < sizeof(struct cmsghdr)) {
			error = EINVAL;
			goto bad;
		}
		control = m_get(MT_CONTROL);
		control->m_len = mp->msg_controllen;
		memcpy(mtod(control, char*), mp->msg_control, mp->msg_controllen);
	} else
		control = NULL;

	len = auio.uio_resid;

	error = sosend(so, to, &auio, NULL, control, flags);
	if (error) {
		/* what went wrong! */
		if (auio.uio_resid != len && (/*error == ERESTART || */
		    error == EINTR || error == EWOULDBLOCK))
			error = 0; /* not really an error */
	}
	if (error == 0)
		*retsize = len - auio.uio_resid; /* tell them how much we did send */
bad:
	if (to)
		m_freem(to);

	return error;
}


int sosend(struct socket *so, struct mbuf *addr, struct uio *uio, struct mbuf *top,
	   struct mbuf *control, int flags)
{
	struct mbuf **mp;
	struct mbuf *m;
	int32 space;
	int32 len;
	uint64 resid;
	int clen = 0;
	int error, dontroute, mlen;
	int atomic = sosendallatonce(so) || top;

	if (uio)
		resid = uio->uio_resid;
	else
		resid = top->m_pkthdr.len;

	/* resid shouldn't be below 0 and also a flag of MSG_EOR on a 
	 * SOCK_STREAM isn't allowed (doesn't even make sense!)
	 */
	if (resid < 0 || (so->so_type == SOCK_STREAM && (flags & MSG_EOR))) {
		error = EINVAL;
		goto release;
	}

	dontroute = (flags & MSG_DONTROUTE) && (so->so_options & SO_DONTROUTE) == 0 &&
		    (so->so_proto->pr_flags & PR_ATOMIC);

	if (control)
		clen = control->m_len;

#define snderr(errno)	{ error = errno; /* unlock */ goto release; } 
restart:
	/* XXX - lock here! */
	
	/* Main Loop! We should loop here until resid == 0 */
	do { 
		if (so->so_state & SS_CANTSENDMORE)
			snderr(EPIPE); /* ??? */
		if (so->so_error)
			snderr(so->so_error);
		if ((so->so_state & SS_ISCONNECTED) == 0) {
			if (so->so_proto->pr_flags & PR_CONNREQUIRED) {
				/* we need to be connected and we're not... */
				if ((so->so_state & SS_ISCONFIRMING) == 0 &&
				    !(resid == 0 && clen != 0))
					/* we're not even trying to connect and we
					 * have data to send, so it's an error!
					 * return ENOTCONN
					 */
					snderr(ENOTCONN);
			} else if (addr == NULL)
				/* UDP is a connectionless protocol, so it can work
				 * without being connected as long as we tell it where we
				 * want to send the data :)
				 */
				/* Doh! No address to send to (UDP) */
				snderr(EDESTADDRREQ);
		}
		space = sbspace(&so->so_snd);

		if (flags & MSG_OOB)
			space += 1024;

		if ((atomic && resid > so->so_snd.sb_hiwat) || clen > so->so_snd.sb_hiwat)
			snderr(EMSGSIZE);

		if (space < resid + clen && uio && 
		    (atomic || space < so->so_snd.sb_lowat || space < clen)) {
			if (so->so_state & SS_NBIO) /* non blocking set */
				snderr(EWOULDBLOCK);
			/* free lock - we're waiting on send buffer space */
			/* XXX - unlcok */
			error = sbwait(&so->so_snd);
			if (error)
				goto out;
			goto restart;
		}
		mp = &top;
		space -= clen;

		do {
			if (!uio) {
				/* data is actually just packaged as top. */
				resid = 0;
				if (flags & MSG_EOR)
					top->m_flags |= M_EOR;
			} else do {
				if (!top) {
					MGETHDR(m, MT_DATA);
					mlen = MHLEN;
					m->m_pkthdr.len = 0;
					m->m_pkthdr.rcvif = NULL;
				} else {
					MGET(m, MT_DATA);
					mlen = MLEN;
				}
				if (resid >= MINCLSIZE && space >= MCLBYTES) {
					MCLGET(m);
					if ((m->m_flags & M_EXT) == 0)
						/* didn't get a cluster */
						goto nopages;

					mlen = MCLBYTES;
					if (atomic && !top) {
						len = min(MCLBYTES - max_hdr, resid);
						m->m_data += max_hdr;
					} else
						len = min(MCLBYTES, resid);
					space -= len;
				} else {
nopages:
					len = min(min(mlen, resid), space);
					space -= len;
					/* leave room for headers if required */
					if (atomic && !top && len < mlen)
						MH_ALIGN(m, len);
				}

				error = uiomove(mtod(m, caddr_t), (int)len, uio);
				resid = uio->uio_resid;
				m->m_len = len;
				*mp = m;
				top->m_pkthdr.len += len;
				if (error)
					goto release;
				mp = &m->m_next;
				if (resid <= 0) {
					if (flags & MSG_EOR)
						/* we're the last record */
						top->m_flags |= M_EOR;
					break;
				}
			} while (space > 0 && atomic);

			if (dontroute)
				so->so_options |= SO_DONTROUTE;

			/* XXX - locking */
			error = (*so->so_proto->pr_userreq)(so, (flags & MSG_OOB) ? PRU_SENDOOB: PRU_SEND,
						        top, addr, control);

			/* XXX - unlock */
			if (dontroute)
				so->so_options &= ~SO_DONTROUTE;
			clen = 0;
			top = NULL;
			control = NULL;
			mp = &top;
			if (error)
				goto release;
		} while (resid && space > 0);
	} while (resid);

release:
	/* unlock */
out:
	if (top)
		m_freem(top);
	if (control)
		m_freem(control);
	return (error);
}


int recvit(void *sp, struct msghdr *mp, caddr_t namelenp, int *retsize)
{
	struct socket *so = (struct socket*)sp;
	struct uio auio;
	struct iovec *iov;
	struct mbuf *control = NULL;
	struct mbuf *from = NULL;
	int error = 0, i, len = 0;

	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_READ;
	auio.uio_offset = 0;
	auio.uio_resid = 0;
	iov = mp->msg_iov;

	for (i=0; i < mp->msg_iovlen; i++, iov++) {
		if (iov->iov_len < 0)
			return EINVAL;
		if ((auio.uio_resid += iov->iov_len) < 0)
			return EINVAL;
	}
	len = auio.uio_resid;

	if ((error = soreceive(so, &from, &auio,
				NULL, mp->msg_control ? &control : NULL, 
				&mp->msg_flags)) != 0) {
		if (auio.uio_resid != len && (error == EINTR || error == EWOULDBLOCK))
			error = 0;
	}

	if (error)
		goto out;

	*retsize = len - auio.uio_resid;

	if (mp->msg_name) {
		len = mp->msg_namelen;
		
		if (len <= 0 || !from) {
			len = 0;
		} else {
			if (len > from->m_len)
				len = from->m_len;
			memcpy((caddr_t)mp->msg_name, mtod(from, caddr_t), len);
		}
		mp->msg_namelen = len;
		if (namelenp)
			memcpy(namelenp, (caddr_t)&len, sizeof(int));
	}

	/* XXX - add control handling */
out:
	if (from)
		m_freem(from);
	if (control)
		m_freem(control);

	return error;
}


int soreceive(struct socket *so, struct mbuf **paddr, struct uio *uio, struct mbuf**mp0,
		struct mbuf **controlp, int *flagsp)
{
	struct mbuf *m, **mp;
	int flags = 0;
	int len, error = 0, offset;
	struct mbuf *nextrecord;
	int moff, type = 0;
	int orig_resid = uio->uio_resid;
	struct protosw *pr = so->so_proto;

	mp = mp0;
	if (paddr)
		*paddr = NULL;
	if (controlp)
		*controlp = NULL;

	/* ensure we don't have MSG_EOR set */
	if (flagsp)
		flags = (*flagsp) & ~MSG_EOR;

	/* XXX - handle OOB data */

restart:
	/* XXX - locking */
	m = so->so_rcv.sb_mb;

	/* if we don't want to wait for incoming packets, let's check the
	 * the current buffer state if it does fit our requirements
	 */ 
	/* XXX - I fixed a warning here, but is this expression correct anyway?
	 * the last part makes me wonder... -- axeld.
	 */
	if (!m || ((flags & MSG_DONTWAIT) == 0
			&& so->so_rcv.sb_cc < uio->uio_resid
			&& (so->so_rcv.sb_cc < so->so_rcv.sb_lowat ||
				((flags & MSG_WAITALL) && (uio->uio_resid <= so->so_rcv.sb_hiwat)))
			&& !m->m_nextpkt && (pr->pr_flags & PR_ATOMIC) == 0)) {
		if (so->so_error) {
			if (m)
				goto dontblock;
			error = so->so_error;
			if ((flags & MSG_PEEK) == 0)
				so->so_error = 0;
			goto release;
		}
		if (so->so_state & SS_CANTRCVMORE) {
			if (m)
				goto dontblock;
			else
				goto release;
		}
		for (;m; m = m->m_next)
			if (m->m_type == MT_OOBDATA || (m->m_flags & M_EOR)) {
				m = so->so_rcv.sb_mb;
				goto dontblock;
			}
		if ((so->so_state & (SS_ISCONNECTED | SS_ISCONNECTING)) == 0 &&
			(so->so_proto->pr_flags & PR_CONNREQUIRED)) {
			error = ENOTCONN;
			goto release;
		}
		if (uio->uio_resid == 0)
			goto release;
		if ((so->so_state & SS_NBIO) || (flags & MSG_DONTWAIT)) {
			error = EWOULDBLOCK;	
			goto release;
		}
		error = sbwait(&so->so_rcv);
		if (error)
			return error;
		goto restart;
	}

dontblock:
	nextrecord = m->m_nextpkt;
	if (pr->pr_flags & PR_ADDR) {
		orig_resid = 0;
		if (flags & MSG_PEEK) {
			if (!paddr)
				*paddr = m_copym(m, 0, m->m_len);
			m = m->m_next;
		} else {
			sbfree(&so->so_rcv, m);
			if (paddr) {
				*paddr = m;
				so->so_rcv.sb_mb = m->m_next;
				m->m_next = NULL;
				m = so->so_rcv.sb_mb;
			} else {
				MFREE(m, so->so_rcv.sb_mb);
				m = so->so_rcv.sb_mb;
			}
		}
	}
	if (m) {
		if ((flags & MSG_PEEK) == 0)
			m->m_nextpkt = nextrecord;
		type = m->m_type;
		if (type == MT_OOBDATA)
			flags |= MSG_OOB;
	}

	moff = 0;
	offset = 0;
	while (m && uio->uio_resid > 0 && error == 0) {
		if (m->m_type == MT_OOBDATA) {
			if (type != MT_OOBDATA)
				break;
		} else if (type == MT_OOBDATA) {	
			break;
		}
		so->so_state &= ~SS_RCVATMARK;
		len = uio->uio_resid;
		if (so->so_oobmark && len > so->so_oobmark - offset)
			len = so->so_oobmark - offset;
		if (len > m->m_len - moff)
			len = m->m_len - moff;
		if (!mp)
			error = uiomove(mtod(m, caddr_t) + moff, (int)len, uio);
		else
			uio->uio_resid -= len;
		if (len == m->m_len - moff) {
			if (m->m_flags & M_EOR)
				flags |= MSG_EOR;
			if (flags & MSG_PEEK) {
				m = m->m_next;
				moff = 0;
			} else {
				nextrecord = m->m_nextpkt;
				sbfree(&so->so_rcv, m);
				if (mp) {
					*mp = m;
					mp = &m->m_next;
					so->so_rcv.sb_mb = m = m->m_next;
					*mp = NULL;
				} else {
					MFREE(m, so->so_rcv.sb_mb);
					m = so->so_rcv.sb_mb;
				}
				if (m)
					m->m_nextpkt = nextrecord;
			}
		} else {
			if (flags & MSG_PEEK)
				moff += len;
			else {
				if (mp)
					*mp = m_copym(m, 0, len);
				m->m_data += len;
				m->m_len -= len;
				so->so_rcv.sb_cc -= len;
			}
		}
		if (so->so_oobmark) {
			if ((flags & MSG_PEEK) == 0) {
				so->so_oobmark -= len;
				if (so->so_oobmark == 0) {
					so->so_state |= SS_RCVATMARK;
					break;
				}
			} else {
				offset += len;
				if (offset == so->so_oobmark)
					break;
			}
		}
		if (flags & MSG_EOR)
			break;
		/* XXX - wait for more data if reqd */
	}

	if (m && pr->pr_flags & PR_ATOMIC) {
		flags |= MSG_TRUNC;
		if ((flags & MSG_PEEK) == 0) 
			sbdroprecord(&so->so_rcv);
	}
	if ((flags & MSG_PEEK) == 0) {
		if (!m)
			so->so_rcv.sb_mb = nextrecord;
		if (pr->pr_flags & PR_WANTRCVD && so->so_pcb)
			(*pr->pr_userreq)(so, PRU_RCVD, (struct mbuf*)flags, NULL, NULL);
	}
	if (orig_resid == uio->uio_resid && orig_resid &&
		(flags & MSG_EOR) == 0 && (so->so_state & SS_CANTRCVMORE) == 0)
		goto restart;
	
	if (flagsp)
		*flagsp = flags;

release:
	return error;
}

int soo_ioctl(void *sp, int cmd, caddr_t data)
{
	struct socket *so = (struct socket*)sp;

	switch (cmd) {
		case FIONBIO:
			if (*(int*)data)
				so->so_state |= SS_NBIO;
			else
				so->so_state &= ~SS_NBIO;
			return 0;
		case FIONREAD:
			/* how many bytes do we have waiting... */
			*(int*)data = so->so_rcv.sb_cc;
			return 0;
	}

	if (IOCGROUP(cmd) == 'i') {
		return ifioctl(so, cmd, data);
	}
	if (IOCGROUP(cmd) == 'r') {
		return 0;
	}	
	return (*so->so_proto->pr_userreq)(so, PRU_CONTROL, 
		(struct mbuf*)cmd, (struct mbuf*)data, NULL);
}

int soclose(void *sp)
{
	struct socket *so = (struct socket*)sp;
	int error = 0;

	if (so->so_pcb == NULL)
		goto discard;

	if (so->so_state & SS_ISCONNECTED) {
		if ((so->so_state & SS_ISDISCONNECTING) == 0) {
			error = sodisconnect(so);
			if (error)
				goto drop;
		}
	}

drop:
	if (so->so_pcb) {
		int error2 = (*so->so_proto->pr_userreq)(so, PRU_DETACH, NULL, NULL, NULL);
		if (error2 == 0)
			error = error2;
	}

discard:
	sofree(so);

	return error;
}

int sodisconnect(struct socket *so)
{
	int error;
	
	if ((so->so_state & SS_ISCONNECTED) == 0) {
		error = ENOTCONN;
		goto bad;
	}
	if (so->so_state & SS_ISDISCONNECTING) {
		error = EALREADY;
		goto bad;
	}
	error = so->so_proto->pr_userreq(so, PRU_DISCONNECT, 
		NULL, NULL, NULL);

bad:
	return error;
}

void sofree(struct socket *so)
{
	if (so->so_pcb) {
		printf("Can't free - still have a pcb!\n");
		return;
	}

	if (so->so_head) {
		if (!soqremque(so, 0) && ! soqremque(so, 1)) {
			printf("sofree: Couldn't remove the queues!\n");
		}
		/* we need to handle this! */
		so->so_head = NULL;
	}
	
	delete_sem(so->so_rcv.sb_pop);

	pool_put(spool, so);

	return;
}

void sowakeup(struct socket *so, struct sockbuf *sb)
{
	sb->sb_flags &= ~SB_SEL;
	if (sb->sb_flags & SB_WAIT) {
		sb->sb_flags &= ~SB_WAIT;
		release_sem(sb->sb_pop);
	}
}	

void soqinsque(struct socket *head, struct socket *so, int q)
{
	struct socket **prev;
	so->so_head = head;
	if (q == 0) {
		head->so_q0len++;
		so->so_q0 = 0;
		for (prev = &(head->so_q0); *prev; )
			prev = &((*prev)->so_q0);
	} else {
		head->so_qlen++;
		so->so_q = NULL;
		for (prev = &(head->so_q); *prev; )
			prev = &((*prev)->so_q);
	}
	*prev = so;
}

int soqremque(struct socket *so, int q)
{
	struct socket *head, *prev, *next;

	head = so->so_head;
	prev = head;
	for (;;) {
		next = q ? prev->so_q : prev->so_q0;
		if (next == so)
			break;
		if (next == NULL)
			return 0;
		prev = next;
	}
	if (q == 0) {
		prev->so_q0 = next->so_q0;
		head->so_q0len--;
	} else {
		prev->so_q = next->so_q;
		head->so_qlen--;
	}
	next->so_q0 = next->so_q = 0;
	next->so_head = 0;
	return 1;
}

void socantsendmore(struct socket *so)
{
	so->so_state |= SS_CANTSENDMORE;
	sowwakeup(so);
}

void socantrcvmore(struct socket *so)
{
	so->so_state |= SS_CANTRCVMORE;
	sorwakeup(so);
}

void soisconnecting(struct socket *so)
{
	so->so_state &= ~(SS_ISCONNECTED|SS_ISDISCONNECTING);
	so->so_state |= SS_ISCONNECTING;
}

void soisconnected(struct socket *so)
{
	struct socket *head = so->so_head;

	so->so_state &= ~(SS_ISCONNECTING|SS_ISDISCONNECTING|SS_ISCONFIRMING);
	so->so_state |= SS_ISCONNECTED;
	if (head && soqremque(so, 0)) {
		soqinsque(head, so, 1);
		sorwakeup(head);
		wakeup(head->so_timeo);
	} else {
		wakeup(so->so_timeo);
		sorwakeup(so);
		sowwakeup(so);
	}
}

void soisdisconnected(struct socket *so)
{
	so->so_state &= ~(SS_ISCONNECTING|SS_ISCONNECTED|SS_ISDISCONNECTING);
	so->so_state |= (SS_CANTRCVMORE|SS_CANTSENDMORE|SS_ISDISCONNECTED);
	wakeup(so->so_timeo);
	sowwakeup(so);
	sorwakeup(so);
}

int nsleep(sem_id chan, char *msg, int timeo)
{
	status_t rv;
/* XXX - OK, this is no longer supported, how do we do this then?
#ifdef _KERNEL_MODE
	struct thread_rec str;
#endif
*/
	printf("nsleep: %s\n", msg);

	if (timeo > 0)
		rv = acquire_sem_etc(chan, 1, B_TIMEOUT|B_CAN_INTERRUPT, timeo);
	else
		rv = acquire_sem_etc(chan, 1, B_CAN_INTERRUPT, 0);

	if (rv == B_TIMED_OUT)
		return EWOULDBLOCK;
/* ???
#ifdef _KERNEL_MODE
	if (has_sigal_pending(&str) == 0)
		return 0;
	// we should check the signal mask here... 
#endif
*/
	return EINTR;
}

void wakeup(sem_id chan)
{
	/* we should release as many as are waiting...
	 * the number 100 is just something that shuld be large enough...
	 */
	release_sem_etc(chan, 100, B_DO_NOT_RESCHEDULE);
}

