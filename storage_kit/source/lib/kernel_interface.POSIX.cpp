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
#include "storage_support.h"

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

#include <OS.h>

// This is just for cout while developing; shouldn't need it
// when all is said and done.
#include <iostream>

// for convenience:
struct LongDIR : DIR { char _buffer[B_FILE_NAME_LENGTH]; };


class DirCache {
public:
	DirCache() {
//		fSemID = create_sem(1, "StorageKit_DirCache_Semaphore");
//		cout << "HEY!!!!!!!! fSemID == " << fSemID << endl;
		time(&fTime);		
	}
	
	void DoOutput() {
//		cout << "DirCache(" << fTime << ")" << endl;
	}
	
private:
	sem_id fSemID;
	time_t fTime;	// This is just a test. Won't actually be used.
	
};

DirCache dirCache;


// This function takes a file descriptor (assumed to be file descriptor
// for a directory), mallocs a new DIR struct, and sets the DIR's fd
// member to the given file descriptor. Returns a pointer to the new
// DIR or NULL if the function fails.
DIR*
fd_to_dir( StorageKit::FileDescriptor fd ) {
	DIR *result = (DIR*)malloc(sizeof(DIR) + NAME_MAX);
	result->fd = fd;
	return result;
}

/*	This function takes a DIR* (assumed to be acquired through a call to
	opendir() or fs_open_attr_dir), steals the file descriptor from it, and
	frees the DIR*, returning the file descriptor (or -1 if the function
	fails).
*/
StorageKit::FileDescriptor
dir_to_fd( DIR *dir ) {
	if (dir == NULL) {
		return -1;
	} else {
		StorageKit::FileDescriptor fd = dir->fd;
//		free(dir);
		return fd;
	}
}

//------------------------------------------------------------------------------
// File Functions
//------------------------------------------------------------------------------

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
//	return (result == -1) ? errno : B_OK ;
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
//	return (result == -1) ? errno : B_OK ;
	return (result == -1) ? errno : B_OK ;
}

status_t
StorageKit::close(StorageKit::FileDescriptor file) {
	return (::close(file) == -1) ? errno : B_OK ;
}

/*! \param fd the file descriptor
	\param buf the buffer to be read into
	\param len the number of bytes to be read
	\return the number of bytes actually read or an error code
*/
ssize_t
StorageKit::read(StorageKit::FileDescriptor fd, void *buf, ssize_t len)
{
	ssize_t result = (buf == NULL || len < 0 ? B_BAD_VALUE : B_OK);
	if (result == B_OK) {
		result = ::read(fd, buf, len);
		if (result == -1)
			result = errno;
	}
	return result;
}

/*! \param fd the file descriptor
	\param buf the buffer to be read into
	\param pos file position from which to be read
	\param len the number of bytes to be read
	\return the number of bytes actually read or an error code
*/
ssize_t
StorageKit::read(StorageKit::FileDescriptor fd, void *buf, off_t pos,
				 ssize_t len)
{
	ssize_t result = (buf == NULL || pos < 0 || len < 0 ? B_BAD_VALUE : B_OK);
	if (result == B_OK) {
		result = ::read_pos(fd, pos, buf, len);
		if (result == -1)
			result = errno;
	}
	return result;
}

/*! \param fd the file descriptor
	\param buf the buffer containing the data to be written
	\param len the number of bytes to be written
	\return the number of bytes actually written or an error code
*/
ssize_t
StorageKit::write(StorageKit::FileDescriptor fd, const void *buf, ssize_t len)
{
	ssize_t result = (buf == NULL || len < 0 ? B_BAD_VALUE : B_OK);
	if (result == B_OK) {
		result = ::write(fd, buf, len);
		if (result == -1)
			result = errno;
	}
	return result;
}

/*! \param fd the file descriptor
	\param buf the buffer containing the data to be written
	\param pos file position to which to be written
	\param len the number of bytes to be written
	\return the number of bytes actually written or an error code
*/
ssize_t
StorageKit::write(StorageKit::FileDescriptor fd, const void *buf, off_t pos,
				  ssize_t len)
{
	ssize_t result = (buf == NULL || pos < 0 || len < 0 ? B_BAD_VALUE : B_OK);
	if (result == B_OK) {
		result = ::write_pos(fd, pos, buf, len);
		if (result == -1)
			result = errno;
	}
	return result;
}

