/* fd.c
 *
 * Operations on file descriptors...
 * see fd.h for the definitions
 *
 */

#include <ktypes.h>
#include <fd.h>
#include <vfs.h>
#include <errors.h>
#include <debug.h>
#include <memheap.h>

#define CHECK_USER_ADDR(x) \
	if ((addr)(x) >= KERNEL_BASE && (addr)(x) <= KERNEL_TOP) \
		return EINVAL; 

#define CHECK_SYS_ADDR(x) \
	if ((addr)(x) < KERNEL_BASE && (addr)(x) >= KERNEL_TOP) \
		return EINVAL; 


/* General fd routines */
struct file_descriptor *alloc_fd(void)
{
	struct file_descriptor *f;

	f = kmalloc(sizeof(struct file_descriptor));
	if(f) {
		f->vnode = NULL;
		f->cookie = NULL;
		f->ref_count = 1;
	}
	return f;
}

int new_fd(struct ioctx *ioctx, struct file_descriptor *f)
{
	int fd = -1;
	int i;

	mutex_lock(&ioctx->io_mutex);

	for(i=0; i<ioctx->table_size; i++) {
		if(!ioctx->fds[i]) {
			fd = i;
			break;
		}
	}
	if(fd < 0) {
		fd = ERR_NO_MORE_HANDLES;
		goto err;
	}

	ioctx->fds[fd] = f;
	ioctx->num_used_fds++;

err:
	mutex_unlock(&ioctx->io_mutex);

	return fd;
}

struct file_descriptor *get_fd(struct ioctx *ioctx, int fd)
{
	struct file_descriptor *f;

	mutex_lock(&ioctx->io_mutex);

	if(fd >= 0 && fd < ioctx->table_size && ioctx->fds[fd]) {
		// valid fd
		f = ioctx->fds[fd];
		atomic_add(&f->ref_count, 1);
 	} else {
		f = NULL;
	}

	mutex_unlock(&ioctx->io_mutex);
	return f;
}

void remove_fd(struct ioctx *ioctx, int fd)
{
	struct file_descriptor *f;

	mutex_lock(&ioctx->io_mutex);

	if(fd >= 0 && fd < ioctx->table_size && ioctx->fds[fd]) {
		// valid fd
		f = ioctx->fds[fd];
		ioctx->fds[fd] = NULL;
		ioctx->num_used_fds--;
	} else {
		f = NULL;
	}

	mutex_unlock(&ioctx->io_mutex);

	if(f)
		put_fd(f);
}

/* USER routines */ 
ssize_t user_read(int fd, void *buf, off_t pos, ssize_t len)
{
	struct file_descriptor *f = get_fd(get_current_ioctx(false), fd);

	dprintf("user_read: fd %d\n", fd);
	
	if(!f)
		return EBADF;

	/* This is a user_function, so abort if we have a kernel address */
	CHECK_USER_ADDR(buf)
	
	if (f->ops->fd_read)
		return f->ops->fd_read(f, buf, pos, len);
	else
		return EOPNOTSUPP;
}

ssize_t user_write(int fd, const void *buf, off_t pos, ssize_t len)
{
	struct file_descriptor *f = get_fd(get_current_ioctx(false), fd);

	dprintf("user_write: fd %d\n", fd);
	
	if(!f)
		return EBADF;

	CHECK_USER_ADDR(buf)

	if (f->ops->fd_write)
		return f->ops->fd_write(f, buf, pos, len);
	else
		return EOPNOTSUPP;
}

int user_ioctl(int fd, int op, void *buf, size_t len)
{
	struct file_descriptor *f = get_fd(get_current_ioctx(false), fd);

	dprintf("user_ioctl: fd %d\n", fd);
	
	if(!f)
		return EBADF;

	CHECK_USER_ADDR(buf)

	if (f->ops->fd_ioctl)
		return f->ops->fd_ioctl(f, op, buf, len);
	else
		return EOPNOTSUPP;
}


int user_close(int fd)
{
	struct ioctx *ic = get_current_ioctx(false);
	struct file_descriptor *f = get_fd(ic, fd);
	
	if (!f)
		return EBADF;
	
	if (f->ops->fd_close)
		return f->ops->fd_close(f, fd, ic);
	else
		return EOPNOTSUPP;
}

/* SYSTEM functions */

ssize_t sys_read(int fd, void *buf, off_t pos, ssize_t len)
{
	struct file_descriptor *f = get_fd(get_current_ioctx(true), fd);

	dprintf("user_read: fd %d\n", fd);
	
	if(!f)
		return EBADF;

	/* This is a user_function, so abort if we have a kernel address */
	CHECK_SYS_ADDR(buf)
	
	if (f->ops->fd_read)
		return f->ops->fd_read(f, buf, pos, len);
	else
		return EOPNOTSUPP;
}

ssize_t sys_write(int fd, const void *buf, off_t pos, ssize_t len)
{
	struct file_descriptor *f = get_fd(get_current_ioctx(true), fd);

	dprintf("sys_write: fd %d\n", fd);
	
	if(!f)
		return EBADF;

	CHECK_SYS_ADDR(buf)

	if (f->ops->fd_write)
		return f->ops->fd_write(f, buf, pos, len);
	else
		return EOPNOTSUPP;
}

int sys_ioctl(int fd, int op, void *buf, size_t len)
{
	struct file_descriptor *f = get_fd(get_current_ioctx(true), fd);

	dprintf("sys_ioctl: fd %d\n", fd);
	
	if(!f)
		return EBADF;

	CHECK_SYS_ADDR(buf)

	if (f->ops->fd_ioctl)
		return f->ops->fd_ioctl(f, op, buf, len);
	else
		return EOPNOTSUPP;
}

int sys_close(int fd)
{
	struct ioctx *ic = get_current_ioctx(true);
	struct file_descriptor *f = get_fd(ic, fd);
	
	if (!f)
		return EBADF;
	
	if (f->ops->fd_close)
		return f->ops->fd_close(f, fd, ic);
	else
		return EOPNOTSUPP;
}