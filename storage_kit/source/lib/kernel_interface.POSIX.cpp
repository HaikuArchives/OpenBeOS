//----------------------------------------------------------------------
// kernel_interface.POSIX.cpp
// Initial implementation of our kernel interface with calls to 
// POSIX api. This will later be replaced with a version that
// makes syscalls into the actual kernel
//----------------------------------------------------------------------

#include "kernel_interface.h"

#include <fcntl.h>

#include <stdio.h>
	// open, close
	
#include <errno.h>
	// errno

#include "SKError.h"
	// SKError

// Used to throw the appropriate error as noted by errno
void throw_error()
{
	switch (errno)
	{
		case ENAMETOOLONG:
			throw new SKError(errno, "Specified pathname is too long");
			break;
			
		default:
			throw new SKError(errno);
			break;
	}
}


storage_kit::fd storage_kit::open(const char *path, storage_kit::open_mode mode) {
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
	fd result = ::open(path, posix_flags, posix_mode);
	
	// Check for errors
	if (result == -1)
		throw_error();
	
	return result;
}


int storage_kit::close(storage_kit::fd file) {
	return close(file);
}