/*! \param fd the file descriptor
	\param pos the relative new position of the read/write pointer in bytes
	\param mode \c SEEK_SET/\c SEEK_END/\c SEEK_CUR to indicate that \a pos
		   is relative to the file's beginning/end/current read/write pointer
	\return the new position of the read/write pointer relative to the
			beginning of the file, or an error code
*/
off_t
StorageKit::seek(StorageKit::FileDescriptor fd, off_t pos,
				 StorageKit::SeekMode mode)
{
	off_t result = ::lseek(fd, pos, mode);
	if (result == -1)
		result = errno;
	return result;
}

/*! \param fd the file descriptor
	\return the position of the read/write pointer relative to the
			beginning of the file, or an error code
*/
off_t
StorageKit::get_position(StorageKit::FileDescriptor fd)
{
	off_t result = ::lseek(fd, 0, SEEK_CUR);
	if (result == -1)
		result = errno;
	return result;
}



StorageKit::FileDescriptor
StorageKit::dup(StorageKit::FileDescriptor file) {
	return ::dup(file);
}

/*!	If the supplied file descriptor is -1, the copy will be -1 as well and
	B_OK is returned.
	\param file the file descriptor to be duplicated
	\param result the variable the resulting file descriptor will be stored in
	\return B_OK, if everything went fine, or an error code.
*/
status_t
StorageKit::dup( FileDescriptor file, FileDescriptor& result )
{
	status_t error = B_OK;
	if (file == -1)
		result = -1;
	else {
		result = dup(file);
		if (result == -1)
			error = errno;
	}
	return error;
}

status_t
StorageKit::sync( FileDescriptor file ) {
	return (fsync(file) == -1) ? errno : B_OK ;
}

//! /todo Get rid of DumpLock() at some point (it's only for debugging)
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
	
	return (::fcntl(file, F_SETLK, lock) == 0) ? B_OK : errno;
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
	
	return (::fcntl(file, F_SETLK, lock) == 0) ? B_OK : errno ;
*/
}

status_t
StorageKit::get_stat(const char *path, Stat *s) {
	if (path == NULL || s == NULL)
		return B_BAD_VALUE;
		
	return (::lstat(path, s) == -1) ? errno : B_OK ;
}

status_t
StorageKit::get_stat(FileDescriptor file, Stat *s) {
	if (s == NULL)
		return B_BAD_VALUE;
		
	return (::fstat(file, s) == -1) ? errno : B_OK ;
}

status_t
StorageKit::get_stat(entry_ref &ref, Stat *result) {
	char path[B_PATH_NAME_LENGTH];
	status_t status;
	
	status = StorageKit::entry_ref_to_path(&ref, path, B_PATH_NAME_LENGTH);
	return (status != B_OK) ? status : StorageKit::get_stat(path, result);
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

		case WSTAT_SIZE:
			// For enlarging files the truncate() behavior seems to be not
			// precisely defined, but with a bit of luck it might come pretty
			// close to what we need.
			result = ::ftruncate(file, s.st_size);
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
	
	return (result == -1) ? errno : B_OK ;
}


//------------------------------------------------------------------------------
// Attribute Functions
//------------------------------------------------------------------------------



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

status_t
StorageKit::stat_attr( FileDescriptor file, const char *name, AttrInfo *ai )
{
	if (name == NULL || ai == NULL)
		return B_BAD_VALUE;
		
	return (fs_stat_attr( file, name, ai ) == -1) ? errno : B_OK ;
}


//------------------------------------------------------------------------------
// Attribute Directory Functions
//------------------------------------------------------------------------------

StorageKit::FileDescriptor
StorageKit::open_attr_dir( FileDescriptor file ) {
	// Open the dir and convert it to a plain file descriptor.
	return dir_to_fd( fs_fopen_attr_dir( file ) );
}


void
StorageKit::rewind_attr_dir( FileDescriptor dirFd ) {
	DIR *dir = fd_to_dir( dirFd );
	if (dir != NULL)
		fs_rewind_attr_dir( dir );
//	free(dir);
}

