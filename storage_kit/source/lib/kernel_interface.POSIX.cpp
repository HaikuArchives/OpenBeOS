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

#include <fsproto.h>

#include <stdio.h>
	// open, close
	
#include <errno.h>
	// errno

#include <string.h>
	// strerror()
	
#include <fs_attr.h>
	//  BeOS's C-based attribute functions

#include "Error.h"
	// StorageKit::Error
	
// This is just for cout while developing; shouldn't need it
// when all is said and done.
#include <iostream>

// Converts the given error code into a BeOS status_t error code
status_t PosixErrnoToBeOSError() {
//	cout << endl << "PosixErrnoToBeOSError() -- errno == " << errno << " == 0x" << hex << errno << dec << endl << endl;
	switch (errno) {
		case ENOMEM:
			return B_NO_MEMORY;
				
		case EFAULT:
			return B_BAD_ADDRESS;
				
		case EACCES:
			return B_PERMISSION_DENIED;
			
		case EAGAIN:
			return B_BUSY;
			
		default:
			return errno;		
	}	
}

#define ERROR_CASE(number,string) \
case number: \
	throw new StorageKit::Error(errno, string); \
	break; 

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

		case EACCES:
			throw new StorageKit::Error(errno, "Operation is prohibited by locks held by other processes.");
			break;
			
		ERROR_CASE(EAGAIN, " Operation is prohibited because the file has been memory-mapped by another process")
		
		ERROR_CASE(EBADF, "Operation is prohibited because the file has been memory-mapped by another process")
		
		default:
			throw new StorageKit::Error(errno, strerror(errno));
			break;

	}
}

status_t
StorageKit::open( const char *path, Mode mode, FileDescriptor &result ) {
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
	result = ::open(path, posix_flags);
	
	// Check for errors
	return (result == -1) ? PosixErrnoToBeOSError() : B_OK ;
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

status_t
StorageKit::remove_attr ( StorageKit::FileDescriptor file, const char *attr ) {
	// fs_remove_attr is supposed to set errno properly upon failure,
	// but currently does not appear to. It isn't set consistent
	// with what is returned by R5::BNode::RemoveAttr(), and it isn't
	// set consistent with what the BeBook's claims it is set to either.
	return fs_remove_attr ( file, attr ) == -1 ? errno : B_OK ;
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
	return (fs_close_attr_dir( dir ) == -1) ? errno : B_OK ;
}

status_t
StorageKit::stat_attr( FileDescriptor file, const char *name, AttrInfo *ai )
{
	return (fs_stat_attr( file, name, ai ) == -1) ? errno : B_OK ;
}


void DumpLock(StorageKit::FileLock &lock) {
	cout << endl;
	cout << "type   == ";
	switch (lock.l_type) {
		case F_RDLCK:
			cout << "F_RDLCK";
			break;
			
		case F_WRLCK:
			cout << "F_WRLCK";
			break;
			
		case F_UNLCK:
			cout << "F_UNLCK";
			break;
			
		default:
			cout << lock.l_type;
			break;
	}
	cout << endl;

	cout << "whence == " << lock.l_whence << endl;
	cout << "start  == " << lock.l_start << endl;
	cout << "len    == " << lock.l_len << endl;
	cout << "pid    == " << lock.l_pid << endl;
	cout << endl;
}

// As best I can tell, fcntl(fd, F_SETLK, lock) and fcntl(fd, F_GETLK, lock)
// are unimplemented in BeOS R5. Thus locking will have to wait for the new
// kernel. I believe this function would work if fcntl() worked correctly.
status_t
StorageKit::lock(FileDescriptor file, Mode mode, FileLock *lock) {
	return B_FILE_ERROR;

/*
	if (lock == NULL)
		return B_BAD_VALUE;

//	DumpLock(*lock);
	short lock_type;
	switch (mode) {
		case READ:
			lock_type = F_RDLCK;
			break;
			
		case WRITE:
		case READ_WRITE:
		default:
			lock_type = F_WRLCK;
			break;
	}

	
//	lock->l_type = F_UNLCK;
	lock->l_type = lock_type;
	lock->l_whence = SEEK_SET;
	lock->l_start = 0;				// Beginning of file...
	lock->l_len = 0;				// ...to end of file
	lock->l_pid = 0;				// Don't really care :-)

//	DumpLock(*lock);
	::fcntl(file, F_GETLK, lock);
//	DumpLock(*lock);
	if (lock->l_type != F_UNLCK) {
		return errno;
	} 
	
//	lock->l_type = F_RDLCK;
	lock->l_type = lock_type;
//	DumpLock(*lock);
	
	errno = 0;
	
	return (::fcntl(file, F_SETLK, lock) == 0) ? B_OK : PosixErrnoToBeOSError();
*/
}

// As best I can tell, fcntl(fd, F_SETLK, lock) and fcntl(fd, F_GETLK, lock)
// are unimplemented in BeOS R5. Thus locking will have to wait for the new
// kernel. I believe this function would work if fcntl() worked correctly.
status_t
StorageKit::unlock(FileDescriptor file, FileLock *lock) {
	return B_FILE_ERROR;

/*
	if (lock == NULL)
		return B_BAD_VALUE;
	
	lock->l_type = F_UNLCK;
	
	return (::fcntl(file, F_SETLK, lock) == 0) ? B_OK : PosixErrnoToBeOSError() ;
*/
}

status_t
StorageKit::get_stat(FileDescriptor file, Stat *s) {
	if (s == NULL)
		return B_BAD_VALUE;
		
	return (::fstat(file, s) == -1) ? PosixErrnoToBeOSError() : B_OK ;
}


status_t
StorageKit::set_stat(FileDescriptor file, Stat &s, StatMember what) {
	int result;
	
	switch (what) {
		case WSTAT_MODE:
			// For some stupid reason, BeOS R5 has no fchmod function, just
			// chmod(char *filename, ...), so for the moment we're screwed.
//			result = fchmod(file, s.st_mode);
//			break;
			return B_BAD_VALUE;
			
		case WSTAT_UID:
			result = ::fchown(file, s.st_uid, 0xFFFFFFFF);
			break;
			
		case WSTAT_GID:
			result = ::fchown(file, 0xFFFFFFFF, s.st_gid);
			break;
			
		// These would all require a call to utime(char *filename, ...), but
		// we have no filename, only a file descriptor, so they'll have to
		// wait until our new kernel shows up (or we decide to try calling
		// into the R5 kernel ;-)
		case WSTAT_ATIME:
		case WSTAT_MTIME:
		case WSTAT_CRTIME:
			return B_BAD_VALUE;
			
		default:
			return B_BAD_VALUE;	
	}
	
	return (result == -1) ? PosixErrnoToBeOSError() : B_OK ;
}

status_t
StorageKit::sync( FileDescriptor file ) {
	return (fsync(file) == -1) ? PosixErrnoToBeOSError() : B_OK ;
}
