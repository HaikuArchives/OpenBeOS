/* file.h
 *
 * File Descriptors
 */
 

#ifndef _FILE_H
#define _FILE_H

#include <sem.h>
#include <lock.h>
#include <atomic.h>
#include <memheap.h>

/* Types of file descriptors we can create */
#define DTYPE_VNODE        1
#define DTYPE_SOCKET       2

/* These are defined later but need to be here... */
struct file_descriptor;
struct vnode;

struct ioctx {
	struct vnode *cwd;
	mutex io_mutex;
	int table_size;
	int num_used_fds;
	struct file_descriptor **fds;
};

struct fd_ops {
	char *fs_name;
	ssize_t (*fd_read) (struct file_descriptor *, void *, off_t, size_t *);
	ssize_t (*fd_write)(struct file_descriptor *, const void *, off_t, size_t *);
	int     (*fd_ioctl)(struct file_descriptor *, int, void *, size_t);
//	int     (*fd_poll)(struct file_descriptor *, int);
//	int     (*fd_stat)(struct file_descriptor *, ...); XXX - complete me :(
	int     (*fd_close)(struct file_descriptor *, int, struct ioctx *);
	void    (*fd_cleanup)(struct file_descriptor *);
};

struct file_descriptor {
	int fd_type;               /* descriptor type */
	int ref_count;
	struct fd_ops *ops;
	struct vnode *vnode;
	void *cookie;
};


struct file_descriptor *alloc_fd(void);
int new_fd(struct ioctx *, struct file_descriptor *);
struct file_descriptor *get_fd(struct ioctx *, int);
void remove_fd(struct ioctx *, int);
static void put_fd(struct file_descriptor *);
void free_fd(struct file_descriptor *);
static struct ioctx *get_current_ioctx(bool);

static __inline struct ioctx *get_current_ioctx(bool kernel)
{
	struct ioctx *ioctx;

	if(kernel) {
		ioctx = proc_get_kernel_proc()->ioctx;
	} else {
		ioctx = thread_get_current_thread()->proc->ioctx;
	}
	return ioctx;
}

/* Run a cleanup (fd_free) routine if there is one and free structure, but only
 * if we've just removed the final reference to it :)
 */
static __inline void put_fd(struct file_descriptor *f)
{
	if(atomic_add(&f->ref_count, -1) == 1) {
		if (f->ops->fd_cleanup)
			f->ops->fd_cleanup(f);
		kfree(f);
	}
}

#endif /* _FILE_H */
