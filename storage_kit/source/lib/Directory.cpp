//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Directory.cpp
//---------------------------------------------------------------------
#include <Directory.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

enum {
	NOT_IMPLEMENTED	= B_ERROR,
};

// constructor
//! Creates an uninitialized Directory object.
/*! \todo Implement! */
BDirectory::BDirectory()
{
}

// copy constructor
//! Creates a copy of the supplied BDirectory.
/*!	\param dir the BDirectory object to be copied
	\todo Implement!
*/
BDirectory::BDirectory(const BDirectory &dir)
{
}

// constructor
/*! \brief Creates a BDirectory and initializes it to the directory referred
	to by the supplied entry_ref.
	\param ref the entry_ref referring to the directory
	\todo Implement!
*/
BDirectory::BDirectory(const entry_ref *ref)
{
}

// constructor
/*! \brief Creates a BDirectory and initializes it to the directory referred
	to by the supplied node_ref.
	\param nref the node_ref referring to the directory
	\todo Implement!
*/
BDirectory::BDirectory(const node_ref *nref)
{
}

// constructor
/*! \brief Creates a BDirectory and initializes it to the directory referred
	to by the supplied BEntry.
	\param entry the BEntry referring to the directory
	\todo Implement!
*/
BDirectory::BDirectory(const BEntry *entry)
{
}

// constructor
/*! \brief Creates a BDirectory and initializes it to the directory referred
	to by the supplied path name.
	\param path the directory's path name 
	\todo Implement!
*/
BDirectory::BDirectory(const char *path)
{
}

// constructor
/*! \brief Creates a BDirectory and initializes it to the directory referred
	to by the supplied path name relative to the specified BDirectory.
	\param dir the BDirectory, relative to which the directory's path name is
		   given
	\param path the directory's path name relative to \a dir
	\todo Implement!
*/
BDirectory::BDirectory(const BDirectory *dir, const char *path)
{
}

// destructor
//! Frees all allocated resources.
/*! \todo Implement! */
BDirectory::~BDirectory()
{
}

// SetTo
/*! \brief Re-initializes the BDirectory to the directory referred to by the
	supplied entry_ref.
	\param ref the entry_ref referring to the directory
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: NULL \a ref.
	- \c B_ENTRY_NOT_FOUND: Directory not found.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	\todo Implement!
*/
status_t
BDirectory::SetTo(const entry_ref *ref)
{
	return NOT_IMPLEMENTED;
}

// SetTo
/*! \brief Re-initializes the BDirectory to the directory referred to by the
	supplied node_ref.
	\param nref the node_ref referring to the directory
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: NULL \a nref.
	- \c B_ENTRY_NOT_FOUND: Directory not found.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	\todo Implement!
*/
status_t
BDirectory::SetTo(const node_ref *nref)
{
	return NOT_IMPLEMENTED;
}

// SetTo
/*! \brief Re-initializes the BDirectory to the directory referred to by the
	supplied BEntry.
	\param entry the BEntry referring to the directory
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: NULL \a entry.
	- \c B_ENTRY_NOT_FOUND: Directory not found.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	\todo Implement!
*/
status_t
BDirectory::SetTo(const BEntry *entry)
{
	return NOT_IMPLEMENTED;
}

// SetTo
/*! \brief Re-initializes the BDirectory to the directory referred to by the
	supplied path name.
	\param path the directory's path name 
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: NULL \a path.
	- \c B_ENTRY_NOT_FOUND: Directory not found.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_NAME_TOO_LONG: The supplied path name (\a path) is too long.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	- \c B_NOT_A_DIRECTORY: \a path includes a non-directory.
	\todo Implement!
*/
status_t
BDirectory::SetTo(const char *path)
{
	return NOT_IMPLEMENTED;
}

// SetTo
/*! \brief Re-initializes the BDirectory to the directory referred to by the
	supplied path name relative to the specified BDirectory.
	\param dir the BDirectory, relative to which the directory's path name is
		   given
	\param path the directory's path name relative to \a dir
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: NULL \a dir or \a path, or \a path is absolute.
	- \c B_ENTRY_NOT_FOUND: Directory not found.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_NAME_TOO_LONG: The supplied path name (\a path) is too long.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	- \c B_NOT_A_DIRECTORY: \a path includes a non-directory.
	\todo Implement!
*/
status_t
BDirectory::SetTo(const BDirectory *dir, const char *path)
{
	return NOT_IMPLEMENTED;
}

