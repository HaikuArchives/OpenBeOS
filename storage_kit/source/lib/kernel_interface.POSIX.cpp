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