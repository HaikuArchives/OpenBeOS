/* socket "server" */

#include <stdio.h>
#include <kernel/OS.h>

#include "sys/socket.h"
#include "sys/socketvar.h"
#include "pools.h"
#include "netinet/in_pcb.h"
#include "net_module.h"
#include "net_misc.h"
#include "protocols.h"
#include "mbuf.h"

static pool_ctl *spool;

/* XXX - This is just so lame! */
static int32 socknum = 0;

struct sock_fd {
	struct sock_fd *next;
	struct sock_fd *prev;
	int fd;
	struct socket *so;
};
static pool_ctl *sfdpool;
static struct sock_fd *sockets = NULL;

/* XXX - Hack for debugging */
#undef NET_DEBUG
#define NET_DEBUG 1

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

int sockets_init(void)
{
	if (!spool)
		pool_init(&spool, sizeof(struct socket));

	if (!spool)
		return ENOMEM;

	if (!sfdpool)
		pool_init(&sfdpool, sizeof(struct sock_fd));

	return 0;
}

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
#if NET_DEBUG
	printf("Just created socket #%d (%s)\n",  sfd->fd, sfd->so->so_proto->name);
#endif

	return sfd->fd;
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
 
int sobind(struct socket *so, struct mbuf *nam)
{
	int error;
/* xxx - locking! */
	error = (*so->so_proto->userreq) (so, PRU_BIND, NULL, nam, NULL);

	return error;
}