StorageKit::DirEntry*
StorageKit::read_attr_dir( FileDescriptor dirFd ) {
	DIR* dir = fd_to_dir(dirFd);
	if (dir == NULL)
		return NULL;

	DirEntry *result = fs_read_attr_dir( dir );
//	free(dir);
	return result;
}

status_t
StorageKit::close_attr_dir ( FileDescriptor dirFd )
{
	if (dirFd == StorageKit::NullFd)
		return B_BAD_VALUE;
		
	// Allocate a new DIR*, set it up with the proper file descriptor,
	// and then close it.
	DIR *dir = fd_to_dir(dirFd);
	if (dir == NULL)
		return B_NO_MEMORY;
		
	return (fs_close_attr_dir( dir ) == -1) ? errno : B_OK ;
}



//------------------------------------------------------------------------------
// Directory Functions
//------------------------------------------------------------------------------

status_t
StorageKit::open_dir( const char *path, FileDescriptor &result ) {
//	cout << endl << "open_dir()" << endl;
	result = NullFd;
	if (DIR *dir = ::opendir(path)) {
		result = dir->fd;
		free(dir);
	}
	return (result < 0) ? errno : B_OK ;
}

/*!	The parent directory must already exist.
	\param path the directory's path name
	\param mode the file permissions
	\return B_OK, if everything went fine, an error code otherwise
*/
status_t
StorageKit::create_dir( const char *path, mode_t mode )
{
	status_t error = (path ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		if (mkdir(path, mode) == -1)
			error = errno;
	}
	return error;
}

/*!	The parent directory must already exist.
	\param path the directory's path name
	\param result set to a file descriptor for the new directory
	\param mode the file permissions
	\return B_OK, if everything went fine, an error code otherwise
*/
status_t
StorageKit::create_dir( const char *path, FileDescriptor &result,
						mode_t mode )
{
	status_t error = create_dir(path, mode);
	if (error == B_OK)
		error = open_dir(path, result);
	return error;
}

/*!	\param dir the directory
	\param buffer the dirent structure to be filled
	\param length the size of the dirent structure
	\param count the maximal number of entries to be read
	\return
	- the number of entries stored in the supplied buffer,
	- \c 0, if at the end of the entry list,
	- \c B_BAD_VALUE, if \a buffer is NULL, or the supplied buffer is too small
*/
int32
StorageKit::read_dir( FileDescriptor dir, DirEntry *buffer, size_t length,
					  int32 count ) {
	// init a DIR structure
	LongDIR dirDir;
	dirDir.fd = dir;
	// check parameters
	int32 result = (buffer == NULL ? B_BAD_VALUE : 0);
	if (result == 0 && count > 0) {
		// read one entry and copy it into the buffer
		if (dirent *entry = readdir(&dirDir)) {
			// Don't trust entry->d_reclen.
			// Unlike stated in BeBook::BEntryList, the value is not the length
			// of the whole structure, but only of the name. Some FSs count
			// the terminating '\0', others don't.
			// So we calculate the size ourselves (including the '\0'):
			size_t entryLen = entry->d_name + strlen(entry->d_name) + 1
							  - (char*)entry;
			if (length >= entryLen) {
				memcpy(buffer, entry, entryLen);
				result = 1;
			} else	// buffer too small
				result = B_BAD_VALUE;
		}
	}
	return result;
}

status_t
StorageKit::rewind_dir( FileDescriptor dir ) {
	if (dir < 0)
		return B_BAD_VALUE;
	else {
		// init a DIR structure
		LongDIR dirDir;
		dirDir.fd = dir;
		::rewinddir(&dirDir);
		return B_OK;
	}
}

status_t
StorageKit::find_dir( FileDescriptor dir, const char *name,
					  DirEntry *result, size_t length ) {
	if (dir < 0 || name == NULL || result == NULL)
		return B_BAD_VALUE;
	
	status_t status;
	
	status = StorageKit::rewind_dir(dir);
	if (status == B_OK) {
		while (	StorageKit::read_dir(dir, result, length, 1) == 1)
		{
			if (strcmp(result->d_name, name) == 0)
				return B_OK;
		}
		status = B_ENTRY_NOT_FOUND;
	}
	
	return status;
}

