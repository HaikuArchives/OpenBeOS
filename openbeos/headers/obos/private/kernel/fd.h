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
#include <sys/stat.h>


/* These are defined later but need to be here... */
struct file_descriptor;
struct vnode;

struct io_context {
	struct vnode *cwd;
	mutex	io_mutex;
	int		table_size;
	int		num_used_fds;
	struct file_descriptor **fds;
};

struct fd_ops {
	char *fs_name;
	ssize_t (*fd_read) (struct file_descriptor *, void *buffer, off_t pos, size_t *length);
	ssize_t (*fd_write)(struct file_descriptor *, const void *buffer, off_t pos, size_t *length);
	int     (*fd_ioctl)(struct file_descriptor *, ulong op, void *buffer, size_t length);
//	int     (*fd_poll)(struct file_descriptor *, int);
	int     (*fd_stat)(struct file_descriptor *, struct stat *);
	int     (*fd_close)(struct file_descriptor *, int, struct io_context *);
	void    (*fd_free)(struct file_descriptor *);
};

struct file_descriptor {
	int32	type;               /* descriptor type */
	int32	open_count;
	int32	ref_count;
	struct fd_ops *ops;
	struct vnode *vnode;
	void	*cookie;
};

//#define DTYPE_VNODE        1
//#define DTYPE_SOCKET       2

/* Types of file descriptors we can create */

enum fd_types {
	FDTYPE_VNODE = 1,	// ToDo: to be removed shortly
	FDTYPE_FILE	= 1,
	FDTYPE_ATTR,
	FDTYPE_DIR,
	FDTYPE_ATTRDIR,
	FDTYPE_INDEX,
	FDTYPE_QUERY,
	FDTYPE_SOCKET
};


/* Prototypes */

struct file_descriptor *alloc_fd(void);
int new_fd(struct io_context *, struct file_descriptor *);
struct file_descriptor *get_fd(struct io_context *, int);
void remove_fd(struct io_context *, int);
void put_fd(struct file_descriptor *);
void free_fd(struct file_descriptor *);
static struct io_context *get_current_io_context(bool kernel);


/** @fn int sys_ioctl(int fd, ulong cmd, void *data, size_t len)
 * The kernel version of ioctl()
 */
int sys_ioctl(int, ulong, void *, size_t);
/** @fn int user_ioctl(int fd, ulong cmd, void *data, size_t len)
 * The user_ioctl() function to interface with sys_ioctl()
 */
int user_ioctl(int, ulong, void *, size_t);
int user_fstat(int, struct stat *);


/* Inlines */

static __inline struct io_context *get_current_io_context(bool kernel)
{
	if (kernel)
		return proc_get_kernel_proc()->ioctx;

	return thread_get_current_thread()->proc->ioctx;
}

#endif /* _FILE_H */
