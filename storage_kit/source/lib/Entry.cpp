//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//---------------------------------------------------------------------
/*!
	\file Entry.cpp
	BEntry and entry_ref implementations.
*/

#include <Entry.h>

#include <Directory.h>
#include <Path.h>
#include "kernel_interface.h"
#include "storage_support.h"

#ifdef USE_OPENBEOS_NAMESPACE
using namespace OpenBeOS;
#endif

//----------------------------------------------------------------------------
// struct entry_ref
//----------------------------------------------------------------------------

/*! \struct entry_ref
	\brief A filesystem entry represented as a name in a concrete directory.
	
	entry_refs may refer to pre-existing (concrete) files, as well as non-existing
	(abstract) files. However, the parent directory of the file \b must exist.
	
	The result of this dichotomy is a blending of the persistence gained by referring
	to entries with a reference to their internal filesystem node and the flexibility gained
	by referring to entries by name.
	
	For example, if the directory in which the entry resides (or a
	directory further up in the hierarchy) is moved or renamed, the entry_ref will
	still refer to the correct file (whereas a pathname to the previous location of the
	file would now be invalid).
	
	On the other hand, say that the entry_ref refers to a concrete file. If the file
	itself is renamed, the entry_ref now refers to an abstract file with the old name
	(the upside in this case is that abstract entries may be represented by entry_refs
	without	preallocating an internal filesystem node for them).
*/


//! Creates an unitialized entry_ref. 
entry_ref::entry_ref() : device(0), directory(0), name(NULL) {
}

/*! \brief Creates an entry_ref initialized to the given file name in the given
	directory on the given device.
	
	\p name may refer to either a pre-existing file in the given
	directory, or a non-existent file. No explicit checking is done to verify validity of the given arguments, but
	later use of the entry_ref will fail if \p dev is not a valid device or \p dir
	is a not a directory on \p dev.
	
	\param dev the device on which the entry's parent directory resides
	\param dir the directory in which the entry resides
	\param name the leaf name of the entry, which is not required to exist
*/
entry_ref::entry_ref(dev_t dev, ino_t dir, const char *name) :
	device(dev), directory(dir), name(NULL) {
	set_name(name);
}

/*! \brief Creates a copy of the given entry_ref.

	\param ref a reference to an entry_ref to copy
*/
entry_ref::entry_ref(const entry_ref &ref) : device(ref.device), directory(ref.directory),
	name(NULL) {
	set_name(ref.name);	
}

//! Destroys the object and frees the storage allocated for the leaf name, if necessary. 
entry_ref::~entry_ref() {
	if (name != NULL)
		delete [] name;
}

/*! \brief Set the entry_ref's leaf name, freeing the storage allocated for any previous
	name and then making a copy of the new name.
	
	\param name pointer to a null-terminated string containing the new name for
	the entry. May be \c NULL.
*/
status_t entry_ref::set_name(const char *name) {
	if (this->name != NULL) {
		delete [] this->name;
	}
	
	if (name == NULL) {
		this->name = NULL;
	} else {
		this->name = new char[strlen(name)+1];
		if (this->name == NULL)
			return B_NO_MEMORY;
		strcpy(this->name, name);
	}
	
	return B_OK;			
}

/*! \brief Compares the entry_ref with another entry_ref, returning true if they are equal.
	\return
	- \c true: the entry_refs are equal
	- \c false: the entry_refs are not equal
*/
bool
entry_ref::operator==(const entry_ref &ref) const {
	return (	device == ref.device &&
				directory == ref.directory &&
				strcmp(name, ref.name) == 0		);
}

/*! \brief Compares the entry_ref with another entry_ref, returning true if they are not equal.
	\return
	- \c true: the entry_refs are not equal
	- \c false: the entry_refs are equal
*/
bool
entry_ref::operator!=(const entry_ref &ref) const {
	return !(*this == ref);
}

/*! \brief Makes the entry_ref a copy of the entry_ref specified by \a ref.
	\param ref then entry_ref to copy
	\return
	- a reference to the copy
*/
entry_ref&
entry_ref::operator=(const entry_ref &ref) {
	if (this == &ref)
		return *this;	

	device = ref.device;
	directory = ref.directory;
	set_name(ref.name);
	return *this;
}

/*!
	\var dev_t entry_ref::device
	\brief The device id of the storage device on which the entry resides

*/

/*!
	\var ino_t entry_ref::directory
	\brief The inode number of the directory in which the entry resides
*/

