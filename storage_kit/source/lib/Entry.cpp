//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Entry.cpp
//  Description:  A file location wrapper class
//---------------------------------------------------------------------
#include <Entry.h>

#include <Directory.h>
#include <Path.h>

#ifdef USE_OPENBEOS_NAMESPACE
using namespace OpenBeOS;
#endif

//----------------------------------------------------------------------------
// struct entry_ref
//----------------------------------------------------------------------------

entry_ref::entry_ref() : device(0), directory(0), name(NULL) {
}

entry_ref::entry_ref(dev_t dev, ino_t dir, const char *name) :
	device(dev), directory(dir), name(NULL) {
	set_name(name);
}

entry_ref::entry_ref(const entry_ref &ref) : device(ref.device), directory(ref.directory),
	name(NULL) {
	set_name(ref.name);	
}

entry_ref::~entry_ref() {
	if (name != NULL)
		delete [] name;
}
	
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

bool
entry_ref::operator==(const entry_ref &ref) const {
	return (	device == ref.device &&
				directory == ref.directory &&
				strcmp(name, ref.name) == 0		);
}

bool
entry_ref::operator!=(const entry_ref &ref) const {
	return !(*this == ref);
}

entry_ref&
entry_ref::operator=(const entry_ref &ref) {
	if (this == &ref)
		return *this;	

	device = ref.device;
	directory = ref.directory;
	set_name(ref.name);
	return *this;
}


//----------------------------------------------------------------------------
// BEntry
//----------------------------------------------------------------------------

/*! Creates an uninitialized BEntry object. */
BEntry::BEntry() :
	fCStatus(B_NO_INIT),
	fDirFd(StorageKit::NullFd),
	fName(NULL)
{
}

/*! Creates a BEntry object initialized to path or a dir path combination,
	resolves symlinks if traverse is true */
BEntry::BEntry(const BDirectory *dir, const char *path, bool traverse = false) :
	fCStatus(B_NO_INIT),
	fDirFd(StorageKit::NullFd),
	fName(NULL)
{
	SetTo(dir, path, traverse);
};

/*! Creates a BEntry object initialized to the entry_ref, resolves
	symlinks if traverse is true */
BEntry::BEntry(const entry_ref *ref, bool traverse = false) :
	fCStatus(B_NO_INIT),
	fDirFd(StorageKit::NullFd),
	fName(NULL)
{
	SetTo(ref, traverse);
};

/*! Creates a BEntry object initialized to the path, resolves
	symlinks if traverse is true */
BEntry::BEntry(const char *path, bool traverse = false) :
	fCStatus(B_NO_INIT),
	fDirFd(StorageKit::NullFd),
	fName(NULL)
{
	SetTo(path, traverse);
};

/*! Copy constructor; creates a BEntry object initialized as a duplicate of another entry. */
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

/*! Destructor, frees all previously allocated resources */
BEntry::~BEntry(){
	Unset();
};

/*! Returns the status of the most recent construction or
	SetTo() call */
status_t
BEntry::InitCheck() const {
	return fCStatus;
};

/*! Returns true if the Entry exists in the filesytem, false otherwise. */
bool
BEntry::Exists() const {
	if (fCStatus != B_OK)
		return fCStatus;
		
	// Attempt to find the entry in our current directory
	StorageKit::DirEntry *entry;
	return StorageKit::find_dir(fDirFd, fName, entry) == B_OK;
};

/*! Gets a stat structure for the Entry */
status_t
BEntry::GetStat(struct stat *result) const{
	if (fCStatus != B_OK)
		return fCStatus;
		
	entry_ref ref;
	status_t status = StorageKit::find_dir(fDirFd, fName, ref);
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

	fCStatus = StorageKit::dir_to_path(dir->fDir, rootPath, B_PATH_NAME_LENGTH);
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
		if (SplitPathInTwain(path, pathStr, leafStr)) {
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
			dir->SetTo(&ref);
			return dir->InitCheck();
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
	if (fCStatus != B_OK)
		return fCStatus;
		
//	if (!Exists())
//		return B_ENTRY_NOT_FOUND;
		
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
		// Haven't implemented this yet
		return B_ERROR;
	
		// Get our parent directory
		BDirectory dir;
		status = GetParent(&dir);
		if (status == B_ENTRY_NOT_FOUND) {
			// Apparently we are a BEntry representing the root directory,
			// which you're not allowed to rename.
			return B_NOT_ALLOWED;
		}
	}
		
	
};

/*! Moves the BEntry to path or dir path combination, replacing an existing entry if clober is true. */
status_t
BEntry::MoveTo(BDirectory *dir, const char *path = NULL, bool clobber = false) {
	if (fCStatus != B_OK)
		return fCStatus;
		
	return B_ERROR;
};

/*! Removes the entry from the file system */
status_t
BEntry::Remove() {
	if (fCStatus != B_OK)
		return fCStatus;

	return B_ERROR;
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
			fCStatus = SetName(item.fName);
		}
	}
	
	return *this;
};

