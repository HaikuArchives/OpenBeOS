//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: kernel_interface.POSIX.cpp
//  Description: Initial implementation of our kernel interface with
//  calls to the POSIX api. This will later be replaced with a version
//  that makes syscalls into the actual kernel
//----------------------------------------------------------------------

#include "kernel_interface.h"

#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
	// open, close
	
#include <errno.h>
	// errno
	
#include <fs_attr.h>
	//  BeOS's C-based attribute functions

#include "Error.h"
	// SKError

// Used to throw the appropriate error as noted by errno
void ThrowError() {
	switch (errno) {
		case ENAMETOOLONG:
			throw new StorageKit::Error(errno, "Specified pathname is too long");
			break;
			
		case ENOENT:
			throw new StorageKit::EEntryNotFound();
			break;

		case ENOTDIR:
			throw new StorageKit::Error(errno, "Some portion of the path that was expected to be a directory was in fact not");
			break;

/*       ENXIO  O_NONBLOCK | O_WRONLY is set, the named file is a FIFO and no process  has  the  file  open  for
              reading.  Or, the file is a device special file and no corresponding device exists.

       ENODEV pathname  refers  to a device special file and no corresponding device exists.  (This is a Linux
              kernel bug - in this situation ENXIO must be returned.)

       EROFS  pathname refers to a file on a read-only filesystem and write access was requested.

       ETXTBSY
              pathname refers to an executable image which is currently being executed and  write  access  was
              requested.

       EFAULT pathname points outside your accessible address space.

       ELOOP  Too  many symbolic links were encountered in resolving pathname, or O_NOFOLLOW was specified but
              pathname was a symbolic link.

       ENOSPC pathname was to be created but the device containing pathname has no room for the new file.

       ENOMEM Insufficient kernel memory was available.

       EMFILE The process already has the maximum number of files open.

       ENFILE The limit on the total number of files open on the system has been reached.*/
       
       default:
			throw new StorageKit::Error(errno);
			break;
	}
}


StorageKit::FileDescriptor
StorageKit::open(const char *path, StorageKit::OpenMode mode) {
	// Choose the proper posix flags
	int posix_flags;
	switch (mode) { 
		case READ: 
			posix_flags = O_RDONLY;	// Read only
			break; 
		case WRITE: 
			posix_flags = O_WRONLY;	// Write only 
			break; 
		case READ_WRITE: 
			posix_flags = O_RDWR;	// Read/Write
			break;       
	}
	
	// Add in O_CREAT so the file will be created if necessary
//	posix_flags &= O_CREAT;
	
	// Choose rwxrwxrwx as default persmissions
//	mode_t posix_mode = S_IRWXU | S_IRWXG | S_IRWXO;
	
	// Open the file
	FileDescriptor result = ::open(path, posix_flags);
	
	// Check for errors
	if (result == -1)
		ThrowError();
	
	return result;
}


status_t
StorageKit::close(StorageKit::FileDescriptor file) {
	return ::close(file);
}

StorageKit::FileDescriptor
StorageKit::dup(StorageKit::FileDescriptor file) {
	return ::dup(file);
}


ssize_t
StorageKit::read_attr ( StorageKit::FileDescriptor file, const char *attribute, 
				 uint32 type, off_t pos, void *buf, size_t count ) {
  return fs_read_attr ( file, attribute, type, pos, buf, count );
}

ssize_t
StorageKit::write_attr ( StorageKit::FileDescriptor file, const char *attribute, 
				  uint32 type, off_t pos, const void *buf, 
				  size_t count ) {
  return fs_write_attr ( file, attribute, type, pos, buf, count );
}

int
StorageKit::remove_attr ( StorageKit::FileDescriptor file, const char *attr ) {
  int result = fs_remove_attr ( file, attr );
  if(result < B_OK) {
    ThrowError();
  }
  return result;
}

StorageKit::Dir*
StorageKit::open_attr_dir( FileDescriptor file ) {
	return fs_fopen_attr_dir( file );
}


void
StorageKit::rewind_attr_dir( Dir *dir )
{
	fs_rewind_attr_dir( dir );
}

StorageKit::DirEntry*
StorageKit::read_attr_dir( Dir* dir ) {
	return fs_read_attr_dir( dir );
}

status_t
StorageKit::close_attr_dir ( Dir* dir )
{
	int result = fs_close_attr_dir( dir );
	if( result < B_OK ) {
		ThrowError();
	}

	return result;
}

int
StorageKit::stat_attr( StorageKit::FileDescriptor file, const char *name, attr_info *ai )
{
  int result = fs_stat_attr( file, name, ai );
  if ( result < B_OK ) {
    ThrowError();
  }
  return result;
}