/*!
	\var char *entry_ref::name
	\brief The leaf name of the entry
*/


//----------------------------------------------------------------------------
// BEntry
//----------------------------------------------------------------------------

/*!
	\class BEntry
	\brief A location in the filesystem
	
	The BEntry class defines objects that represent "locations" in the file system
	hierarchy.  Each location (or entry) is given as a name within a directory. For
	example, when you create a BEntry thus.
	
	BEntry entry("/boot/home/fido");
	
	...you're telling the BEntry object to represent the location of the file
	called fido within the directory "/boot/home".
	
	\author <a href='mailto:tylerdauwalder@users.sf.net'>Tyler Dauwalder</a>
	\author <a href='mailto:scusack@users.sf.net'>Simon Cusack</a>
	
	\version 0.0.0
*/

//! Creates an uninitialized BEntry object.
/*!	Should be followed by a	call to one of the SetTo functions,
	or an assignment:
	- SetTo(const BDirectory*, const char*, bool)
	- SetTo(const entry_ref*, bool)
	- SetTo(const char*, bool)
	- operator=(const BEntry&)
*/
BEntry::BEntry() :
	fCStatus(B_NO_INIT),
	fDirFd(StorageKit::NullFd),
	fName(NULL)
{
}

//! Creates a BEntry initialized to the given directory and path combination.
/*!	If traverse is true and \c dir/path refers to a symlink, the BEntry will
	refer to the linked file; if false,	the BEntry will refer to the symlink itself.
	
	\param dir directory in which \a path resides
	\param path relative path reckoned off of \a dir
	\param traverse whether or not to traverse symlinks
	\see SetTo(const BDirectory*, const char *, bool)

*/
BEntry::BEntry(const BDirectory *dir, const char *path, bool traverse = false) :
	fCStatus(B_NO_INIT),
	fDirFd(StorageKit::NullFd),
	fName(NULL)
{
	SetTo(dir, path, traverse);
};

//! Creates a BEntry for the file referred to by the given entry_ref.
/*!	If traverse is true and \a ref refers to a symlink, the BEntry
	will refer to the linked file; if false, the BEntry will refer
	to the symlink itself.
	
	\param ref the entry_ref referring to the given file
	\param traverse whether or not symlinks are to be traversed
	\see SetTo(const entry_ref*, bool)
*/

BEntry::BEntry(const entry_ref *ref, bool traverse = false) :
	fCStatus(B_NO_INIT),
	fDirFd(StorageKit::NullFd),
	fName(NULL)
{
	SetTo(ref, traverse);
};

//! Creates a BEntry initialized to the given path.
/*!	If \a path is relative, it will
	be reckoned off the current working directory. If \a path refers to a symlink and
	traverse is true, the BEntry will refer to the linked file. If traverse is false,
	the BEntry will refer to the symlink itself.
	
	\param path the file of interest
	\param traverse whether or not symlinks are to be traversed	
	\see SetTo(const char*, bool)
	
*/
BEntry::BEntry(const char *path, bool traverse = false) :
	fCStatus(B_NO_INIT),
	fDirFd(StorageKit::NullFd),
	fName(NULL)
{
	SetTo(path, traverse);
};

//! Creates a copy of the given BEntry.
/*! \param entry the entry to be copied
	\see operator=(const BEntry&)
*/
BEntry::BEntry(const BEntry &entry) :
	fCStatus(B_NO_INIT),
	fDirFd(StorageKit::NullFd),
	fName(NULL)
{
	entry_ref ref;
	status_t status;
	
	fCStatus = entry.GetRef(&ref);
	if (fCStatus == B_OK)
		SetTo(&ref, false);
			// Here we don't want to traverse, since we need to be
			// straight copy of the given entry
};

//! Frees all of the BEntry's allocated resources.
/*! \see Unset()
*/
BEntry::~BEntry(){
	Unset();
};

//! Returns the result of the most recent construction or SetTo() call.
/*! \return
		- \c B_OK success
		- \c B_NO_INIT the object has been Unset() or is uninitialized
		- <code>some error code</code>
*/
status_t
BEntry::InitCheck() const {
	return fCStatus;
};

//! Returns true if the Entry exists in the filesytem, false otherwise. 
bool
BEntry::Exists() const {
	if (fCStatus != B_OK)
		return fCStatus;
		
	// Attempt to find the entry in our current directory
	StorageKit::LongDirEntry entry;
	return StorageKit::find_dir(fDirFd, fName, &entry, sizeof(entry)) == B_OK;
};

