/* socket "server" */

#include <stdio.h>
#include <kernel/OS.h>
#include <iovec.h>
#include <string.h>
#include <errno.h>

#include "sys/socket.h"
#include "sys/socketvar.h"
#include "pools.h"
#include "netinet/in_pcb.h"
#include "net_module.h"
#include "net_misc.h"
#include "protocols.h"
#include "mbuf.h"
#include "sys/net_uio.h"


/* XXX - This file has the various system calls in it, socket, bind, sendto 
 * which really don't belong here and are just here to allow for testing.
 * They should be moved into libsocket as soon as we have one.
 */

struct sock_fd {
	struct sock_fd *next;
	struct sock_fd *prev;
	int fd;
	struct socket *so;
};

/* XXX - This is just so lame! */
static int32 socknum = 0;

static pool_ctl *spool;
static pool_ctl *sfdpool;

static struct sock_fd *sockets = NULL;
static benaphore sockets_lock;


/* XXX - Hack for debugging */
#undef SHOW_DEBUG
#define SHOW_DEBUG 1

/* for now - should be moved to be_error.h */
#define EDESTADDRREQ EINVAL


int sockets_init(void)
{
	if (!spool)
		pool_init(&spool, sizeof(struct socket));

	if (!spool)
		return ENOMEM;

	if (!sfdpool)
		pool_init(&sfdpool, sizeof(struct sock_fd));

	INIT_BENAPHORE(sockets_lock, "sockets_lock");
	return CHECK_BENAPHORE(sockets_lock);
}


