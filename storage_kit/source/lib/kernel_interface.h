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

// Forward Declarations
typedef struct attr_info;
typedef struct entry_ref;

//! Private Storage Kit Namespace
/*! Private Storage Kit Namespace */
namespace StorageKit {

// Type aliases
typedef int FileDescriptor;
typedef dirent DirEntry;
typedef flock FileLock;
typedef struct stat Stat;
typedef uint32 StatMember;
typedef attr_info AttrInfo;
typedef int OpenFlags;			// open() flags
typedef mode_t CreationFlags;	// open() mode
typedef int SeekMode;			// lseek() mode

// For convenience:
struct LongDirEntry : DirEntry { char _buffer[B_FILE_NAME_LENGTH]; };

// Constants -- POSIX versions
const FileDescriptor NullFd = -1;


//------------------------------------------------------------------------------
// File Functions
//------------------------------------------------------------------------------
/*! Opens the filesystem entry specified by path. Returns a
	new file descriptor if successful, -1 otherwise. This version
	fails if the given file does not exist, or if you specify
	O_CREAT as one of the flags (use the four argument version
	of StorageKit::open() if you wish to create the file when
	it doesn't already exist). */
status_t open( const char *path, OpenFlags flags, FileDescriptor &result );

/*! Same as the other version of open() except the file is created with the
	permissions given by creationFlags if it doesn't exist. */
status_t open( const char *path, OpenFlags flags, CreationFlags creationFlags,
	FileDescriptor &result );

/*! Closes a previously open()ed file. */
status_t close( FileDescriptor file );

//! Reads data from a file into a buffer.
ssize_t read(FileDescriptor fd, void *buf, ssize_t len);

//! Reads data from a certain position in a file into a buffer.
ssize_t read(FileDescriptor fd, void *buf, off_t pos, ssize_t len);

//! Writes data from a buffer into a file.
ssize_t write(FileDescriptor fd, const void *buf, ssize_t len);

//! Writes data from a buffer to a certain position in a file.
ssize_t write(FileDescriptor fd, const void *buf, off_t pos, ssize_t len);

//! Moves a file's read/write pointer.
off_t seek(FileDescriptor fd, off_t pos, SeekMode mode);

//! Returns the position of a file's read/write pointer.
off_t get_position(FileDescriptor fd);

/*! Returns a new file descriptor that refers to the same file as
	that specified, or -1 if unsuccessful. Remember to call close
	on the file descriptor when through with it. */
FileDescriptor dup( FileDescriptor file );

/*! \brief Similar to the first version, aside from that an error code is
	returned and the resulting file descriptor is passed back via a reference
	parameter an. */
status_t dup( FileDescriptor file, FileDescriptor& result );

/*! Flushes any buffers associated with the given file to disk
	and then returns. */
status_t sync( FileDescriptor file );

/*! Locks the given file so it may not be accessed by anyone else. */
status_t lock( FileDescriptor file, OpenFlags mode, FileLock *lock );

/*! Unlocks a file previously locked with lock(). */
status_t unlock( FileDescriptor file, FileLock *lock );

/*! Returns statistical information for the given file. */
status_t get_stat(const char *path, Stat *s);
status_t get_stat(FileDescriptor file, Stat *s);
status_t get_stat(entry_ref &ref, Stat *s);

/*! Modifies a given portion of the file's statistical information. */
status_t set_stat( FileDescriptor file, Stat &s, StatMember what );


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
FileDescriptor open_attr_dir( FileDescriptor file );

/*! Rewinds the given attribute directory. */
void rewind_attr_dir( FileDescriptor dirFd );

/*! Returns the next item in the given attribute directory, or
	B_ENTRY_NOT_FOUND if at the end of the list. */
DirEntry* read_attr_dir( FileDescriptor dirFd );

/*! Closes an attribute directory previously opened with open_attr_dir(). */
status_t close_attr_dir( FileDescriptor dirFd );


//------------------------------------------------------------------------------
// Directory Functions
//------------------------------------------------------------------------------
/*! Opens the given directory. Sets result to a properly "unitialized" directory
	if the function fails. */
status_t open_dir( const char *path, FileDescriptor &result );

//! Creates a new directory.
status_t create_dir( const char *path,
					 mode_t mode = S_IRWXU | S_IRWXG | S_IRWXU );

//! Creates a new directory and opens it.
status_t create_dir( const char *path, FileDescriptor &result,
					 mode_t mode = S_IRWXU | S_IRWXG | S_IRWXU);

//! \brief Returns the next entries in the given directory.
int32 read_dir( FileDescriptor dir, DirEntry *buffer, size_t length,
				int32 count = INT_MAX);

/*! Rewindes the directory to the first entry in the list. */
status_t rewind_dir( FileDescriptor dir );

/*! Iterates through the given directory searching for an entry whose name
	matches that given by name. On success, places the DirEntry in result
	and returns B_OK. On failures, returns an error code and sets result to
	StorageKit::NullDir.
	
	<b>Note:</b> This call modifies the internal position marker of dir. */
status_t find_dir( FileDescriptor dir, const char *name, DirEntry *result,
				   size_t length );

/*! Calls the other version of StorageKit::find_dir() and stores the results
	in the given entry_ref. */
status_t find_dir( FileDescriptor dir, const char *name, entry_ref *result );

/*! Creates a duplicated of the given directory and places it in result if successful,
	returning B_OK. Returns an error code and sets result to -1 if
	unsuccessful. */
status_t dup_dir( FileDescriptor dir, FileDescriptor &result );

/*! Closes the given directory. */
status_t close_dir( FileDescriptor dir );

//------------------------------------------------------------------------------
// SymLink functions
//------------------------------------------------------------------------------
//! Creates a new symbolic link.
status_t create_link( const char *path, const char *linkToPath );

//! Creates a new symbolic link and opens it.
status_t create_link( const char *path, const char *linkToPath,
					  FileDescriptor &result );

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
	of B_PATH_NAME_LENGTH is a good idea).
	
	Returns B_OK if successful.
	
	If ref or result is NULL or size is -1, B_BAD_VALUE is returned. Otherwise,
	an error code is returned. The state of result after an error is undefined.