// GetEntry
//! Returns a BEntry referring to the directory represented by this object.
/*!	If the initialization of \a entry fails, it is Unset().
	\param entry a pointer to the entry that shall be set to refer to the
		   directory
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: NULL \a entry.
	- \c B_ENTRY_NOT_FOUND: Directory not found.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	\todo Implement!
*/
status_t
BDirectory::GetEntry(BEntry *entry) const
{
	return NOT_IMPLEMENTED;
}

// IsRootDirectory
/*!	\brief Returns whether the directory represented be this BDirectory is a
	root directory of a volume.
	\return
	- \c true, if the BDirectory is properly initialized and represents a
	  root directory of some volume,
	- \c false, otherwise.
	\todo Implement!
*/
bool
BDirectory::IsRootDirectory() const
{
	return false;	// not implemented
}

// FindEntry
/*! \brief Finds an entry referred to by a path relative to the directory
	represented by this BDirectory.
	If the entry couldn't be found, \a entry is Unset().
	\param path a path name relative to the directory
	\param entry a pointer to a BEntry to be initialized with the found entry
	\param traverse specifies whether to follow it, if the found entry
		   is a symbolic link.
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: NULL \a path or \a entry.
	- \c B_ENTRY_NOT_FOUND: Entry not found.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_NAME_TOO_LONG: The supplied path name (\a path) is too long.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	- \c B_NOT_A_DIRECTORY: \a path includes a non-directory.
	\note The functionality of this method differs from the one of
		  BEntry::SetTo(BDirectory *, const char *, bool) in that the
		  latter doesn't require the entry to be existent, whereas this
		  function does.
	\todo Implement!
*/
status_t
BDirectory::FindEntry(const char *path, BEntry *entry, bool traverse) const
{
	return NOT_IMPLEMENTED;
}

// Contains
/*!	\brief Returns whether this directory or any of its subdirectories
	at any level contains the entry referred to by the supplied path name.
	Only entries that match the node flavor specified by \a nodeFlags are
	considered.
	\param path the entry's path name. May be relative to this directory or
		   absolute.
	\param nodeFlags Any of the following:
		   - \c B_FILE_NODE: The entry must be a file.
		   - \c B_DIRECTORY_NODE: The entry must be a directory.
		   - \c B_SYMLINK_NODE: The entry must be a symbolic link.
		   - \c B_ANY_NODE: The entry may be of any kind.
	\return
	- \c true, if the BDirectory is properly initialized and the entry of the
	  matching kind could be found,
	- \c false, otherwise
	\todo Implement!
*/
bool
BDirectory::Contains(const char *path, int32 nodeFlags) const
{
	return false;	// not implemented
}

// Contains
/*!	\brief Returns whether this directory or any of its subdirectories
	at any level contains the entry referred to by the supplied BEntry.
	Only entries that match the node flavor specified by \a nodeFlags are
	considered.
	\param entry a BEntry referring to the entry
	\param nodeFlags Any of the following:
		   - \c B_FILE_NODE: The entry must be a file.
		   - \c B_DIRECTORY_NODE: The entry must be a directory.
		   - \c B_SYMLINK_NODE: The entry must be a symbolic link.
		   - \c B_ANY_NODE: The entry may be of any kind.
	\return
	- \c true, if the BDirectory is properly initialized and the entry of the
	  matching kind could be found,
	- \c false, otherwise
	\todo Implement!
*/
bool
BDirectory::Contains(const BEntry *entry, int32 nodeFlags) const
{
	return false;	// not implemented
}

// GetStatFor
/*!	\brief Returns the stat structure of the entry referred to by the supplied
	path name.
	\param path the entry's path name. May be relative to this directory or
		   absolute.
	\param st a pointer to the stat structure to be filled in by this function
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: NULL \a path or \a st.
	- \c B_ENTRY_NOT_FOUND: Entry not found.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_NAME_TOO_LONG: The supplied path name (\a path) is too long.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	- \c B_NOT_A_DIRECTORY: \a path includes a non-directory.
	\todo Implement!
*/
status_t
BDirectory::GetStatFor(const char *path, struct stat *st) const
{
	return NOT_IMPLEMENTED;
}

// GetNextEntry
//! Returns the BDirectory's next entry as a BEntry.
/*!	Unlike GetNextDirents() this method ignores the entries "." and "..".
	\param entry a pointer to a BEntry to be initialized with the found entry
	\param traverse specifies whether to follow it, if the found entry
		   is a symbolic link.
	\note The iterator used be this method is the same one used by
		  GetNextRef(), GetNextDirents(), Rewind() and CountEntries().
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: NULL \a entry.
	- \c B_ENTRY_NOT_FOUND: No more entries found.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	\todo Implement!
*/
status_t
BDirectory::GetNextEntry(BEntry *entry, bool traverse)
{
	return NOT_IMPLEMENTED;
}

// GetNextRef
//! Returns the BDirectory's next entry as an entry_ref.
/*!	Unlike GetNextDirents() this method ignores the entries "." and "..".
	\param ref a pointer to an entry_ref to be filled in with the data of the
		   found entry
	\param traverse specifies whether to follow it, if the found entry
		   is a symbolic link.
	\note The iterator used be this method is the same one used by
		  GetNextEntry(), GetNextDirents(), Rewind() and CountEntries().
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: NULL \a ref.
	- \c B_ENTRY_NOT_FOUND: No more entries found.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	\todo Implement!
*/
status_t
BDirectory::GetNextRef(entry_ref *ref)
{
	return NOT_IMPLEMENTED;
}

// GetNextDirents
//! Returns the BDirectory's next entries as dirent structures.
/*!	Unlike GetNextEntry() and GetNextRef(), this method returns also
	the entries "." and "..".
	\param buf a pointer to a buffer to be filled with dirent structures of
		   the found entries
	\param count the maximal number of entries to be returned.
	\note The iterator used be this method is the same one used by
		  GetNextEntry(), GetNextRef(), Rewind() and CountEntries().
	\return
	- The number of dirent structures stored in the buffer, 0 when there are
	  no more entries to be returned.
	- \c B_BAD_VALUE: NULL \a buf.
	- \c B_ENTRY_NOT_FOUND: Entry not found.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_NAME_TOO_LONG: The entry's name is too long for the buffer.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	\todo Implement!
*/
int32
BDirectory::GetNextDirents(dirent *buf, size_t bufSize, int32 count)
{
	return NOT_IMPLEMENTED;
}

// Rewind
//!	Rewinds the directory iterator.
/*!	\return
	- \c B_OK: Everything went fine.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	\see GetNextEntry(), GetNextRef(), GetNextDirents(), CountEntries()
	\todo Implement!
*/
status_t
BDirectory::Rewind()
{
	return NOT_IMPLEMENTED;
}

// CountEntries
//!	Returns the number of entries in this directory.
/*!	CountEntries() uses the directory iterator also used by GetNextEntry(),
	GetNextRef() and GetNextDirents(). It does a Rewind(), iterates through
	the entries and Rewind()s again. The entries "." and ".." are not counted.
	\return
	- \c B_OK: Everything went fine.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	\see GetNextEntry(), GetNextRef(), GetNextDirents(), Rewind()
	\todo Implement!
*/
int32
BDirectory::CountEntries()
{
	return NOT_IMPLEMENTED;
}

// CreateDirectory
//! Creates a new directory.
/*! If an entry with the supplied name does already exist, the method fails.
	\param path the new directory's path name. May be relative to this
		   directory or absolute.
	\param dir a pointer to a BDirectory to be initialized to the newly
		   created directory. May be NULL.
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: NULL \a path.
	- \c B_ENTRY_NOT_FOUND: \a path does not refer to a possible entry.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_FILE_EXISTS: An entry with that name does already exist.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	\todo Implement!
*/
status_t
BDirectory::CreateDirectory(const char *path, BDirectory *dir)
{
	return NOT_IMPLEMENTED;
}

