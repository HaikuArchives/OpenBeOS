//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Entry.cpp
//  Description:  A file location wrapper class
//---------------------------------------------------------------------
#include <Entry.h>

#include <Directory.h>

#ifdef USE_OPENBEOS_NAMESPACE
using namespace OpenBeOS;
#endif

/*! Creates an uninitialized BEntry object. */
BEntry::BEntry() :
	fCStatus(B_NO_INIT),
	fDir(StorageKit::NullDir),
	fLeaf(NULL)
{
}

/*! Creates a BEntry object initialized to path or a dir path combination,
	resolves symlinks if traverse is true */
BEntry::BEntry(const BDirectory *dir, const char *path, bool traverse = false){
	SetTo(dir, path, traverse);
};

/*! Creates a BEntry object initialized to the entry_ref, resolves
	symlinks if traverse is true */
BEntry::BEntry(const entry_ref *ref, bool traverse = false){
	SetTo(ref, traverse);
};

/*! Creates a BEntry object initialized to the path, resolves
	symlinks if traverse is true */
BEntry::BEntry(const char *path, bool traverse = false) :
	fCStatus(B_NO_INIT),
	fDir(StorageKit::NullDir),
	fLeaf(NULL)
{
	SetTo(path, traverse);
};

/*! Copy constructor; creates a BEntry object initialized another entry. */
BEntry::BEntry(const BEntry &entry){
};

/*! Destructor, frees all previously allocated resources */
BEntry::~BEntry(){
	Unset();
};

/*! Returns the status of the most recent construction or
	SetTo() call */
status_t BEntry::InitCheck() const {
	return fCStatus;
};

/*! Returns true if the Entry exists in the filesytem, false otherwise. */
bool BEntry::Exists() const{
	return false;
};

/*! Gets a stat structure for the Entry */
status_t BEntry::GetStat(struct stat *st) const{
};

/*! Reinitializes the BEntry to the path or directory path combination,
	resolving symlinks if traverse is true */
status_t
BEntry::SetTo(const BDirectory *dir, const char *path, bool traverse = false){
};
				  
/*! Reinitializes the BEntry to the entry_ref, resolving symlinks if
	traverse is true */
status_t
BEntry::SetTo(const entry_ref *ref, bool traverse = false){
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
			// Open the directory
			StorageKit::Dir dir;
			fCStatus = StorageKit::open_dir(pathStr, dir);
			if (fCStatus == B_OK) {
				fCStatus = set(dir, leafStr, traverse);
				if (fCStatus != B_OK)
					StorageKit::close_dir(dir);		
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
	if (fDir != StorageKit::NullDir) {
		StorageKit::close_dir(fDir);
	}
	
	// Free our leaf name
	if (fLeaf != NULL) {
		delete [] fLeaf;
	}

	fDir = StorageKit::NullDir;
	fLeaf = NULL;
	fCStatus = B_NO_INIT;
};

/*! Gets an entry_ref structure from the BEntry */
status_t
BEntry::GetRef(entry_ref *ref) const {
};

/*! Gets the path for the BEntry */
status_t
BEntry::GetPath(BPath *path) const {
};

/*! Gets the parent of the BEntry */
status_t BEntry::GetParent(BEntry *entry) const {
};

/*! Gets the parent of the BEntry as a BDirectory */
status_t
BEntry::GetParent(BDirectory *dir) const {
};

/*! Gets the name of the */
status_t
BEntry::GetName(char *buffer) const {
};

/*! Renames the BEntry to path, replacing an existing entry if clobber is true. */
status_t
BEntry::Rename(const char *path, bool clobber = false) {
};

/*! Moves the BEntry to path or dir path combination, replacing an existing entry if clober is true. */
status_t
BEntry::MoveTo(BDirectory *dir, const char *path = NULL, bool clobber = false) {
};

/*! Removes the entry from the file system */
status_t
BEntry::Remove() {
};

/*! Equality operator */
bool
BEntry::operator==(const BEntry &item) const {
};

/*! Inequality operator */
bool
BEntry::operator!=(const BEntry &item) const {
};

/*! Reinitialises the BEntry to be a copy of the arguement */
BEntry&
BEntry::operator=(const BEntry &item) {
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
BEntry::set(StorageKit::Dir dir, const char *leaf, bool traverse) {

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
		for (	entry = StorageKit::read_dir(dir);
				entry != NULL;
				entry = StorageKit::read_dir(dir)	) {

			if (strcmp(entry->d_name, leaf) == 0) {
			
				// We've found the entry in the directory, now we convert
				// it to an absolute pathname and find out if it's a symbolic
				// link. If so, we traverse it.
				char path[B_PATH_NAME_LENGTH+1];
				status_t result = StorageKit::entry_ref_to_path(entry->d_pdev,
					entry->d_pino, entry->d_name, path, B_PATH_NAME_LENGTH+1);
					
				if (result == B_OK) {
//					printf("Entry == '%s'\n", path);	// Prints out the full path of this entry
				
					
					// Attempt to read the name of the file the link points to.
					// If this fails, the entry is not a symlink.
					char target[B_PATH_NAME_LENGTH+1];
					ssize_t len = StorageKit::read_link(path, target, B_PATH_NAME_LENGTH+1);
					
					if (len >= 0) {
					
						// target is now the pathname of the entry the link points
						// to, so we now recursively SetTo() ourselves to this
						// new entry. If target is absolute, we just set ourselves
						// to the target, otherwise we need to convert the relative
						// path to an absolute path and then call SetTo()
						if (target[0] == '/') {
						
							// Absolute path
							status_t result = SetTo(target, traverse);
							if (result == B_OK) {
								StorageKit::close_dir(dir);
									// We're responsible for dir when successful
							}
							
							return result;
							
						} else {
						
							// Relative path
							status_t result = StorageKit::entry_ref_to_path(entry->d_pdev,
								entry->d_pino, target, path, B_PATH_NAME_LENGTH+1);
							
							if (result == B_OK) {
								result = SetTo(path, traverse);
							}
							
							if (result == B_OK) {
								StorageKit::close_dir(dir);
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
	
	fDir = dir;
	SetLeaf(leaf);
	return B_OK;
};

/*! Possibly called by Unset() */
status_t
BEntry::clear(){
};

/*! Handles string allocation, deallocation, and copying for our leaf name. */
status_t
BEntry::SetLeaf(const char *leaf) {
	if (leaf == NULL)
		return B_BAD_VALUE;	
	
	if (fLeaf != NULL) {
		delete [] fLeaf;
	}
	
	fLeaf = new char[strlen(leaf)+1];
	if (fLeaf == NULL)
		return B_NO_MEMORY;
		
	strcpy(fLeaf, leaf);
	
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
		path = new char[1];
		path[0] = 0;	
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