/*! \brief Fills in a stat structure for the entry. The information is copied into
	the \c stat structure pointed to by \a result.
	
	\b NOTE: The BStatable object does not cache the stat structure; every time you 
	call GetStat(), fresh stat information is retrieved.
	
	\param result pointer to a pre-allocated structure into which the stat information will be copied
	\return
	- \c B_OK success
	- "error code" another error code
*/
status_t
BEntry::GetStat(struct stat *result) const{
	if (fCStatus != B_OK)
		return fCStatus;
		
	entry_ref ref;
	status_t status = StorageKit::find_dir(fDirFd, fName, &ref);
	if (status != B_OK)
		return status;
		
	return StorageKit::get_stat(ref, result);
};

/*! Reinitializes the BEntry to the path or directory path combination,
	resolving symlinks if traverse is true */
status_t
BEntry::SetTo(const BDirectory *dir, const char *path, bool traverse = false){
	//! @todo Perhaps we should return dir->fCStatus if dir->fCStatus != B_OK ???
	if (dir == NULL || dir->fCStatus != B_OK || path == NULL)
		return B_BAD_VALUE;
		
	char rootPath[B_PATH_NAME_LENGTH];

	fCStatus = StorageKit::dir_to_path(dir->get_fd(), rootPath,
									   B_PATH_NAME_LENGTH);
	if (fCStatus != B_OK)
		return fCStatus;
	
	//! @todo Need to verify whether the R5 implemenation returns an error if path is not relative 
	
	// Concatenate our two path strings together
	sprintf(rootPath, "%s/%s", rootPath, path);
	
	return SetTo(rootPath, traverse);
};
				  
/*! Reinitializes the BEntry to the entry_ref, resolving symlinks if
	traverse is true */
status_t
BEntry::SetTo(const entry_ref *ref, bool traverse = false){
	if (ref == NULL)
		return B_BAD_VALUE;

	char path[B_PATH_NAME_LENGTH];

	fCStatus = StorageKit::entry_ref_to_path(ref, path, B_PATH_NAME_LENGTH);
	return (fCStatus == B_OK) ? SetTo(path, traverse) : fCStatus ;
};

/*! Reinitializes the BEntry object to the path, resolving symlinks if
	traverse is true */
status_t
BEntry::SetTo(const char *path, bool traverse = false) {
	Unset();

	if (path != NULL) {
		// Get the path and leaf portions of the given path
		char *pathStr, *leafStr;
		pathStr = leafStr = NULL;
		if (StorageKit::split_path(path, pathStr, leafStr)) {
//			printf("path == '%s'\n", pathStr);
//			printf("leaf == '%s'\n", leafStr);
			
			// Open the directory
			StorageKit::FileDescriptor dirFd;
			fCStatus = StorageKit::open_dir(pathStr, dirFd);
			if (fCStatus == B_OK) {
				fCStatus = set(dirFd, leafStr, traverse);
				if (fCStatus != B_OK)
					StorageKit::close_dir(dirFd);		
			}				
		}
		
		delete [] pathStr;
		delete [] leafStr;
	}
	
	return fCStatus;
};

/*! Reinitializes the BEntry to an uninitialized BEntry object */
void
BEntry::Unset() {
	// Close the directory
	if (fDirFd != StorageKit::NullFd) {
		StorageKit::close_dir(fDirFd);
	}
	
	// Free our leaf name
	if (fName != NULL) {
		delete [] fName;
	}

	fDirFd = StorageKit::NullFd;
	fName = NULL;
	fCStatus = B_NO_INIT;
};

/*! Gets an entry_ref structure from the BEntry */
status_t
BEntry::GetRef(entry_ref *ref) const {
	if (fCStatus != B_OK)
		return fCStatus;

	if (ref == NULL)
		return B_BAD_VALUE;
	
	// Unset the entry_ref
	ref->device = ref->directory = 0;
	ref->set_name(NULL);

	status_t status;
		
	// Convert our directory to an self-referencing entry_ref 
	status = StorageKit::dir_to_self_entry_ref(fDirFd, ref);
	if (status != B_OK)
		return status;
		
	// Change the name from "." to our leaf name
	return ref->set_name(fName);
};

