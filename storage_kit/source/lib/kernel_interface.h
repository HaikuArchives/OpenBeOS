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

// For typedefs
#include <sys/dirent.h>		// For dirent
#include <sys/stat.h>		// For struct stat
#include <fcntl.h>			// For flock

#include "Error.h"

typedef struct attr_info;

// Forward Declarations
struct entry_ref;

//! Private Storage Kit Namespace
/*! Private Storage Kit Namespace */
namespace StorageKit {

// Modes for opening files
enum Mode {
	READ,
	WRITE,
	READ_WRITE
};

// Specialized Exceptions
class EEntryNotFound : public Error {
public:
	EEntryNotFound() : Error(ENOENT, "Entry not found") {};
};

// File descriptor type -- POSIX versions
typedef int FileDescriptor;
typedef DIR* Dir;
typedef dirent DirEntry;
typedef flock FileLock;
typedef struct stat Stat;
typedef uint32 StatMember;
typedef attr_info AttrInfo;

// Constants -- POSIX versions
const Dir NullDir = NULL;

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

//------------------------------------------------------------------------------
// File Functions
//------------------------------------------------------------------------------
/*! Opens the filesystem entry specified by path. Returns a
	new file descriptor if successful, -1 otherwise. */
status_t open( const char *path, Mode mode, FileDescriptor &fd );

/*! Closes a previously open()ed file. */
status_t close( FileDescriptor file );

/*! Returns a new file descriptor that refers to the same file as
	that specified, or -1 if unsuccessful. Remember to call close
	on the file descriptor when through with it. */
FileDescriptor dup( FileDescriptor file );

/*! Flushes any buffers associated with the given file to disk
	and then returns. */
status_t sync( FileDescriptor file );

/*! Returns statistical information for the given file. */
status_t get_stat( FileDescriptor file, Stat *s );

/*! Modifies a given portion of the file's statistical information. */
status_t set_stat( FileDescriptor file, Stat &s, StatMember what );

/*! Locks the given file so it may not be accessed by anyone else. */
status_t lock( FileDescriptor file, Mode mode, FileLock *lock );

/*! Unlocks a file previously locked with lock(). */
status_t unlock( FileDescriptor file, FileLock *lock );


//------------------------------------------------------------------------------
// Attribute Functions
//------------------------------------------------------------------------------
/*! Reads the data from the specified attribute into the given buffer of size
	count. Returns the number of bytes actually read. */
ssize_t read_attr( FileDescriptor file, const char *attribute, uint32 type, 
						off_t pos, void *buf, size_t count );
						
/*! Write count bytes from the given data buffer into the specified attribute. */
ssize_t write_attr ( FileDescriptor file, const char *attribute, uint32 type, 
						off_t pos, const void *buf, size_t count );

/*! Removes the specified attribute and any data associated with it. */						
status_t remove_attr( FileDescriptor file, const char *attr );

/*! Returns statistical information about the given attribute. */
status_t stat_attr( FileDescriptor file, const char *name, AttrInfo *ai );



//------------------------------------------------------------------------------
// Attribute Directory Functions
//------------------------------------------------------------------------------
/*! Opens the attribute directory of a given file. */
Dir open_attr_dir( FileDescriptor file );

/*! Rewinds the given attribute directory. */
void rewind_attr_dir( Dir dir );

/*! Returns the next item in the given attribute directory, or
	B_ENTRY_NOT_FOUND if at the end of the list. */
DirEntry* read_attr_dir( Dir dir );

/*! Closes an attribute directory previously opened with open_attr_dir(). */
status_t close_attr_dir( Dir dir );


//------------------------------------------------------------------------------
// Directory Functions
//------------------------------------------------------------------------------
/*! Opens the given directory. Sets result to a properly "unitialized" directory
	if the function fails. */
status_t open_dir( const char *path, Dir &result );

/*! Returns the next entry in the given directory, or B_ENTRY_NOT_FOUND
	if at the end of the list. */
DirEntry* read_dir( Dir dir );

/*! Closes the given directory. */
status_t close_dir( Dir dir );

//------------------------------------------------------------------------------
// SymLink functions
//------------------------------------------------------------------------------
/*! If path refers to a symlink, the pathname of the target to which path
	is linked is copied into result and NULL terminated, the path being truncated
	at size-1 chars if necessary (a buffer of size B_PATH_NAME_LENGTH+1 is a good
	idea), and the number of chars in the target pathname is returned. If size is
	less than 1 or result is NULL, B_BAD_VALUE will be returned and result will
	remain unmodified. For any other error, result is set to an empty string and
	an error code is returned. */
ssize_t read_link( const char *path, char *result, int size );


//------------------------------------------------------------------------------
// Miscellaneous Functions
//------------------------------------------------------------------------------
/*! Converts the given entry_ref into an absolute pathname, returning
	the result in the string of length size pointed to by result (a size
	of B_PATH_NAME_LENGTH+1 is a good idea). Returns B_OK if successful.
	If ref or result is NULL or size is -1, B_BAD_VALUE is returned and result
	is unmodified. Otherwise, an error code is returned and result is
	set to be an empty string. */
status_t entry_ref_to_path( struct entry_ref *ref, char *result, int size );

/*! See the other definition of entry_ref_to_path() */
status_t entry_ref_to_path( dev_t device, ino_t directory, const char *name, char *result, int size );

} // namespace StorageKit

#endif
