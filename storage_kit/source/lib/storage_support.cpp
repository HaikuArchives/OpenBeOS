//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: storage_support.cpp
//  Description: This file contains the implementation of miscellaneous
//  Storage Kit internal support functionality.
//----------------------------------------------------------------------

#include "SupportDefs.h"
#include "storage_support.h"

namespace StorageKit {

// parse_path
static
void
parse_path(const char *fullPath, int &leafStart, int &leafEnd, int &pathEnd)
{
	if (fullPath == NULL)
		return;

	enum PathParserState { PPS_START, PPS_LEAF, PPS_END } state = PPS_START;

	int len = strlen(fullPath);
	
	leafStart = -1;
	leafEnd = -1;
	pathEnd = -2;
	
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
}

/*!	The caller is responsible for deleting the returned directory path name
	and the leaf name.
	\param fullPath the path name to be split
	\param path a variable the directory path name pointer shall
		   be written into, may be NULL
	\param leaf a variable the leaf name pointer shall be
		   written into, may be NULL
*/
bool
split_path(const char *fullPath, char *&path, char *&leaf)
{
	return split_path(fullPath, &path, &leaf);
}

/*!	The caller is responsible for deleting the returned directory path name
	and the leaf name.
	\param fullPath the path name to be split
	\param path a pointer to a variable the directory path name pointer shall
		   be written into, may be NULL
	\param leaf a pointer to a variable the leaf name pointer shall be
		   written into, may be NULL
*/
bool
split_path(const char *fullPath, char **path, char **leaf)
{
	if (path)
		*path = NULL;
	if (leaf)
		*leaf = NULL;

	if (fullPath == NULL)
		return false;

	int leafStart, leafEnd, pathEnd, len;
	parse_path(fullPath, leafStart, leafEnd, pathEnd);

	// Tidy up/handle special cases
	if (leafEnd == -1) {
	
		// Handle special cases 
		if (fullPath[0] == '/') {
			// Handle "/"
			if (path) {
				*path = new char[2];
				(*path)[0] = '/';
				(*path)[1] = 0;
			}
			if (leaf) {
				*leaf = new char[2];
				(*leaf)[0] = '.';
				(*leaf)[1] = 0;
			}
			return true;	
		} else if (fullPath[0] == 0) {
			// Handle "", which we'll treat as "./"
			if (path) {
				*path = new char[1];
				(*path)[0] = 0;
			}
			if (leaf) {
				*leaf = new char[2];
				(*leaf)[0] = '.';
				(*leaf)[1] = 0;
			}
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
	if (path) {
		if (pathEnd == -2) {
			// empty path
			*path = new char[2];
			(*path)[0] = '.';
			(*path)[1] = 0;
		} else {
			// non-empty path
			len = pathEnd + 1;
			*path = new char[len+1];
			memcpy(*path, fullPath, len);
			(*path)[len] = 0;
		}
	}
	if (leaf) {
		len = leafEnd - leafStart + 1;
		*leaf = new char[len+1];
		memcpy(*leaf, fullPath + leafStart, len);
		(*leaf)[len] = 0;
	}
	
	return true;
}

} // namespace StorageKit
