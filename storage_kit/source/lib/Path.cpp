//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Path.cpp
//  Description: An absolute pathname wrapper class
//---------------------------------------------------------------------
#include "Path.h"
#include "Error.h"

#ifdef USE_OPENBEOS_NAMESPACE
using namespace OpenBeOS;
#endif

#define B_REF_TYPE 0

bool BPath::MustNormalize(const char *path) {
	// Check for useless input
	if (path == NULL)
		throw new EBadPathInput("NULL path input");
	else if (path[0] == 0)
		throw new EBadPathInput("Empty path input");
		
	int len = strlen(path);	
		
	/* Look for anything in the string that forces us to normalize:
			+ No leading /
			+ A leading ./ or ../
			+ any occurence of /./ or /../ or //, or a trailing /. or /..
			+ a trailing /
	*/;
	if (path[0] != '/')
		return true;	//	"/*"
	else if (len >= 2 && path[0] == '.' && path[1] == '/')
		return true;	//	"./*"
	else if (len >= 3 && path[0] == '.' && path[1] == '.' && path[2] == '/')
		return true;	//	"../*"
	else if (len > 0 && path[len-1] == '/')
		return true;	// 	"*/"
	else {
		enum ParseState {
			NoMatch,
			InitialSlash,
			OneDot,
			TwoDots
		} state = NoMatch;
		
		for (int i = 0; path[i] != 0; i++) {
			switch (state) {
				case NoMatch:
					if (path[i] == '/')
						state = InitialSlash;
					break;
				
				case InitialSlash:
					if (path[i] == '/')
						return true;		// "*//*"
					else if (path[i] == '.')
						state = OneDot;
					else
						state = NoMatch;
					break;
				
				case OneDot:
					if (path[i] == '/')
						return true;		// "*/./*"
					else if (path[i] == '.')
						state = TwoDots;
					else
						state = NoMatch;
					break;
					
				case TwoDots:
					if (path[i] == '/')
						return true;		// "*/../*"
					else
						state = NoMatch;
					break;						
			}
		}
		// If we hit the end of the string while in either
		// of these two states, there was a trailing /. or /..
		if (state == OneDot || state == TwoDots)
			return true;
		else
			return false;			
	}
	
}

BPath::BPath() : fName(NULL) {
}
	
BPath::BPath(const char *dir, const char *leaf = NULL, bool normalize = false) {
}
	
BPath::BPath(const BDirectory *dir, const char *leaf, bool normalize = false) {
}

BPath::BPath(const BPath &path) {
}

BPath::BPath(const BEntry *entry) {
}

BPath::BPath(const entry_ref *ref) {
}

BPath::~BPath() {
}



status_t BPath::InitCheck() const {
}

status_t BPath::SetTo(const char *path, const char *leaf = NULL, bool normalize = false) {
}
	
status_t BPath::SetTo(const BDirectory *dir, const char *path, bool normalize = false) {
}
	
status_t BPath::SetTo(const BEntry *entry) {
}
	
status_t BPath::SetTo(const entry_ref *ref) {
}

status_t BPath::Append(const char *path, bool normalize = false) {
}
	
void BPath::Unset() {
}
	
const char *BPath::Path() const {
}
	
const char *BPath::Leaf() const {
}

status_t BPath::GetParent(BPath *) const {
}
	
bool BPath::operator==(const BPath &item) const {
}

bool BPath::operator==(const char *path) const {
}

bool BPath::operator!=(const BPath &item) const {
}

bool BPath::operator!=(const char *path) const {
}
	

BPath& BPath::operator=(const BPath &item) {
}

BPath& BPath::operator=(const char *path) {
}

	
bool BPath::IsFixedSize() const {
	return false;
}
	
type_code BPath::TypeCode() const {
	return B_REF_TYPE;
}
	
ssize_t BPath::FlattenedSize() const {
	return -1;
}
	
status_t BPath::Flatten(void *buffer, ssize_t size) const {
	return B_ERROR;
}
	
bool BPath::AllowsTypeCode(type_code code) const {
	return (code == B_REF_TYPE);
}
	
status_t BPath::Unflatten(type_code c, const void *buf, ssize_t size) {
	return B_ERROR;
}

void BPath::_WarPath1() {}
void BPath::_WarPath2() {}
void BPath::_WarPath3() {}
