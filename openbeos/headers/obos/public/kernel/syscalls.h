/*
** Copyright 2001, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#ifndef _LIBSYS_SYSCALLS_H
#define _LIBSYS_SYSCALLS_H

#include <ktypes.h>
#include <types.h>
#include <defines.h>
#include <resource.h>
#include <vfs_types.h>
#include <vm_types.h>
#include <sem_types.h>
#include <port_types.h>
#include <thread_types.h>

#ifdef __cplusplus
extern "C" {
#endif

int sys_null();

/* fs api */
int sys_mount(const char *path, const char *device, const char *fs_name, void *args);
int sys_unmount(const char *path);
int sys_sync();
int sys_open(const char *path, stream_type st, int omode);
int sys_close(int fd);
int sys_fsync(int fd);
ssize_t sys_read(int fd, void *buf, off_t pos, ssize_t len);
ssize_t sys_write(int fd, const void *buf, off_t pos, ssize_t len);
int sys_seek(int fd, off_t pos, seek_type seek_type);
int sys_ioctl(int fd, int op, void *buf, size_t len);
int sys_create(const char *path, stream_type stream_type);
int sys_unlink(const char *path);
int sys_rename(const char *oldpath, const char *newpath);
int sys_rstat(const char *path, struct file_stat *stat);
int sys_wstat(const char *path, struct file_stat *stat, int stat_mask);
char *sys_getcwd(char* buf, size_t size);
int sys_setcwd(const char* path);
int sys_dup(int fd);
int sys_dup2(int ofd, int nfd);

bigtime_t sys_system_time();
int sys_snooze(bigtime_t time);
int sys_getrlimit(int resource, struct rlimit * rlp);
int sys_setrlimit(int resource, const struct rlimit * rlp);

/* sem functions */
sem_id sys_sem_create(int count, const char *name);
int sys_sem_delete(sem_id id);
int sys_sem_acquire(sem_id id, int count);
int sys_sem_acquire_etc(sem_id id, int count, int flags, bigtime_t timeout);
int sys_sem_release(sem_id id, int count);
int sys_sem_release_etc(sem_id id, int count, int flags);
int sys_sem_get_count(sem_id id, int32* thread_count);
int sys_sem_get_sem_info(sem_id id, struct sem_info *info);
int sys_sem_get_next_sem_info(proc_id proc, uint32 *cookie, struct sem_info *info);
int sys_set_sem_owner(sem_id id, proc_id proc);

int sys_proc_get_table(struct proc_info *pi, size_t len);
thread_id sys_get_current_thread_id();
void sys_exit(int retcode);
proc_id sys_proc_create_proc(const char *path, const char *name, char **args, int argc, int priority);
thread_id sys_thread_create_thread(const char *name, int (*func)(void *args), void *args);
int sys_thread_wait_on_thread(thread_id tid, int *retcode);
int sys_thread_suspend_thread(thread_id tid);
int sys_thread_resume_thread(thread_id tid);
int sys_thread_kill_thread(thread_id tid);
int sys_proc_kill_proc(proc_id pid);
proc_id sys_get_current_proc_id();
int sys_proc_wait_on_proc(proc_id pid, int *retcode);
region_id sys_vm_create_anonymous_region(const char *name, void **address, int addr_type,
	addr size, int wiring, int lock);
region_id sys_vm_clone_region(const char *name, void **address, int addr_type,
	region_id source_region, int mapping, int lock);
region_id sys_vm_map_file(const char *name, void **address, int addr_type,
	addr size, int lock, int mapping, const char *path, off_t offset);
int sys_vm_delete_region(region_id id);
int sys_vm_get_region_info(region_id id, vm_region_info *info);

/* kernel port functions */
port_id		sys_port_create(int32 queue_length, const char *name);
int			sys_port_close(port_id id);
int			sys_port_delete(port_id id);
port_id		sys_port_find(const char *port_name);
int			sys_port_get_info(port_id id, struct port_info *info);
int		 	sys_port_get_next_port_info(proc_id proc, uint32 *cookie, struct port_info *info);
ssize_t		sys_port_buffer_size(port_id port);
ssize_t		sys_port_buffer_size_etc(port_id port, uint32 flags, bigtime_t timeout);
int32		sys_port_count(port_id port);
ssize_t		sys_port_read(port_id port, int32 *msg_code, void *msg_buffer, size_t buffer_size);
ssize_t		sys_port_read_etc(port_id port,	int32 *msg_code, void *msg_buffer, size_t buffer_size, uint32 flags, bigtime_t timeout);
int			sys_port_set_owner(port_id port, proc_id proc);
int			sys_port_write(port_id port, int32 msg_code, void *msg_buffer, size_t buffer_size);
int			sys_port_write_etc(port_id port, int32 msg_code, void *msg_buffer, size_t buffer_size, uint32 flags, bigtime_t timeout);

/* atomic_* ops (needed for cpus that dont support them directly) */
int sys_atomic_add(int *val, int incr);
int sys_atomic_and(int *val, int incr);
int sys_atomic_or(int *val, int incr);
int sys_atomic_set(int *val, int set_to);
int sys_test_and_set(int *val, int set_to, int test_val);

int sys_sysctl(int *, uint, void *, size_t *, void *, size_t);
int sys_socket(int, int, int);

#ifdef __cplusplus
}
#endif

#endif