/*! Gets the path for the BEntry */
status_t
BEntry::GetPath(BPath *path) const {
	if (fCStatus != B_OK)
		return fCStatus;

	if (path == NULL)
		return B_BAD_VALUE;
		
	entry_ref ref;
	status_t status;
	
	status = GetRef(&ref);
	if (status != B_OK)
		return status;
		
	path->SetTo(&ref);
	return path->InitCheck();
};

/*! Gets the parent of the BEntry as another BEntry. */
status_t BEntry::GetParent(BEntry *entry) const {
	if (fCStatus != B_OK)
		return fCStatus;
	
	if (entry == NULL)
		return B_BAD_VALUE;
		
	// Convert ourselves to a BEntry and change the
	// leaf name to "."

	entry_ref ref;
	status_t status;

	status = GetRef(&ref);
	if (status == B_OK) {
	
		// Verify we aren't an entry representing "/"
		status = StorageKit::entry_ref_is_root_dir(ref) ? B_ENTRY_NOT_FOUND : B_OK ;
		if (status == B_OK) {

			status = ref.set_name(".");
			if (status == B_OK) {
			
				entry->SetTo(&ref);
				return entry->InitCheck();
				
			}
		}
	}
	
	// If we get this far, an error occured, so we Unset() the
	// argument as dictated by the BeBook
	entry->Unset();
	return status;

};

/*! Gets the parent of the BEntry as a BDirectory. */
status_t
BEntry::GetParent(BDirectory *dir) const {
	if (fCStatus != B_OK)
		return fCStatus;
		
	if (dir == NULL)
		return B_BAD_VALUE;
	
	entry_ref ref;
	status_t status;
	
	status = GetRef(&ref);
	if (status == B_OK) {
	
		// Verify we aren't an entry representing "/"
		status = StorageKit::entry_ref_is_root_dir(ref) ? B_ENTRY_NOT_FOUND : B_OK ;
		if (status == B_OK) {

			// Now point the entry_ref to the parent directory (instead of ourselves)
			status = ref.set_name(".");
			if (status == B_OK) {
				dir->SetTo(&ref);
				return dir->InitCheck();
			}
		}
	}
	
	// If we get this far, an error occured, so we Unset() the
	// argument as dictated by the BeBook
	dir->Unset();
	return status;
	
};

/*! Gets the name of the entry's leaf. */
status_t
BEntry::GetName(char *buffer) const {
	status_t result = B_ERROR;
	
	if (fCStatus != B_OK) {
		result = fCStatus;
	} else if (buffer == NULL) {
		result = B_BAD_VALUE;
	} else {
		strcpy(buffer, fName);
		result = B_OK;
	}
	
	// Set buffer to NULL on error per R5 BeBook
//	if (result != B_OK)
//		buffer = NULL;

	return result;
};

/*! Renames the BEntry to path, replacing an existing entry if clobber is true. */
status_t
BEntry::Rename(const char *path, bool clobber = false) {
//	printf("Rename( '%s' )\n", path);
	if (fCStatus != B_OK)
		return fCStatus;
		
	status_t status;
	
	// We'll handle absolute paths locally, and pass relative paths on
	// to MoveTo()
	if (path[0] == '/') {
		// Absolute path
		if (!clobber) {
			// We're not supposed to kill an already-existing file,
			// so we'll try to figure out if it exists by stat()ing it.
			StorageKit::Stat s;
			status = StorageKit::get_stat(path, &s);
			if (status == B_OK) {
				return B_FILE_EXISTS;
			} else if (status != B_ENTRY_NOT_FOUND) {
				return status;
			}
		}
		
		// Turn ourselves into a pathname, rename ourselves
		BPath oldPath;
		status = GetPath(&oldPath);
		if (status == B_OK) {
			status = StorageKit::rename(oldPath.Path(), path);
			if (status == B_OK) {
				SetTo(path, false);
				status = InitCheck();
			}
		}
	
		return status;
	} else {
		// Get our parent directory
		BDirectory dir;
		status = GetParent(&dir);
		if (status == B_ENTRY_NOT_FOUND) {
			// Apparently we are a BEntry representing the root directory,
			// which you're not allowed to rename.
			return B_NOT_ALLOWED;
		} else if (status == B_OK) {
			// Let MoveTo() handle the dirty work
			return MoveTo(&dir, path, clobber);		
		} else {
			return status;
		}
	}
		
	
};