*/
status_t entry_ref_to_path( const struct entry_ref *ref, char *result, int size );

/*! See the other definition of entry_ref_to_path() */
status_t entry_ref_to_path( dev_t device, ino_t directory, const char *name, char *result, int size );

/*! Converts the given directory into an entry_ref. Note that the entry_ref is
	actually a reference to the file "." in the given directory.

	Returns B_OK if successful.
	
	If dir is < 0 or result is NULL, B_BAD_VALUE is returned.
	Otherwise, an appropriate error code is returned.
 */
status_t dir_to_self_entry_ref( FileDescriptor dir, entry_ref *result );


/*! Converts the given directory into an absolute pathname, returning the
	result in the string of length size pointed to by result (a size of
	B_PATH_NAME_LENGTH is a good idea).
	
	Returns B_OK if successful.
	
	If dir is < 0 or result is NULL or size is -1, B_BAD_VALUE
	is returned. Otherwise, an error code is returned. The state of result after
	an error is undefined.
*/
status_t dir_to_path( FileDescriptor dir, char *result, int size );

/*!	\brief Returns the canonical representation of a given path referring to an
	potentially abstract entry in an existing directory. */
status_t get_canonical_path(const char *path, char *result, size_t size);

/*!	\brief Returns the canonical representation of a given path referring to an
	potentially abstract entry in an existing directory. */
status_t get_canonical_path(const char *path, char *&result);

/*!	\brief Returns the canonical representation of a given path referring to an
	existing directory. */
status_t get_canonical_dir_path(const char *path, char *result, size_t size);

/*!	\brief Returns the canonical representation of a given path referring to an
	existing directory. */
status_t get_canonical_dir_path(const char *path, char *&result);

/*! Returns true if the given entry_ref represents the root directory, false otherwise. */
bool entry_ref_is_root_dir( entry_ref &ref );

/*! Renames oldPath to newPath, replacing newPath if it exists. */
status_t rename(const char *oldPath, const char *newPath);

/*! Removes path from the filesystem. */
status_t remove(const char *path);


} // namespace StorageKit

#endif