void sockets_shutdown(void)
{
	pool_destroy(spool);
	pool_destroy(sfdpool);
	
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
				printf("Duh??? We don't mess with system space!! \n");
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


static int get_sock(int fd, struct sock_fd **sfd)
{
	struct sock_fd *f = sockets;
	while (f) {
		if (f->fd == fd)
			break;
		f = f->next;
		if (f == sockets) {
			f = NULL;
			break;
		}
	}
	*sfd = f;
	if (f)
		return 0;
	return EBADF; /* is this correct? */
}


int socreate(int dom, struct socket **aso, int type, int proto)
{
	net_module *prm = NULL; /* protocol module */
	struct socket *so;
	int error;
	
	if (proto)
		prm = pffindproto(dom, proto, type);
	else
		prm = pffindtype(dom, type);
	
	if (!prm || !prm->userreq)
		return EPROTONOSUPPORT;
	
	if (prm->sock_type != type)
		return EPROTOTYPE;
	
	so = (struct socket*)pool_get(spool);
	memset(so, 0, sizeof(struct socket));
	so->so_type = type;
	so->so_proto = prm;
	so->so_rcv.sb_pop = create_sem(0, "so_rcv sem");

	error = prm->userreq(so, PRU_ATTACH, NULL, (struct mbuf *)proto, NULL);
	*aso = so;

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


int sobind(struct socket *so, struct mbuf *nam)
{
	int error;
/* xxx - locking! */
	error = (*so->so_proto->userreq) (so, PRU_BIND, NULL, nam, NULL);

	return error;
}


int sendit(int sock, struct msghdr *mp, int flags, size_t *retsize)
{
	struct sock_fd *sfd;
	struct uio auio;
	struct iovec *iov;
	int i;
	struct mbuf *to;
	struct mbuf *control;
	int len;
	int error;

	if ((error = get_sock(sock, &sfd)) != 0)
		return error;

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

	error = sosend(sfd->so, to, &auio, NULL, control, flags);
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
		    (so->so_proto->flags & PR_ATOMIC);

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
			if (so->so_proto->flags & PR_CONNREQUIRED) {
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
			error = (*so->so_proto->userreq)(so, (flags & MSG_OOB) ? PRU_SENDOOB: PRU_SEND,
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


int recvit(int s, struct msghdr *mp, caddr_t namelenp, int *retsize)
{
	struct sock_fd *sfd;
	struct uio auio;
	struct iovec *iov;
	struct mbuf *control = NULL;
	struct mbuf *from = NULL;
	int error = 0, i, len = 0;

	if ((error = get_sock(s, &sfd)) != 0)
		return error;

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

	if ((error = soreceive(sfd->so, &from, &auio,
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
		if (len <= 0 || !from)
			len = 0;
		else {
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
	struct net_module *pr = so->so_proto;

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
			&& !m->m_nextpkt && (pr->flags & PR_ATOMIC) == 0)) {
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
			(so->so_proto->flags & PR_CONNREQUIRED)) {
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
	if (pr->flags & PR_ADDR) {
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

	if (m && pr->flags & PR_ATOMIC) {
		flags |= MSG_TRUNC;
		if ((flags & MSG_PEEK) == 0) 
			sbdroprecord(&so->so_rcv);
	}
	if ((flags & MSG_PEEK) == 0) {
		if (!m)
			so->so_rcv.sb_mb = nextrecord;
		if (pr->flags & PR_WANTRCVD && so->so_pcb)
			(*pr->userreq)(so, PRU_RCVD, (struct mbuf*)flags, NULL, NULL);
	}
	if (orig_resid == uio->uio_resid && orig_resid &&
		(flags & MSG_EOR) == 0 && (so->so_state & SS_CANTRCVMORE) == 0)
		goto restart;
	
	if (flagsp)
		*flagsp = flags;

release:
	return error;
}


void sowakeup(struct socket *so, struct sockbuf *sb)
{
	sb->sb_flags &= ~SB_SEL;
	if (sb->sb_flags & SB_WAIT) {
		sb->sb_flags &= ~SB_WAIT;
		release_sem(sb->sb_pop);
	}
}	


//	#pragma mark -


int socket(int dom, int type, int proto)
{
	struct socket *so;
	struct sock_fd *sfd;
	/* This is a hack to enable us to create a socket! */
	int error;

	error = socreate(dom, &so, type, proto);
	if (error < 0)
		return error;

	sfd = (struct sock_fd*)pool_get(sfdpool);
	sfd->so = so;
	sfd->fd = atomic_add(&socknum, 1);

	/* XXX - add locking! */
	if (sockets) {
		sockets->prev->next = sfd;
		sfd->prev = sockets->prev;
		sfd->next = sockets;
		sockets->prev = sfd;
	} else {
		sfd->next = sfd->prev = sfd;
		sockets = sfd;
	}
#if SHOW_DEBUG
	printf("Just created socket #%d (%s)\n",  sfd->fd, sfd->so->so_proto->name);
#endif

	return sfd->fd;
}


/* NB normally we'd be worried about kernel vs userland here, but
 * that's not a worry for a userland stack.
 * XXX - if this ever goes into kernel, fix this!!!
 */

int bind(int sock, struct sockaddr *name, size_t namelen)
{
	int error;
	struct mbuf *nam;
	struct sock_fd *sfd;

	if ((error = get_sock(sock, &sfd)))
		return error;

	nam = m_get(MT_SONAME);
	nam->m_len = namelen;
	memcpy(mtod(nam, char*), name, namelen);

	error = sobind(sfd->so, nam);
	m_freem(nam);
	return error;
}


int sendto(int sock, caddr_t buf, size_t len, int flags, 
	   struct sockaddr *to, size_t tolen)
{
	struct msghdr msg;
	struct iovec aiov;
	size_t retval = 0;
	int error = 0;

	msg.msg_name = (caddr_t)to;
	msg.msg_namelen = tolen;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;

	aiov.iov_base = (char*)buf;
	aiov.iov_len = len;

	error = sendit(sock, &msg, flags, &retval);
	if (error < 0)
		return error;
	return retval;
}


int recvfrom(int sock, caddr_t buf, size_t len, int flags,
           struct sockaddr *from, size_t fromlen)
{
	struct msghdr msg;
	struct iovec aiov;
	int retval = 0;
	int error = 0;

	msg.msg_name = (caddr_t)from;
	msg.msg_namelen = fromlen;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_flags = flags;

	aiov.iov_base = (char*)buf;
	aiov.iov_len = len;
	
	error = recvit(sock, &msg, NULL, &retval);
	if (error < 0)
		return error;
	return retval;
}