/*! Moves the BEntry to path or dir path combination, replacing an existing entry if clober is true. */
status_t
BEntry::MoveTo(BDirectory *dir, const char *path = NULL, bool clobber = false) {
//	printf("MoveTo(dir, '%s')\n", path);
	if (fCStatus != B_OK)
		return fCStatus;
	else if (dir == NULL)
		return B_BAD_VALUE;
	else if (dir->InitCheck() != B_OK)
		return dir->InitCheck();

	// NULL path simply means move without renaming
	if (path == NULL)
		MoveTo(dir, fName);
	else if (path[0] == '/')
		return B_BAD_VALUE;
	
	status_t status;
		
	// Convert our directory to an absolute pathname
	char fullPath[B_PATH_NAME_LENGTH];
	status = StorageKit::dir_to_path(dir->get_fd(), fullPath, B_PATH_NAME_LENGTH);
	if (status != B_OK)
		return status;
		
	// Concatenate our pathname to it
	sprintf(fullPath, "%s/%s", fullPath, path);
	
	// Now let rename do the dirty work
	return Rename(fullPath, clobber);
};

/*! Removes the entry from the file system. */
status_t
BEntry::Remove() {
	if (fCStatus != B_OK)
		return fCStatus;
		
	BPath path;
	status_t status;
	
	status = GetPath(&path);
	if (status != B_OK)
		return status;
		
	return StorageKit::remove(path.Path());
};


/*! Equality operator */
bool
BEntry::operator==(const BEntry &item) const {

	// First check statuses
	if (this->InitCheck() == B_NO_INIT && item.InitCheck() == B_NO_INIT) {
		return true;
	} else if (this->InitCheck() == B_OK && item.InitCheck() == B_OK) {

		// Directories don't compare well directly, so we'll
		// compare entry_refs instead
		entry_ref ref1, ref2;
		if (this->GetRef(&ref1) != B_OK)
			return false;
		if (item.GetRef(&ref2) != B_OK)
			return false;
		return (ref1 == ref2);

	} else {
		return false;
	}	

};

/*! Inequality operator */
bool
BEntry::operator!=(const BEntry &item) const {
	return !(*this == item);
};

/*! Reinitializes the BEntry to be a copy of the arguement */
BEntry&
BEntry::operator=(const BEntry &item) {
	if (this == &item)
		return *this;

	Unset();
	if (item.fCStatus == B_OK) {
		fCStatus = StorageKit::dup_dir(item.fDirFd, fDirFd);
		if (fCStatus == B_OK) {
			fCStatus = set_name(item.fName);
		}
	}
	
	return *this;
};

/*! Reserved for future use. */
void BEntry::_PennyEntry1(){};
/*! Reserved for future use. */
void BEntry::_PennyEntry2(){};
/*! Reserved for future use. */
void BEntry::_PennyEntry3(){};
/*! Reserved for future use. */
void BEntry::_PennyEntry4(){};
/*! Reserved for future use. */
void BEntry::_PennyEntry5(){};
/*! Reserved for future use. */
void BEntry::_PennyEntry6(){};

/*! Updates the BEntry with the data from the stat structure according to the mask. */
status_t
BEntry::set_stat(struct stat &st, uint32 what){
	if (fCStatus != B_OK)
		return B_FILE_ERROR;
	
	BPath path;
	status_t status;
	
	status = GetPath(&path);
	if (status != B_OK)
		return status;
	
	return StorageKit::set_stat(path.Path(), st, what);
};

/*! Sets the Entry to point to the entry named by path in the given directory. If traverse
	is true and the given entry is a symlink, the object is recursively set to point to the
	entry pointed to by the symlink.
	
	<code>leaf</code> <b>must</b> be a leaf-name only (i.e. it must contain no '/' characters),
	otherwise this function will return B_BAD_VALUE. If B_OK is returned, 
	the caller is no longer responsible for StorageKit::close_dir()ing dir. */
