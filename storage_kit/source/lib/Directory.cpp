//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Directory.cpp
//---------------------------------------------------------------------
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <Path.h>
#include <SymLink.h>
#include "storage_support.h"
#include "kernel_interface.h"

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

// constructor
//! Creates an uninitialized Directory object.
BDirectory::BDirectory()
		  : BNode(),
			BEntryList()
{
}

// copy constructor
//! Creates a copy of the supplied BDirectory.
/*!	\param dir the BDirectory object to be copied
*/
BDirectory::BDirectory(const BDirectory &dir)
		  : BNode(),
			BEntryList()
{
	*this = dir;
}

// constructor
/*! \brief Creates a BDirectory and initializes it to the directory referred
	to by the supplied entry_ref.
	\param ref the entry_ref referring to the directory
*/
BDirectory::BDirectory(const entry_ref *ref)
		  : BNode(),
			BEntryList()
{
	SetTo(ref);
}

// constructor
/*! \brief Creates a BDirectory and initializes it to the directory referred
	to by the supplied node_ref.
	\param nref the node_ref referring to the directory
*/
BDirectory::BDirectory(const node_ref *nref)
		  : BNode(),
			BEntryList()
{
	SetTo(nref);
}

// constructor
/*! \brief Creates a BDirectory and initializes it to the directory referred
	to by the supplied BEntry.
	\param entry the BEntry referring to the directory
*/
BDirectory::BDirectory(const BEntry *entry)
		  : BNode(),
			BEntryList()
{
	SetTo(entry);
}

// constructor
/*! \brief Creates a BDirectory and initializes it to the directory referred
	to by the supplied path name.
	\param path the directory's path name 
*/
BDirectory::BDirectory(const char *path)
		  : BNode(),
			BEntryList()
{
	SetTo(path);
}

// constructor
/*! \brief Creates a BDirectory and initializes it to the directory referred
	to by the supplied path name relative to the specified BDirectory.
	\param dir the BDirectory, relative to which the directory's path name is
		   given
	\param path the directory's path name relative to \a dir
*/
BDirectory::BDirectory(const BDirectory *dir, const char *path)
		  : BNode(),
			BEntryList()
{
	SetTo(dir, path);
}

// destructor
//! Frees all allocated resources.
/*! If the BDirectory is properly initialized, the directory's file descriptor
	is closed.
*/
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
	\todo Currently implemented using StorageKit::entry_ref_to_path().
		  Reimplement!