// CreateFile
//! Creates a new file.
/*!	If a file with the supplied name does already exist, the method fails,
	unless it is passed \c false to \a failIfExists -- in that case the file
	is truncated to zero size. The new BFile will operate in \c B_READ_WRITE
	mode.
	\param path the new file's path name. May be relative to this
		   directory or absolute.
	\param file a pointer to a BFile to be initialized to the newly
		   created file. May be NULL.
	\param failIfExists \c true, if the method shall fail, if the file does
		   already exist, \c false otherwise.
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: NULL \a path.
	- \c B_ENTRY_NOT_FOUND: \a path does not refer to a possible entry.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_FILE_EXISTS: A file with that name does already exist and
	  \c true has been passed for \a failIfExists.
	- \c B_IS_A_DIRECTORY: A directory with the supplied name does already
	  exist.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	\todo Implement!
*/
status_t
BDirectory::CreateFile(const char *path, BFile *file, bool failIfExists)
{
	return NOT_IMPLEMENTED;
}

// CreateSymLink
//! Creates a new symbolic link.
/*! If an entry with the supplied name does already exist, the method fails.
	\param path the new symbolic link's path name. May be relative to this
		   directory or absolute.
	\param linkToPath the path the symbolic link shall point to.
	\param dir a pointer to a BSymLink to be initialized to the newly
		   created symbolic link. May be NULL.
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: NULL \a path or \a linkToPath.
	- \c B_ENTRY_NOT_FOUND: \a path does not refer to a possible entry.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_FILE_EXISTS: An entry with that name does already exist.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	\todo Implement!
*/
status_t
BDirectory::CreateSymLink(const char *path, const char *linkToPath,
						  BSymLink *link)
{
	return NOT_IMPLEMENTED;
}

// =
//! Assigns another BDirectory to this BDirectory.
/*!	If the other BDirectory is uninitialized, this one will be too. Otherwise
	it will refer to the same directory, unless an error occurs.
	\param dir the original BDirectory
	\return a reference to this BDirectory
	\todo Implement!
*/
BDirectory &
BDirectory::operator=(const BDirectory &dir)
{
	return *this;
}


void BDirectory::_ReservedDirectory1() {}
void BDirectory::_ReservedDirectory2() {}
void BDirectory::_ReservedDirectory3() {}
void BDirectory::_ReservedDirectory4() {}
void BDirectory::_ReservedDirectory5() {}
void BDirectory::_ReservedDirectory6() {}

// close_fd
//! Closes the BDirectory's file descriptor.
/*! \todo Implement! */
void
BDirectory::close_fd()
{
}

// set_fd
//! Sets the file descriptor for the BDirectory.
/*!	\param fd the new file descriptor, may be -1.
	\todo Implement!
*/
status_t
BDirectory::set_fd(int fd)
{
	return BNode::set_fd(fd);
}

//! Returns the BDirectory's file descriptor.
/*! To be used instead of accessing the BNode's private \c fFd member directly.
	\return the file descriptor, or -1, if not properly initialized.
	\todo Implement!
*/
int
BDirectory::get_fd() const
{
	return -1;	// not implemented
}

//! Sets the BNode's status.
/*! To be used instead of accessing the BNode's private \c fCStatus member
	directly.
	\param newStatus the new status to be set.
	\todo Implement!
*/
void
BDirectory::set_status(status_t newStatus)
{
	// not implemented
}


// C functions

// create_directory
//! Creates all missing directories along a given path.
/*!	\param path the directory path name.
	\param mode a permission specification, which shall be used for the
		   newly created directories.
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: NULL \a path.
	- \c B_ENTRY_NOT_FOUND: \a path does not refer to a possible entry.
	- \c B_PERMISSION_DENIED: Directory permissions didn't allow operation.
	- \c B_NO_MEMORY: Insufficient memory for operation.
	- \c B_LINK_LIMIT: Indicates a cyclic loop within the file system.
	- \c B_BUSY: A node was busy.
	- \c B_FILE_ERROR: A general file error.
	- \c B_FILE_EXISTS: An entry other than a directory with that name does
	  already exist.
	- \c B_NO_MORE_FDS: The application has run out of file descriptors.
	\todo Implement!
*/
status_t
create_directory(const char *path, mode_t mode)
{
	return NOT_IMPLEMENTED;
}


#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif
