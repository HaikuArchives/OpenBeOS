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
void ThrowError() throw (StorageKit::Error) {
	switch (errno) {
		case ENAMETOOLONG:
			throw new StorageKit::Error(errno, "Specified pathname is too long");
			break;
			
		default:
			throw new StorageKit::Error(errno);
			break;
	}
}


StorageKit::FileDescriptor StorageKit::Open(const char *path,
StorageKit::OpenMode mode) throw (StorageKit::Error) {
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
	posix_flags &= O_CREAT;
	
	// Choose rwxrwxrwx as default persmissions
	mode_t posix_mode = S_IRWXU | S_IRWXG | S_IRWXO;
	
	// Open the file
	FileDescriptor result = ::open(path, posix_flags, posix_mode);
	
	// Check for errors
	if (result == -1)
		ThrowError();
	
	return result;
}


int StorageKit::Close(StorageKit::FileDescriptor file) {
	return ::close(file);
}


ssize_t StorageKit::read_attr ( StorageKit::FileDescriptor file, const char *attribute, 
				 uint32 type, off_t pos, void *buf, size_t count ) {
  return fs_read_attr ( file, attribute, type, pos, buf, count );
}

ssize_t StorageKit::write_attr ( StorageKit::FileDescriptor file, const char *attribute, 
				  uint32 type, off_t pos, const void *buf, 
				  size_t count ) {
  return fs_write_attr ( file, attribute, type, pos, buf, count );
}

int StorageKit::remove_attr ( StorageKit::FileDescriptor file, const char *attr ) {
  int result = fs_remove_attr ( file, attr );
  if(result < B_OK) {
    ThrowError();
  }
  return result;
}

void StorageKit::rewind_attr_dir( void *cookie )
{
  fs_rewind_attr_dir( (DIR*)cookie );
}

int StorageKit::close_attr_dir ( void *cookie )
{
  int result = fs_close_attr_dir( (DIR*)cookie );
  if( result < B_OK ) {
    ThrowError();
  }

  return result;
}

int StorageKit::stat_attr( StorageKit::FileDescriptor file, const char *name, attr_info *ai )
{
  int result = fs_stat_attr( file, name, ai );
  if ( result < B_OK ) {
    ThrowError();
  }
  return result;
}
