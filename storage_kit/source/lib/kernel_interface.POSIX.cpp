//----------------------------------------------------------------------
// kernel_interface.POSIX.cpp
// Initial implementation of our kernel interface with calls to 
// POSIX api. This will later be replaced with a version that
// makes syscalls into the actual kernel
//----------------------------------------------------------------------

#include "kernel_interface.h"

#include <stdio.h>
	// fopen, fclose


fd storage_kit::open(const char *path, storage_kit::open_mode mode) {
	char posix_mode[4]		// Posix modes are at most 3 characters long

	// Choose the proper posix mode
	switch (mode) {
		case READ:
			mode = "rb";	// Binary open read mode
			break;
		case WRITE:
			mode = "wb";	// Binary open write mode
			break;
		case READ_WRITE:
			mode = "rb+"	// Binary open read/write mode
			break;	
	}

	// Open the file
	FILE *file = fopen(path, posix_mode);

	// Check for validity
	if (file == NULL)
		{}	//	An error occurred. How do we want to handle these?
	else
		return file;
		
}


storage_kit::close(storage_kit::fd file) {
	return ( fclose(file) == 0 ) ? B_OK : B_ERROR;
}