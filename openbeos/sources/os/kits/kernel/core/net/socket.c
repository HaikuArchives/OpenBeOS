/* net.c */

/* XXX - add documentation to this file! */
#include <kernel.h>
#include <sem.h>
#include <lock.h>
#include <pools.h>
#include <vm.h>
#include <memheap.h>
#include <atomic.h>
#include <ktypes.h>
#include <stdio.h>
#include <string.h>
#include <errors.h>
#include <module.h>
#include <pools.h>
#include <debug.h>
#include <fd.h>

#define _KERNEL_
#include <socket.h>

/* Static global objects... */
static pool_ctl *spool;
static benaphore sockets_lock;

/* This is a hack just to get us building :) */
struct socket {
	int so_state;
};

/* XXX - temporary... these go into socket.h */
int sockets_init(kernel_args *ka);

#define sys_socket(a,b,c)    socket(a,b,c, true)

int socreate(int, struct socket **, int, int);
void sofree(struct socket *);

static ssize_t so_read(struct file_descriptor *, void *, off_t, size_t *);
static ssize_t so_write(struct file_descriptor *, const void *, off_t, size_t *);
static int so_close(struct file_descriptor *,int, struct ioctx *);
//static int so_ioctl(struct file_descriptor *, ulong, void *, size_t);


static struct fd_ops socket_functions = {
	"socket",
	so_read,
	so_write,
	NULL,
	so_close,
	NULL
};

int sockets_init(kernel_args *ka)
{
	if (!spool)
		pool_init(&spool, sizeof(struct socket));

	if (!spool)
		return ENOMEM;
	
	INIT_BENAPHORE(sockets_lock, "sockets_lock");
	dprintf("sockets_init completed!\n");
	
	return CHECK_BENAPHORE(sockets_lock);
}

int socket(int dom, int type, int protocol, bool kernel)
{
	struct socket *so = NULL;
	struct file_descriptor *f;
	int err;

	/* try to create a socket. If we fail we simply return. */

	err = socreate(dom, &so, type, protocol);
	if(err < 0)
		return err;

	/* So we have a socket...now get a file descriptor for it */
	f = alloc_fd();
	if(!f)
		return ENOMEM;
	
	f->cookie = so;
	f->ops    = &socket_functions;
	f->fd_type   = DTYPE_SOCKET;

	err = new_fd(get_current_ioctx(kernel), f);
	/* Hmm, this will loose any info from new_fd... */
	if (err < 0)
		err = ERR_VFS_FD_TABLE_FULL;

	dprintf("socket: fd %d (%p)created\n", err, f);
	
	return err;
}

int socreate(int dom, struct socket **so, int type, int proto)
{
	*so = (struct socket*)pool_get(spool);
	
	if (so == NULL) {
		dprintf("socreate: ENOMEM\n");
		return ENOMEM;
	}

	memset(*so, 0, sizeof(*so));

	return 0;
}

static ssize_t so_read(struct file_descriptor *f, void *buf, off_t pos, size_t *len)
{
	dprintf("so_read\n");
	*len = 0;
	return 0;
}

static ssize_t so_write(struct file_descriptor *f, const void *buf, off_t pos, size_t *len)
{
	dprintf("so_write\n");
	*len = 0;
	return 0;
}

static int so_close(struct file_descriptor *f, int fd, struct ioctx *io)
{
	sofree((struct socket*)f->cookie);
	f->cookie = NULL;
	
	dprintf("so_close: %d\n", fd);
	remove_fd(io, fd);
	put_fd(f);

	return 0;
}

void sofree(struct socket *so)
{
	pool_put(spool, so);
	return;
}