status_t
BEntry::set(StorageKit::FileDescriptor dirFd, const char *leaf, bool traverse) {
//	printf("leaf == %s\n", leaf);

	// Verify that path is valid
	if (leaf == NULL) {
		return B_BAD_VALUE;
	} else {
		/*! /todo This should actually be unnecessary once all the
			SetTo() functions have been throughly tested. */
		int len = strlen(leaf);
		for (int i = 0; i < len; i++) {
			if (leaf[i] == '/')
				return B_BAD_VALUE;
		}		
	}

	// If we aren't traversing, then we're done (skip to the bottom of
	// the function). Otherwise, we need to see if leaf refers to a
	// symbolic link. If so, we need to set ourselves to whatever it
	// points to.
	if (traverse) {
	
		StorageKit::LongDirEntry entry;
		while (	StorageKit::read_dir(dirFd, &entry, sizeof(entry), 1) == 1) {

			if (strcmp(entry.d_name, leaf) == 0) {
//				printf("Found Leaf '%s'\n", leaf);
//				printf("Found PDev %d\n", entry->d_pdev);
//				printf("Found PIno %d\n", entry->d_pino);
			
				// We've found the entry in the directory, now we convert
				// it to an absolute pathname and find out if it's a symbolic
				// link. If so, we traverse it.
				char path[B_PATH_NAME_LENGTH+1];
				status_t result = StorageKit::entry_ref_to_path(entry.d_pdev,
					entry.d_pino, leaf, path, B_PATH_NAME_LENGTH+1);
//				printf("+Found PDev %d\n", entry->d_pdev);
//				printf("+Found PIno %d\n", entry->d_pino);
					
				if (result == B_OK) {
//					printf("Entry == '%s'\n", path);	// Prints out the full path of this entry
				
					
					// Attempt to read the name of the file the link points to.
					// If this fails, the entry is not a symlink.
					char target[B_PATH_NAME_LENGTH+1];
					ssize_t len = StorageKit::read_link(path, target, B_PATH_NAME_LENGTH+1);
//					printf("len == %d\n", len);
					if (len >= 0) {
//						printf("target == %s\n", target);
						// target is now the pathname of the entry the link points
						// to, so we now recursively SetTo() ourselves to this
						// new entry. If target is absolute, we just set ourselves
						// to the target, otherwise we need to convert the relative
						// path to an absolute path and then call SetTo()
						if (target[0] == '/') {
						
							// Absolute path
							status_t result = SetTo(target, traverse);
							if (result == B_OK) {
								StorageKit::close_dir(dirFd);
									// We're responsible for dir when successful
							}
							
							return result;
							
						} else {
						
							// Relative path
							status_t result = StorageKit::entry_ref_to_path(entry.d_pdev,
								entry.d_pino, target, path, B_PATH_NAME_LENGTH+1);
//							printf("result == 0x%X\n", result);
							if (result == B_OK) {
//								printf("path == '%s', traversing...\n", path);
								result = SetTo(path, traverse);
							}
							
							if (result == B_OK) {
								StorageKit::close_dir(dirFd);
									// We're responsible for dir when successful
							}							

							return result;
							
						}							
							
					} else {
						// If we get here, we found the entry in the given directory,
						// but it was not a symlink, so we can't traverse it. Thus
						// we just want to set ourselves to the given entry.
						break;
					}

				} else {
					// Error converting entry to full pathname; make the entry abstract
					break;
				}
			}
		}
	}
		
	/*
	   If we get this far, either:
		1. We're explicitly not traversing
		2. We couldn't find the given entry in the directory (thus
			it must be abstract)
		2. We found the given entry but it was not a symlink (thus
			it could not be traversed).
		3. There was an error converting the entry to a full pathname
			(thus we're taking the easy way out and making it abstract).
	*/
	
	fDirFd = dirFd;
	set_name(leaf);
	return B_OK;
};

/*! Handles string allocation, deallocation, and copying for our leaf name. */
status_t
BEntry::set_name(const char *name) {
	if (name == NULL)
		return B_BAD_VALUE;	
	
	if (fName != NULL) {
		delete [] fName;
	}
	
	fName = new char[strlen(name)+1];
	if (fName == NULL)
		return B_NO_MEMORY;
		
	strcpy(fName, name);
	
	return B_OK;
}


/*! Debugging function, dumps the given entry to stdout. This function is not part of
	the R5 implementation, and thus calls to it will mean you can't link with the
	R5 Storage Kit. */
void
BEntry::Dump(const char *name = NULL) {
	if (name != NULL) {
		printf("------------------------------------------------------------\n");
		printf("%s\n", name);
		printf("------------------------------------------------------------\n");
	}
	
	printf("fCStatus == %d\n", fCStatus);
	
	StorageKit::LongDirEntry entry;
	if (fDirFd != -1
		&& StorageKit::find_dir(fDirFd, ".", &entry, sizeof(entry)) == B_OK) {
		printf("dir.device == %ld\n", entry.d_pdev);
		printf("dir.inode  == %lld\n", entry.d_pino);
	} else {
		printf("dir == NullFd\n");
	}
	
	printf("leaf == '%s'\n", fName);
	printf("\n");

}
