//----------------------------------------------------------------------
// kernel_interface.h
// This is the internal interface used by the storage kit to 
// communicate with the kernel
//----------------------------------------------------------------------
#ifndef _sk_kernel_interface_h_
#define _sk_kernel_interface_h_

#include <SupportKit.h>	

namespace storage_kit {

// Modes for opening files
typedef enum {
	READ,
	WRITE,
	READ_WRITE
} open_mode;

// File descriptor type
typedef int fd;

//----------------------------------------------------------------------
// user_* functions pulled from NewOS's vfs.h (with the "user_" part removed)
//----------------------------------------------------------------------
//int mount(const char *path, const char *device, const char *fs_name, void *args);
//int unmount(const char *path);
//int sync(void); 
//int open(const char *path, stream_type st, int omode);
//int close(int fd);
//int fsync(int fd);
//ssize_t read(int fd, void *buf, off_t pos, ssize_t len);
//ssize_t write(int fd, const void *buf, off_t pos, ssize_t len);
//int seek(int fd, off_t pos, seek_type seek_type);
//int ioctl(int fd, int op, void *buf, size_t len);
//int create(const char *path, stream_type stream_type);
//int unlink(const char *path);
//int rename(const char *oldpath, const char *newpath);
//int rstat(const char *path, struct file_stat *stat);
//int wstat(const char *path, struct file_stat *stat, int stat_mask);
//int getcwd(char *buf, size_t size);
//int setcwd(const char* path);
//int dup(int fd);
//int dup2(int ofd, int nfd);
//----------------------------------------------------------------------


fd open(const char *path, open_mode mode);
status_t close(fd file);


}

#endif
