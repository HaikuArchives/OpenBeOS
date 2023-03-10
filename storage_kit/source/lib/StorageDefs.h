//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//---------------------------------------------------------------------
/*!
	\file StorageDefs.h
	Miscellaneous Storage Kit definitions and includes.
*/

#ifndef __sk_def_storage_h__
#define __sk_def_storage_h__

#include <fcntl.h>
#include <sys/param.h>
#include <limits.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif // USE_OPENBEOS_NAMESPACE

// Limits
#define B_DEV_NAME_LENGTH		128
#define B_FILE_NAME_LENGTH		NAME_MAX
#define B_PATH_NAME_LENGTH 		MAXPATHLEN
#define B_ATTR_NAME_LENGTH		(B_FILE_NAME_LENGTH-1)
#define B_MIME_TYPE_LENGTH		(B_ATTR_NAME_LENGTH - 15)
#define B_MAX_SYMLINKS			SYMLINK_MAX


// Open Modes
#define B_READ_ONLY 	O_RDONLY	// read only 
#define B_WRITE_ONLY 	O_WRONLY 	// write only 
#define B_READ_WRITE	O_RDWR   	// read and write

#define	B_FAIL_IF_EXISTS	O_EXCL		// exclusive create 
#define B_CREATE_FILE		O_CREAT		// create the file 
#define B_ERASE_FILE		O_TRUNC		// erase the file's data 
#define B_OPEN_AT_END	   	O_APPEND	// point to the end of the data 


// Node Flavors
enum node_flavor {
  B_FILE_NODE		= 0x01,
  B_SYMLINK_NODE	= 0x02,
  B_DIRECTORY_NODE	= 0x04,
  B_ANY_NODE		= 0x07
};



#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif	// USE_OPENBEOS_NAMESPACE


#endif __sk_def_storage_h__