status_t
StorageKit::find_dir( FileDescriptor dir, const char *name, entry_ref *result )
{
	LongDirEntry entry;
	status_t status = StorageKit::find_dir(dir, name, &entry, sizeof(entry));
	if (status != B_OK)
		return status;
	result->device = entry.d_pdev;
	result->directory = entry.d_pino;
	return result->set_name(entry.d_name);
}

status_t
StorageKit::dup_dir( FileDescriptor dir, FileDescriptor &result )
{
	return StorageKit::dup(dir, result);
}

status_t
StorageKit::close_dir( FileDescriptor dir ) {
	// init a DIR structure
	LongDIR* dirDir = (LongDIR*)malloc(sizeof(LongDIR));
	dirDir->fd = dir;
	return (::closedir(dirDir) == -1) ? errno : B_OK;	
}

//------------------------------------------------------------------------------
// SymLink functions
//------------------------------------------------------------------------------

/*!	The parent directory must already exist.
	\param path the link's path name
	\param linkToPath the path name the link shall point to
	\return B_OK, if everything went fine, an error code otherwise
*/
status_t
StorageKit::create_link( const char *path, const char *linkToPath )
{
	status_t error = (path && linkToPath ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		if (symlink(linkToPath, path) == -1)
			error = errno;
	}
	return error;
}

