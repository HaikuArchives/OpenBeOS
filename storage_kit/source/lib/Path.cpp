//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Path.cpp
//  Description: An absolute pathname wrapper class
//---------------------------------------------------------------------

/*! \file Path.h */
/*! \file Path.cpp */

/*! \class BPath
	\brief An absolute pathanem
	
	Provides a convenient means of managing pathnames.
	
	\see <a href="http://www.opensource.org/licenses/mit-license.html">MIT License</a>
	\author <a href="mailto:tylerdauwalder@users.sf.net">Tyler Dauwalder</a>
	\version 0.0.0
*/

/*! \class BPath::EBadInput
	\brief Internal exception class thrown by BPath::MustNormalize() when given invalid input.
*/

/*!
	\var char *BPath::fName
	\brief Pointer to the object's path name string.
*/

/*!
	\var status_t BPath::fCStatus
	\brief The object's initialization status.
*/


#include <Path.h>
#include <Entry.h>
#include <StorageDefs.h>
#include "kernel_interface.h"
#include "storage_support.h"

#ifdef USE_OPENBEOS_NAMESPACE
using namespace OpenBeOS;
#endif

#define B_REF_TYPE 0

//! Creates an uninitialized BPath object. 
BPath::BPath() : fCStatus(B_NO_INIT), fName(NULL) {
}
	
/*! \brief Creates a BPath object and initializes it to the specified path or
	path and filename combination.

	\param dir The base component of the pathname. May be absolute or relative. If relative,
				it is reckoned off the current working directory.
	\param leaf The (optional) leaf component of the pathname. Must be relative. The value of
				leaf is concatenated to the end of dir (a "/" will be added as a separator, if
				necessary).
				
				
*/
BPath::BPath(const char *dir, const char *leaf = NULL, bool normalize = false) :
fCStatus(B_NO_INIT), fName(NULL)
{
	SetTo(dir, leaf, normalize);
}
	
BPath::BPath(const BDirectory *dir, const char *leaf, bool normalize = false) :
fCStatus(B_NO_INIT), fName(NULL)
{
	SetTo(dir, leaf, normalize);
}

BPath::BPath(const BPath &path) : fCStatus(B_NO_INIT), fName(NULL) {
	SetTo(path.Path());
}

BPath::BPath(const BEntry *entry) : fCStatus(B_NO_INIT), fName(NULL) {
	SetTo(entry);
}

BPath::BPath(const entry_ref *ref) : fCStatus(B_NO_INIT), fName(NULL) {
}

BPath::~BPath() {
	Unset();
}

status_t BPath::InitCheck() const {
	return fCStatus;
}

status_t BPath::SetTo(const char *path, const char *leaf = NULL, bool normalize = false) {
	Unset();

	if (path == NULL)
		return B_BAD_VALUE;
		
	// Deduce whether we'll be normalizing or not
	try {		
		normalize = normalize || MustNormalize(path);
	} catch (BPath::EBadInput) {
		return B_BAD_VALUE;
	}
	
	if (normalize) {
		
		// I'll deal with this later :-)
		return B_FILE_ERROR;
	} else {
		if (fName != NULL)
			delete [] fName;
			
		fName = new char[strlen(path)+1];
		if (fName == NULL)
			return B_NO_MEMORY;
		strcpy(fName, path);
	}
	
	fCStatus = B_OK;
	return B_OK;
}
	
status_t BPath::SetTo(const BDirectory *dir, const char *path, bool normalize = false) {
	//! @todo Implement this once we have a BDirectory implementation
	Unset();
	return B_FILE_ERROR;
}
	
status_t BPath::SetTo(const BEntry *entry) {
	Unset();
	if (entry == NULL) 
		return B_BAD_VALUE;

	// Convert the Entry to an entry ref
	entry_ref ref;
	status_t result = entry->GetRef(&ref);

	return (result == B_OK) ? SetTo(&ref) : result ;
}
	
status_t BPath::SetTo(const entry_ref *ref) {
	Unset();
	if (ref == NULL) 
		return B_BAD_VALUE;
		
	char path[B_PATH_NAME_LENGTH+1];
	status_t result = StorageKit::entry_ref_to_path(ref, path, B_PATH_NAME_LENGTH+1);
	return (result == B_OK) ? SetTo(path) : result ;
}

/*! \brief Appends the given (relative) path to the end of the current path.
	This call fails if the path is absolute or the object to which you're appending is uninitialized.

	\param path Relative pathname to append to current path
	\param normalize Boolean flag used to force normalization; normalization may occur even if false (see \ref MustNormalize)
*/
status_t BPath::Append(const char *path, bool normalize = false) {
	if (path == NULL || StorageKit::is_absolute_path(path))
		return B_BAD_VALUE;
	else
		return SetTo(Path(), path);
}
	
void BPath::Unset() {
	if (fCStatus == B_OK) {
		delete [] fName;
		fName = NULL;
	}
	fCStatus = B_NO_INIT;
}
	
const char *BPath::Path() const {
	return fName;
}
	
const char *BPath::Leaf() const {
}

status_t BPath::GetParent(BPath *) const {
}
	
bool BPath::operator==(const BPath &item) const {
	return *this == item.Path();
}

bool BPath::operator==(const char *path) const {
//	printf("==() fName == '%s'\n", fName);
//	printf("==() path  == '%s'\n", path);
	return (strcmp(fName, path) == 0);
}

bool BPath::operator!=(const BPath &item) const {
	return !(*this == item.Path());
}

bool BPath::operator!=(const char *path) const {
	return !(*this == path);
}
	

BPath& BPath::operator=(const BPath &item) {
	if (this != &item)
		*this = item.fName;
	return *this;
}

BPath& BPath::operator=(const char *path) {
	if (path == NULL) {
		Unset();
	} else {
		SetTo(path);
	}
	return *this;
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

/*! \brief Checks a path to see if normalization is required.

	The following items require normalization:
		- Relative pathnames (after concatenation; e.g. "boot/ltj")	
		- The presence of "." or ".." ("/boot/ltj/../ltj/./gwar")
		- Redundant slashes ("/boot//ltj")
		- A trailing slash ("/boot/ltj/")
	
	\return
		- \c true: \a path requires normalization
		- \c false: \a path does not require normalization

	\exception BPath::EBadInput :  \a path is \c NULL or an empty string.

 */

bool BPath::MustNormalize(const char *path) {
	// Check for useless input
	if (path == NULL || path[0] == 0)
		throw BPath::EBadInput();
		
	int len = strlen(path);	
		
	/* Look for anything in the string that forces us to normalize:
			+ No leading /
			+ A leading ./ or ../
			+ any occurence of /./ or /../ or //, or a trailing /. or /..
			+ a trailing /
	*/;
	if (path[0] != '/')
		return true;	//	"/*"
	else if (len == 1)
		return false;	//	"/"
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