*/
status_t
BDirectory::SetTo(const entry_ref *ref)
{
//printf("BDirectory::SetTo(const entry_ref *ref)\n");
	char path[B_PATH_NAME_LENGTH];
	status_t error = (ref ? B_OK : B_BAD_VALUE);
//printf("  error: %x\n", error);
	if (error == B_OK)
		error = StorageKit::entry_ref_to_path(ref, path, B_PATH_NAME_LENGTH);
//printf("  error: %x\n", error);
	if (error == B_OK)
		error = SetTo(path);
//printf("  error: %x\n", error);
	set_status(error);
//printf("BDirectory::SetTo(const entry_ref *ref) done\n");
	return error;
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
*/
status_t
BDirectory::SetTo(const node_ref *nref)
{
	status_t error = (nref ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		entry_ref ref(nref->device, nref->node, ".");
		error = SetTo(&ref);
	}
	return error;
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
	\todo Currently implemented using StorageKit::entry_ref_to_path().
		  Reimplement!
*/
status_t
BDirectory::SetTo(const BEntry *entry)
{
//printf("BDirectory::SetTo(const BEntry *entry)\n");
	entry_ref ref;
	status_t error = (entry ? B_OK : B_BAD_VALUE);
//printf("  error: %x\n", error);
	if (error == B_OK && entry->InitCheck() != B_OK)
		error = B_BAD_VALUE;
//printf("  error: %x\n", error);
	if (error == B_OK)
		error = entry->GetRef(&ref);
//printf("  error: %x\n", error);
	if (error == B_OK)
		error = SetTo(&ref);
//printf("  error: %x\n", error);
	set_status(error);
//printf("BDirectory::SetTo(const BEntry *entry) done\n");
	return error;
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
*/
status_t
BDirectory::SetTo(const char *path)
{
	status_t result = (path ? B_OK : B_BAD_VALUE);
	Unset();	
	StorageKit::FileDescriptor newFd = -1;
	if (result == B_OK)
		result = StorageKit::open_dir(path, newFd);
	// set the new file descriptor
	if (result == B_OK) {
		result = set_fd(newFd);
		if (result != B_OK)
			StorageKit::close_dir(newFd);
	}
	// finally set the BNode status
	set_status(result);
	return result;
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
	\todo Implemented using SetTo(BEntry*). Check, if necessary to reimplement!
*/
status_t
BDirectory::SetTo(const BDirectory *dir, const char *path)
{
	status_t error = (dir && path ? B_OK : B_BAD_VALUE);
	if (error == B_OK && StorageKit::is_absolute_path(path))
		error = B_BAD_VALUE;
	BEntry entry;
	if (error == B_OK)
		error = entry.SetTo(dir, path);
	if (error == B_OK)
		error = SetTo(&entry);
	set_status(error);
	return error;
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
	\todo Implemented using StorageKit::dir_to_self_entry_ref(). Check, if
		  there is a better alternative.
*/
status_t
BDirectory::GetEntry(BEntry *entry) const
{
	status_t error = (entry ? B_OK : B_BAD_VALUE);
	if (error == B_OK && InitCheck() != B_OK) {
		entry->Unset();
		error = B_NO_INIT;
	}
	entry_ref ref;
	if (error == B_OK)
		error = StorageKit::dir_to_self_entry_ref(get_fd(), &ref);
	if (error == B_OK)
		error = entry->SetTo(&ref);
	return error;
}

// IsRootDirectory
/*!	\brief Returns whether the directory represented be this BDirectory is a
	root directory of a volume.
	\return
	- \c true, if the BDirectory is properly initialized and represents a
	  root directory of some volume,
	- \c false, otherwise.
	\todo Implemented using StorageKit::dir_to_self_entry_ref(). Check, if
		  there is a better alternative.
*/
bool
BDirectory::IsRootDirectory() const
{
	// check first, if we are "/"
	BEntry entry;
	bool result = (GetEntry(&entry) == B_OK);
	bool isRootRoot = false;
	entry_ref ref;
	if (result)
		result = (entry.GetRef(&ref) == B_OK);
	if (result)
		isRootRoot = StorageKit::entry_ref_is_root_dir(ref);
	if (result && !isRootRoot) {
		// Get our own and out parent's stat and compare the device IDs.
		StorageKit::Stat ourStat, parentStat;
		result = (GetStat(&ourStat) == B_OK);
		if (result) {
			result = (GetStatFor("..", &parentStat) == B_OK);
			result &= (ourStat.st_dev != parentStat.st_dev);
		}
	}
	return result;
}

// FindEntry
/*! \brief Finds an entry referred to by a path relative to the directory
	represented by this BDirectory.
	\a path may be absolute. If the BDirectory is not properly initialized,
	the entry is search relative to the current directory.
	If the entry couldn't be found, \a entry is Unset().
	\param path the entry's path name. May be relative to this directory or
		   absolute.
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
*/
status_t
BDirectory::FindEntry(const char *path, BEntry *entry, bool traverse) const
{
	status_t error = (path && entry ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		// init a potentially abstract entry
		if (InitCheck() == B_OK)
			error = entry->SetTo(this, path, traverse);
		else
			error = entry->SetTo(path, traverse);
		// fail, if entry is abstract
		if (error == B_OK && !entry->Exists())
			error = B_ENTRY_NOT_FOUND;
	}
	// unset entry on error
	if (error != B_OK && entry)
		entry->Unset();
	return error;
}

// Contains
/*!	\brief Returns whether this directory or any of its subdirectories
	at any level contains the entry referred to by the supplied path name.
	Only entries that match the node flavor specified by \a nodeFlags are
	considered.
	If the BDirectory is not properly initialized, the method returns \c true,
	if the entry exists and has its kind does match. A non-absolute path is
	considered relative to the current directory.
	\param path the entry's path name. May be relative to this directory or
		   absolute.
	\param nodeFlags Any of the following:
		   - \c B_FILE_NODE: The entry must be a file.
		   - \c B_DIRECTORY_NODE: The entry must be a directory.
		   - \c B_SYMLINK_NODE: The entry must be a symbolic link.
		   - \c B_ANY_NODE: The entry may be of any kind.
	\return
	- \c true, if the entry exists, its kind does match \nodeFlags and the
	  BDirectory is either not properly initialized or it does contain the
	  entry at any level,
	- \c false, otherwise
*/
bool
BDirectory::Contains(const char *path, int32 nodeFlags) const
{
	bool result = true;
	if (path) {
		BEntry entry;
		if (InitCheck() == B_OK && !StorageKit::is_absolute_path(path))
			entry.SetTo(this, path);
		else
			entry.SetTo(path);
		result = Contains(&entry, nodeFlags);
	} else {
		// R5 behavior
		result = (InitCheck() == B_OK);
	}
	return result;
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
*/
bool
BDirectory::Contains(const BEntry *entry, int32 nodeFlags) const
{
	bool result = (entry);
	// check, if the entry exists at all
	if (result)
		result = entry->Exists();
	// test the node kind
	if (result) {
		switch (nodeFlags) {
			case B_FILE_NODE:
				result = entry->IsFile();
				break;
			case B_DIRECTORY_NODE:
				result = entry->IsDirectory();
				break;
			case B_SYMLINK_NODE:
				result = entry->IsSymLink();
				break;
			case B_ANY_NODE:
				break;
			default:
				result = false;
				break;
		}
	}
	// If the directory is initialized, get the canonical paths of the dir and
	// the entry and check, if the latter is a prefix of the first one.
	if (result && InitCheck() == B_OK) {
		char dirPath[B_PATH_NAME_LENGTH];
		char entryPath[B_PATH_NAME_LENGTH];
		result = (StorageKit::dir_to_path(get_fd(), dirPath,
										  B_PATH_NAME_LENGTH) == B_OK);
		entry_ref ref;
		if (result)
			result = (entry->GetRef(&ref) == B_OK);
		if (result) {
			result = (StorageKit::entry_ref_to_path(&ref, entryPath,
													B_PATH_NAME_LENGTH)
					  == B_OK);
		}
		if (result)
			result = !strncmp(dirPath, entryPath, strlen(dirPath));
	}
	return result;
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
*/
status_t
BDirectory::GetStatFor(const char *path, struct stat *st) const
{
	status_t error = (st ? B_OK : B_BAD_VALUE);
	if (error == B_OK && InitCheck() != B_OK)
		error = B_NO_INIT;
	if (error == B_OK) {
		if (path) {
			if (strlen(path) == 0)
				error = B_ENTRY_NOT_FOUND;
			else {
				BEntry entry(this, path);
				error == entry.InitCheck();
				if (error == B_OK)
					error = entry.GetStat(st);
			}
		} else
			error = GetStat(st);
	}
	return error;
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
*/
status_t
BDirectory::GetNextEntry(BEntry *entry, bool traverse)
{
	status_t error = (entry ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		entry_ref ref;
		error = GetNextRef(&ref);
		if (error == B_OK)
			entry->SetTo(&ref, traverse);
	}
	return error;
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
*/
status_t
BDirectory::GetNextRef(entry_ref *ref)
{
	status_t error = (ref ? B_OK : B_BAD_VALUE);
	if (error == B_OK && InitCheck() != B_OK)
		error = B_FILE_ERROR;
	if (error == B_OK) {
		StorageKit::LongDirEntry entry;
		bool next = true;
		while (error == B_OK && next) {
			if (StorageKit::read_dir(get_fd(), &entry, sizeof(entry), 1) != 1)
				error = B_ENTRY_NOT_FOUND;
			if (error == B_OK) {
				next = (!strcmp(entry.d_name, ".")
						|| !strcmp(entry.d_name, ".."));
			}
		}
		if (error == B_OK)
			*ref = entry_ref(entry.d_pdev, entry.d_pino, entry.d_name);
	}
	return error;
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
*/
int32
BDirectory::GetNextDirents(dirent *buf, size_t bufSize, int32 count)
{
	int32 result = (buf ? B_OK : B_BAD_VALUE);
	if (result == B_OK && InitCheck() != B_OK)
		result = B_FILE_ERROR;
	if (result == B_OK)
		result = StorageKit::read_dir(get_fd(), buf, bufSize, count);
	return result;
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
*/
status_t
BDirectory::Rewind()
{
	status_t error = B_OK;
	if (error == B_OK && InitCheck() != B_OK)
		error = B_FILE_ERROR;
	if (error == B_OK)
		error = StorageKit::rewind_dir(get_fd());
	return error;
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
*/
int32
BDirectory::CountEntries()
{
	status_t error = Rewind();
	int32 count = 0;
	if (error == B_OK) {
		StorageKit::LongDirEntry entry;
		while (error == B_OK) {
			if (StorageKit::read_dir(get_fd(), &entry, sizeof(entry), 1) != 1)
				error = B_ENTRY_NOT_FOUND;
			if (error == B_OK
				&& strcmp(entry.d_name, ".") && strcmp(entry.d_name, ".."))
				count++;
		}
		if (error == B_ENTRY_NOT_FOUND)
			error = B_OK;
	}
	Rewind();
	return (error == B_OK ? count : error);
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
*/
status_t
BDirectory::CreateDirectory(const char *path, BDirectory *dir)
{
	status_t error = (path ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		// get the actual (absolute) path using BEntry's help
		BEntry entry;
		if (InitCheck() == B_OK && !StorageKit::is_absolute_path(path))
			entry.SetTo(this, path);
		else
			entry.SetTo(path);
		error = entry.InitCheck();
		BPath realPath;
		if (error == B_OK)
			error = entry.GetPath(&realPath);
		if (error == B_OK)
			error = StorageKit::create_dir(realPath.Path());
		if (error == B_OK && dir)
			error = dir->SetTo(realPath.Path());
	}
	return error;
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
*/
status_t
BDirectory::CreateFile(const char *path, BFile *file, bool failIfExists)
{
	status_t error (path ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		// Let BFile do the dirty job.
		uint32 openMode = B_READ_WRITE | B_CREATE_FILE
						  | (failIfExists ? B_FAIL_IF_EXISTS : 0);
		BFile tmpFile;
		BFile *realFile = (file ? file : &tmpFile);
		if (InitCheck() == B_OK && !StorageKit::is_absolute_path(path))
			error = realFile->SetTo(this, path, openMode);
		else
			error = realFile->SetTo(path, openMode);
		if (error != B_OK)
			realFile->Unset();
	}
	return error;
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
*/
status_t
BDirectory::CreateSymLink(const char *path, const char *linkToPath,
						  BSymLink *link)
{
	status_t error = (path && linkToPath ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		// get the actual (absolute) path using BEntry's help
		BEntry entry;
		if (InitCheck() == B_OK && !StorageKit::is_absolute_path(path))
			entry.SetTo(this, path);
		else
			entry.SetTo(path);
		error = entry.InitCheck();
		BPath realPath;
		if (error == B_OK)
			error = entry.GetPath(&realPath);
		if (error == B_OK)
			error = StorageKit::create_link(realPath.Path(), linkToPath);
		if (error == B_OK && link)
			error = link->SetTo(realPath.Path());
	}
	return error;
}

// =
//! Assigns another BDirectory to this BDirectory.
/*!	If the other BDirectory is uninitialized, this one will be too. Otherwise
	it will refer to the same directory, unless an error occurs.
	\param dir the original BDirectory
	\return a reference to this BDirectory
*/
BDirectory &
BDirectory::operator=(const BDirectory &dir)
{
	Unset();
	if (dir.InitCheck() == B_OK) {
		// duplicate the file descriptor
		StorageKit::FileDescriptor fd = -1;
		status_t status = StorageKit::dup(dir.get_fd(), fd);
		// set it
		if (status == B_OK) {
			status = set_fd(fd);
			if (status != B_OK)
				StorageKit::close(fd);
		}
		set_status(status);
	}
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
void
BDirectory::close_fd()
{
	BNode::close_fd();
}

// set_fd
//! Sets the file descriptor for the BDirectory.
/*!	\param fd the new file descriptor, may be -1.
*/
status_t
BDirectory::set_fd(StorageKit::FileDescriptor fd)
{
	return BNode::set_fd(fd);
}

//! Returns the BDirectory's file descriptor.
/*! To be used instead of accessing the BNode's private \c fFd member directly.
	\return the file descriptor, or -1, if not properly initialized.
*/
StorageKit::FileDescriptor
BDirectory::get_fd() const
{
	return fFd;
}

//! Sets the BNode's status.
/*! To be used instead of accessing the BNode's private \c fCStatus member
	directly.
	\param newStatus the new status to be set.
*/
void
BDirectory::set_status(status_t newStatus)
{
	fCStatus = newStatus;
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
	\todo Check for efficency.
*/
status_t
create_directory(const char *path, mode_t mode)
{
printf("create_directory(`%s')\n", path);
	status_t error = (path ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		BEntry entry(path);
		switch (entry.InitCheck()) {
			case B_OK:
printf("  B_OK\n");
				// If an entry with this name exists, it should be a directory.
				// If none exists, create the directory.
				if (entry.Exists()) {
					if (!entry.IsDirectory())
						error = B_BAD_VALUE;
				} else
					error = StorageKit::create_dir(path, mode);
				break;
			case B_ENTRY_NOT_FOUND:
			{
printf("  B_ENTRY_NOT_FOUND\n");
/* Must not be used until BPath::GetParent() is implemented.
				// The parent dir doesn't exist. Create it first.
				BPath leafPath(path);
				error = leafPath.InitCheck();
printf("  entry\n");
				if (error == B_OK)
					error = leafPath.GetParent(&leafPath);
				if (error == B_OK)
					error = create_directory(leafPath.Path(), mode);
				if (error)
					error = StorageKit::create_dir(path, mode);
*/
error = B_ERROR;
				break;
			}
			default:
printf("  default\n");
				error = entry.InitCheck();
				break;
		}
	}
printf("create_directory(`%s') done\n", path);
	return error;
}


#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif
