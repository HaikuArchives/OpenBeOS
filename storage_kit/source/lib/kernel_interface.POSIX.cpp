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

#include <Entry.h>
	// entry_ref

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
StorageKit::open( const char *path, OpenFlags flags, FileDescriptor &result ) {
	if (path == NULL) {
		result = -1;
		return B_BAD_VALUE;
	}

	// This version of the function may not be called with the O_CREAT flag
	if (flags & O_CREAT)
		return B_BAD_VALUE;

	// Open file and return the proper error code
	result = ::open(path, flags);
	return (result == -1) ? errno : B_OK ;
}



/*! Same as the other version of open() except the file is created with the
	permissions given by creationFlags if it doesn't exist. */
status_t
StorageKit::open( const char *path, OpenFlags flags, CreationFlags creationFlags,
	FileDescriptor &result )
{
	if (path == NULL) {
		result = -1;
		return B_BAD_VALUE;
	}

	// Open/Create the file and return the proper error code
	result = ::open(path, flags | O_CREAT, creationFlags);
	return (result == -1) ? errno : B_OK ;
}

status_t
StorageKit::close(StorageKit::FileDescriptor file) {
	return (::close(file) == -1) ? errno : B_OK ;
}

StorageKit::FileDescriptor
StorageKit::dup(StorageKit::FileDescriptor file) {
	return ::dup(file);
}


ssize_t
StorageKit::read_attr ( StorageKit::FileDescriptor file, const char *attribute, 
				 uint32 type, off_t pos, void *buf, size_t count ) {
	if (attribute == NULL || buf == NULL)
		return B_BAD_VALUE;
			
	return fs_read_attr ( file, attribute, type, pos, buf, count );
}

ssize_t
StorageKit::write_attr ( StorageKit::FileDescriptor file, const char *attribute, 
				  uint32 type, off_t pos, const void *buf, 
				  size_t count ) {
	if (attribute == NULL || buf == NULL)
		return B_BAD_VALUE;
			
	return fs_write_attr ( file, attribute, type, pos, buf, count );
}

status_t
StorageKit::remove_attr ( StorageKit::FileDescriptor file, const char *attr ) {
	if (attr == NULL)
		return B_BAD_VALUE;	

	// fs_remove_attr is supposed to set errno properly upon failure,
	// but currently does not appear to. It isn't set consistent
	// with what is returned by R5::BNode::RemoveAttr(), and it isn't
	// set consistent with what the BeBook's claims it is set to either.
	return fs_remove_attr ( file, attr ) == -1 ? errno : B_OK ;
}

StorageKit::Dir
StorageKit::open_attr_dir( FileDescriptor file ) {
	return fs_fopen_attr_dir( file );
}


void
StorageKit::rewind_attr_dir( Dir dir )
{
	if (dir != NULL)
		fs_rewind_attr_dir( dir );
}

StorageKit::DirEntry*
StorageKit::read_attr_dir( Dir dir ) {
	return (dir == NullDir) ? NULL : fs_read_attr_dir( dir );
}

status_t
StorageKit::close_attr_dir ( Dir dir )
{
	if (dir == NULL)
		return B_BAD_VALUE;
	return (fs_close_attr_dir( dir ) == -1) ? errno : B_OK ;
}

