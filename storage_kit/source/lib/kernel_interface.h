//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: kernel_interface.h
//  Description: This is the internal interface used by the Storage Kit
//  to communicate with the kernel
//---------------------------------------------------------------------
#ifndef _sk_kernel_interface_h_
#define _sk_kernel_interface_h_

#include <SupportKit.h>
#include "Error.h"

typedef struct attr_info;

namespace StorageKit {

// Modes for opening files
typedef enum {
	READ,
	WRITE,
	READ_WRITE
} OpenMode;

// File descriptor type
typedef int FileDescriptor;

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


FileDescriptor Open(const char *path, OpenMode mode) throw (Error);
int Close(FileDescriptor file);

ssize_t read_attr(FileDescriptor file, const char *attribute, uint32 type, 
				off_t pos, void *buf, size_t count);

ssize_t write_attr (FileDescriptor file, const char *attribute, uint32 type, 
             off_t pos, const void *buf, size_t count );

int remove_attr(FileDescriptor file, const char *attr );

void rewind_attr_dir(void *cookie );

int close_attr_dir(void *cookie );

int stat_attr( FileDescriptor file, const char *name, attr_info *ai );

}

#endif