/*! Currently unused. */
void BEntry::_PennyEntry1(){};
/*! Currently unused. */
void BEntry::_PennyEntry2(){};
/*! Currently unused. */
void BEntry::_PennyEntry3(){};
/*! Currently unused. */
void BEntry::_PennyEntry4(){};
/*! Currently unused. */
void BEntry::_PennyEntry5(){};
/*! Currently unused. */
void BEntry::_PennyEntry6(){};

/*! Updates the BEntry with the data from the stat structure according to the mask. */
status_t
BEntry::set_stat(struct stat &st, uint32 what){
};

/*! Probably called by MoveTo. */
status_t
BEntry::move(int fd, const char *path){
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
	
		StorageKit::DirEntry *entry;
		for (	entry = StorageKit::read_dir(dirFd);
				entry != NULL;
				entry = StorageKit::read_dir(dirFd)	) {

			if (strcmp(entry->d_name, leaf) == 0) {
//				printf("Found Leaf '%s'\n", leaf);
//				printf("Found PDev %d\n", entry->d_pdev);
//				printf("Found PIno %d\n", entry->d_pino);
			
				// We've found the entry in the directory, now we convert
				// it to an absolute pathname and find out if it's a symbolic
				// link. If so, we traverse it.
				char path[B_PATH_NAME_LENGTH+1];
				status_t result = StorageKit::entry_ref_to_path(entry->d_pdev,
					entry->d_pino, leaf, path, B_PATH_NAME_LENGTH+1);
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
							status_t result = StorageKit::entry_ref_to_path(entry->d_pdev,
								entry->d_pino, target, path, B_PATH_NAME_LENGTH+1);
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
	SetName(leaf);
	return B_OK;
};

/*! Possibly called by Unset() */
status_t
BEntry::clear(){
};

/*! Handles string allocation, deallocation, and copying for our leaf name. */
status_t
BEntry::SetName(const char *name) {
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

/*! Separates the path and leaf portions of the given full path, returning
	them separately in freshly new'd strings placed into the pointers referenced
	by path and leaf (i.e. YOU MUST delete [] BOTH path AND leaf WHEN YOU'RE
	FINISHED WITH THEM).
		
	path and leaf are assumed to not point to anything useful. They'll be set
	to NULL if the function fails, otherwise they'll point to their respective
	strings. */
bool
BEntry::SplitPathInTwain(const char* fullPath, char *&path, char *&leaf) {
	path = leaf = NULL;

	if (fullPath == NULL)
		return false;

	enum PathParserState { PPS_START, PPS_LEAF, PPS_END } state = PPS_START;

	int len = strlen(fullPath);
	
	int leafStart = -1;
	int leafEnd = -1;
	int pathEnd = -2;
	
	bool loop = true;
	for (int pos = len-1; ; pos--) {
		if (pos < 0)
			break;
			
		switch (state) {
			case PPS_START:
				// Skip all trailing '/' chars, then move on to
				// reading the leaf name
				if (fullPath[pos] != '/') {
					leafEnd = pos;
					state = PPS_LEAF;
				}
				break;
			
			case PPS_LEAF:
				// Read leaf name chars until we hit a '/' char
				if (fullPath[pos] == '/') {
					leafStart = pos+1;
					pathEnd = pos-1;
					loop = false;
				}
				break;					
		}
		
		if (!loop)
			break;
	}
	
	// Tidy up/handle special cases
	if (leafEnd == -1) {
	
		// Handle special cases 
		if (fullPath[0] == '/') {
			// Handle "/"
			path = new char[2];
			path[0] = '/';
			path[1] = 0;
			leaf = new char[2];
			leaf[0] = '.';
			leaf[1] = 0;
			return true;	
		} else if (fullPath[0] == 0) {
			// Handle "", which we'll treat as "./"
			path = new char[1];
			path[0] = 0;
			leaf = new char[2];
			leaf[0] = '.';
			leaf[1] = 0;
			return true;	
		}
	
	} else if (leafStart == -1) {
		// fullPath is just an entry name, no parent directories specified
		leafStart = 0;
	} else if (pathEnd == -1) {
		// The path is '/' (since pathEnd would be -2 if we had
		// run out of characters before hitting a '/')
		pathEnd = 0;
	}
	
	// Alloc new strings and copy the path and leaf over
	if (pathEnd == -2) {
		// empty path
		path = new char[2];
		path[0] = '.';
		path[1] = 0;
	} else {
		// non-empty path
		len = pathEnd + 1;
		path = new char[len+1];
		memcpy(path, fullPath, len);
		path[len] = 0;
	}
	
	len = leafEnd - leafStart + 1;
	leaf = new char[len+1];
	memcpy(leaf, fullPath + leafStart, len);
	leaf[len] = 0;
	
	return true;
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
	
	StorageKit::DirEntry *entry;
	if (fDirFd != StorageKit::NullFd && StorageKit::find_dir(fDirFd, ".", entry) == B_OK) {
		printf("dir.device == %ld\n", entry->d_pdev);
		printf("dir.inode  == %lld\n", entry->d_pino);
	} else {
		printf("dir == NullFd\n");
	}
	
	printf("leaf == '%s'\n", fName);
	printf("\n");

}