status_t
StorageKit::stat_attr( FileDescriptor file, const char *name, AttrInfo *ai )
{
	if (name == NULL || ai == NULL)
		return B_BAD_VALUE;
		
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
StorageKit::lock(FileDescriptor file, OpenFlags mode, FileLock *lock) {
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
StorageKit::get_stat(const char *path, Stat *s) {
	if (path == NULL || s == NULL)
		return B_BAD_VALUE;
		
	return (::stat(path, s) == -1) ? errno : B_OK ;
}

status_t
StorageKit::get_stat(FileDescriptor file, Stat *s) {
	if (s == NULL)
		return B_BAD_VALUE;
		
	return (::fstat(file, s) == -1) ? errno : B_OK ;
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

status_t
StorageKit::open_dir( const char *path, Dir &result ) {
//	cout << endl << "open_dir()" << endl;
	result = ::opendir( path );
	return (result == NULL) ? errno : B_OK ;
}

StorageKit::DirEntry*
StorageKit::read_dir( Dir dir ) {
	return (dir == NullDir) ? NULL : readdir(dir) ;
}

status_t
StorageKit::rewind_dir( Dir dir ) {
	if (dir == NullDir)
		return B_BAD_VALUE;
	else {
		::rewinddir(dir);
		return B_OK;
	}
}

status_t
StorageKit::find_dir( Dir dir, const char *name, DirEntry *&result ) {
	if (dir == NullDir || name == NULL)
		return B_BAD_VALUE;
	
	status_t status;
	
	status = StorageKit::rewind_dir(dir);
	if (status == B_OK) {
		for (	result = StorageKit::read_dir(dir);
				result != NULL;
				result = StorageKit::read_dir(dir)	)
		{
			if (strcmp(result->d_name, name) == 0)
				return B_OK;
		}
		status = B_ENTRY_NOT_FOUND;
	}
	
	result = NULL;
	return status;
}

status_t
StorageKit::dup_dir( Dir dir, Dir &result ) {
	status_t status = B_ERROR;

	if (dir == NullDir) {
		status = B_BAD_VALUE;
	} else {

		// We need to find the entry for "." in the
		// given directory, get its full path, and
		// open it as a directory.

		// Find "."
		DirEntry *entry;
		status = StorageKit::find_dir(dir, ".", entry);

		if (status == B_OK) {
		
			// Convert it to an absolute pathname
			char path[B_PATH_NAME_LENGTH+1];
			status = StorageKit::entry_ref_to_path(entry->d_pdev,
				entry->d_pino, ".", path, B_PATH_NAME_LENGTH+1);
				
			if (status == B_OK) {			
				// Open it
				status = StorageKit::open_dir( path, result );				
			}			
		}		
	}
	
	
	if (status != B_OK) {
		result = NullDir;
	}

	return status;		 	
}


status_t
StorageKit::close_dir( Dir dir ) {
//	cout << endl << "close_dir()" << endl;
	return (::closedir(dir) == -1) ? errno : B_OK;	
}


ssize_t
StorageKit::read_link( const char *path, char *result, int size ) {
	if (result == NULL || size < 1)
		return B_BAD_VALUE;
		
	int len = ::readlink(path, result, size-1);
	if (len == -1) {
		result[0] = 0;		// Null terminate
		return errno;	
	} else {
		result[len] = 0;	// Null terminate
		return len;
	}
	
}


status_t
StorageKit::entry_ref_to_path( const struct entry_ref *ref, char *result, int size ) {
	if (ref == NULL) {
		return B_BAD_VALUE;
	} else {
		return entry_ref_to_path(ref->device, ref->directory, ref->name, result, size);
	}
}

status_t
StorageKit::entry_ref_to_path( dev_t device, ino_t directory, const char *name, char *result, int size ) {
//	printf("device = %ld \n", device);
//	printf("dir    = %lld \n", directory);
//	printf("name   = '%s' \n", name);


	if (result == NULL || size < 1)
		return B_BAD_VALUE;

	// A list of possible relative pathnames to our EntryRefToPath program
	const char app1[] = "tools/EntryRefToPath";
	const char app2[] = "../tools/EntryRefToPath";
	const char app3[] = "../../tools/EntryRefToPath";
	const char app4[] = "storage_kit/tools/EntryRefToPath";
	const char app5[] = "../../../tools/EntryRefToPath";
	const char app6[] = "open-beos/storage_kit/tools/EntryRefToPath";
	const char app7[] = "OpenBeOS/storage_kit/tools/EntryRefToPath";
	const char app8[] = "/boot/home/config/lib/EntryRefToPath";
	const int APP_COUNT = 8;
	const char *app[APP_COUNT] = { app1, app2, app3, app4, app5, app6, app7, app8 };
	char *app_path = NULL;
	
	// Try to find our EntryRefToPath program
	for (int i = 0; i < APP_COUNT; i++) {
		int fd = ::open(app[i], O_RDONLY);
		if (fd != -1) {
			::close(fd);
			app_path = (char *)app[i];
			break;
		}
	}
	
	if (app_path == NULL) {
		result[0] = 0;
		return B_ERROR;
	}
	
	
	// Attempt to invoke our EntryRefToPath program using the
	// various pathnames from above. If one succeeds, it will
	// return B_OK and the others will not be tried.
	char cmd[B_PATH_NAME_LENGTH];

	// Create our command line
	sprintf(cmd, "%s %ld %lld %s", app_path, device, directory, name);

	// Invoke the command, grabbing a pipe to its stdout
	errno = B_OK;
	FILE* file = popen(cmd, "r");

	// If the command succeeded, read everything piped out to us
	if (file != NULL && (int)file != -1) {
		char *pos = result;
		int bytes = 0;
		int bytesLeft = size-1;	// Leave room for the NULL

		// Do buffered reads just in case
		while (!feof(file) && bytesLeft > 0) {
			bytes = fread(pos, 1, bytesLeft, file);
			pos += bytes;
			bytesLeft -= bytes;
		}
		pos[0] = 0;	// Null terminate the string we just read
		
//		printf("bytes == %d\n", pos-result);

		pclose(file);
			
		// If nothing is read from the pipe, then the call must have
		// failed, because EntryRefToPipe will *always* print out
		// at least one character. Unfortunately, we have to check
		// this because popen() doesn't seem to think it's an error
		// when the given command cannot be executed (we still get
		// a valid pipe instead of an error code).
		if (bytesLeft < size-1) {
			// Check for an error. The first character will *always* be
			// a '/' character if there's no error (because the pathname
			// is absolute). If there's no error, we're done.
			if (result[0] != '/') {
				status_t status;
//				printf("result = '%s'\n", result);
				if (sscanf(result, "%ld\n%s", &status, NULL) < 1) {
					status = -12;
				}
				result[0] = 0;
				return status;
			} else {
				return B_OK;
			}
		}
	}
	
	result[0] = 0;
	return -13;
	
}

status_t
StorageKit::dir_to_self_entry_ref( Dir dir, entry_ref *result ) {
	if (dir == StorageKit::NullDir || result == NULL)
		return B_BAD_VALUE;

/*		
	// Here we're ignoring the fact that we're not supposed to know
	// what exactly a StorageKit::Dir is, since it's much more efficient
	result->device = dir->ent.d_pdev;
	result->directory = dir->ent.d_pino;
	return result->set_name( dir->ent.d_name );
*/

	// Convert our directory to an entry_ref 
	StorageKit::rewind_dir(dir);	
	StorageKit::DirEntry *entry;
	for (	entry = StorageKit::read_dir(dir);
			entry != NULL;
			entry = StorageKit::read_dir(dir)	) {
		if (strcmp(entry->d_name, ".") == 0) {
			result->device = entry->d_dev;
			result->directory = entry->d_ino;
			result->set_name(".");
			return B_OK;					
		}
	}
	return B_ENTRY_NOT_FOUND;
	
}

status_t
StorageKit::dir_to_path( Dir dir, char *result, int size ) {
	if (dir == StorageKit::NullDir || result == NULL)
		return B_BAD_VALUE;

	entry_ref entry;
	status_t status;
	
	status = dir_to_self_entry_ref(dir, &entry);
	if (status != B_OK)
		return status;
		
	return entry_ref_to_path(&entry, result, size);
}

bool
StorageKit::entry_ref_is_root_dir( entry_ref &ref ) {
	return ref.directory == 1 && ref.device == 1 && ref.name[0] == '.' && ref.name[1] == 0;
}

status_t
StorageKit::rename(const char *oldPath, const char *newPath) {
	if (oldPath == NULL || newPath == NULL)
		return B_BAD_VALUE;
	
	return (::rename(oldPath, newPath) == -1) ? errno : B_OK ;		
}

/*! Removes path from the filesystem. */
status_t
StorageKit::remove(const char *path) {
	if (path == NULL)
		return B_BAD_VALUE;
	
	return (::remove(path) == -1) ? errno : B_OK ;		
}