/*!	The parent directory must already exist.
	\param path the link's path name
	\param linkToPath the path name the link shall point to
	\param result set to a file descriptor for the new symbolic link
	\return B_OK, if everything went fine, an error code otherwise
*/
status_t
StorageKit::create_link( const char *path, const char *linkToPath,
						 FileDescriptor &result)
{
	status_t error = create_link(path, linkToPath);
	if (error == B_OK)
		error = open(path, O_RDWR, result);
	return error;
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

//------------------------------------------------------------------------------
// Miscellaneous Functions
//------------------------------------------------------------------------------

status_t
StorageKit::entry_ref_to_path( const struct entry_ref *ref, char *result, int size ) {
	if (ref == NULL) {
		return B_BAD_VALUE;
	} else {
		return entry_ref_to_path(ref->device, ref->directory, ref->name, result, size);
	}
}

const char *kEntryRefToPathToolPaths[] = {
	"tools/EntryRefToPath",
	"../tools/EntryRefToPath",
	"../../tools/EntryRefToPath",
	"storage_kit/tools/EntryRefToPath",
	"../../../tools/EntryRefToPath",
	"open-beos/storage_kit/tools/EntryRefToPath",
	"OpenBeOS/storage_kit/tools/EntryRefToPath",
	"/boot/home/config/lib/EntryRefToPath",
	"/boot/home/config/bin/EntryRefToPath",
};
const int kEntryRefToPathToolPathCount
	= sizeof(kEntryRefToPathToolPaths) / sizeof(char*);
static char *entryRefToPathToolPath = NULL;

struct InitEntryRefToPathToolPath {
	InitEntryRefToPathToolPath()
	{
		// Try to find our EntryRefToPath program
		const char *path = NULL;
		for (int i = 0; !path && i < kEntryRefToPathToolPathCount; i++) {
			int fd = ::open(kEntryRefToPathToolPaths[i], O_RDONLY);
			if (fd != -1) {
				::close(fd);
				path = (char *)kEntryRefToPathToolPaths[i];
			}
		}
		if (path) {
			if (StorageKit::is_absolute_path(path)) {
				entryRefToPathToolPath = new char[strlen(path) + 1];
				strcpy(entryRefToPathToolPath, path);
			} else {
				char buffer[B_PATH_NAME_LENGTH];
				if (char *cwd = getcwd(buffer, B_PATH_NAME_LENGTH)) {
					entryRefToPathToolPath
						= new char[strlen(cwd) + 1 + strlen(path) + 1];
					strcpy(entryRefToPathToolPath, cwd);
					strcat(entryRefToPathToolPath, "/");
					strcat(entryRefToPathToolPath, path);
				}
			}
		}
printf("InitEntryRefToPathToolPath() done: `%s'\n", entryRefToPathToolPath);
	}

	~InitEntryRefToPathToolPath()
	{
		delete[] entryRefToPathToolPath;
	}

} _InitEntryRefToPathToolPath;


status_t
StorageKit::entry_ref_to_path( dev_t device, ino_t directory, const char *name, char *result, int size ) {
//	printf("device = %ld \n", device);
//	printf("dir    = %lld \n", directory);
//	printf("name   = '%s' \n", name);


	if (result == NULL || size < 1)
		return B_BAD_VALUE;

	// A list of possible relative pathnames to our EntryRefToPath program
	const char *app_path = entryRefToPathToolPath;
	
	if (app_path == NULL) {
		result[0] = 0;
		return B_ERROR;
	}
	
	
	// Attempt to invoke our EntryRefToPath program using the
	// various pathnames from above. If one succeeds, it will
	// return B_OK and the others will not be tried.
	char cmd[B_PATH_NAME_LENGTH + 30 + B_FILE_NAME_LENGTH];

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
//				printf("+result = '%s'\n", result);
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
StorageKit::dir_to_self_entry_ref( FileDescriptor dir, entry_ref *result ) {
	if (dir == StorageKit::NullFd || result == NULL)
		return B_BAD_VALUE;
	return find_dir(dir, ".", result);
}

status_t
StorageKit::dir_to_path( FileDescriptor dir, char *result, int size ) {
	if (dir < 0 || result == NULL)
		return B_BAD_VALUE;

	entry_ref entry;
	status_t status;
	
	status = dir_to_self_entry_ref(dir, &entry);
	if (status != B_OK)
		return status;
		
	return entry_ref_to_path(&entry, result, size);
}

/*!	\param path the path name.
	\param result a pointer to a buffer the resulting path name shall be
		   written into.
	\param size the size of the buffer
	\return \c B_OK if everything went fine, an error code otherwise
*/
status_t
StorageKit::get_canonical_path(const char *path, char *result, size_t size)
{
	status_t error = (path && result ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		char *dirPath;
		char *leafName;
		if (split_path(path, dirPath, leafName)) {
			// handle special leaf names ("." and "..")
			if (strcmp(leafName, ".") == 0 || strcmp(leafName, "..") == 0)
				error = get_canonical_dir_path(path, result, size);
			else {
				// get the canonical dir path and append the leaf name
				error = get_canonical_dir_path(dirPath, result, size);
				if (error == B_OK) {
					size_t dirPathLen = strlen(result);
					if (dirPathLen + 1 + strlen(leafName) + 1 <= size) {
						strcat(result + dirPathLen, "/");
						strcat(result + dirPathLen + 1, leafName);
					} else
						error = B_BAD_VALUE;
				}
			}
			delete[] dirPath;
			delete[] leafName;
		} else
			error = B_BAD_VALUE;
	}

	return error;
}

/*!	The caller is responsible for deleting the returned path name.
	\param path the path name.
	\param result in this variable the resulting path name is returned
	\return \c B_OK if everything went fine, an error code otherwise
*/
status_t
StorageKit::get_canonical_path(const char *path, char *&result)
{
	status_t error = (path ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		result = new char[B_PATH_NAME_LENGTH];
		error = get_canonical_path(path, result, B_PATH_NAME_LENGTH);
		if (error != B_OK) {
			delete[] result;
			result = NULL;
		}
	}
	return error;
}

/*!	\param path the path name.
	\param result a pointer to a buffer the resulting path name shall be
		   written into.
	\param size the size of the buffer
	\return \c B_OK if everything went fine, an error code otherwise
*/
status_t
StorageKit::get_canonical_dir_path(const char *path, char *result, size_t size)
{
	status_t error = (path && result ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		FileDescriptor dir;
		error = open_dir(path, dir);
		if (error == B_OK) {
			error = dir_to_path(dir, result, size);
			close_dir(dir);
		}
	}
	return error;
}

/*!	The caller is responsible for deleting the returned path name.
	\param path the path name.
	\param result in this variable the resulting path name is returned
	\return \c B_OK if everything went fine, an error code otherwise
*/
status_t
StorageKit::get_canonical_dir_path(const char *path, char *&result)
{
	status_t error = (path ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		result = new char[B_PATH_NAME_LENGTH];
		error = get_canonical_dir_path(path, result, B_PATH_NAME_LENGTH);
		if (error != B_OK) {
			delete[] result;
			result = NULL;
		}
	}
	return error;
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

