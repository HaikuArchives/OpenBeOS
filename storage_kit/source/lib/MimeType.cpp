//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//---------------------------------------------------------------------
/*!
	\file MimeType.cpp
	BMimeType implementation.
*/
#include <MimeType.h>

enum {
	NOT_IMPLEMENTED	= B_ERROR,
};


const char *B_PEF_APP_MIME_TYPE		= "application/x-be-executable";
const char *B_PE_APP_MIME_TYPE		= "application/x-vnd.be-peexecutable";
const char *B_ELF_APP_MIME_TYPE		= "application/x-vnd.be-elfexecutable";
const char *B_RESOURCE_MIME_TYPE	= "application/x-be-resource";
const char *B_FILE_MIME_TYPE		= "application/octet-stream";
// Might be defined platform depended, but ELF will certainly be the common
// format for all platforms anyway.
const char *B_APP_MIME_TYPE			= B_ELF_APP_MIME_TYPE;

// constructor
BMimeType::BMimeType()
{
}

// constructor
BMimeType::BMimeType(const char *MIME_type)
{
}

// destructor
BMimeType::~BMimeType()
{
}

// SetTo
status_t
BMimeType::SetTo(const char *MIME_type)
{
	return NOT_IMPLEMENTED;
}

// Unset
void
BMimeType::Unset()
{
}

// InitCheck
status_t
BMimeType::InitCheck() const
{
	return NOT_IMPLEMENTED;
}

// Type
const char *
BMimeType::Type() const
{
	return NULL;	// not implemented
}

// IsValid
bool
BMimeType::IsValid() const
{
	return false;	// not implemented
}

// IsSupertypeOnly
bool
BMimeType::IsSupertypeOnly() const
{
	return false;	// not implemented
}

// IsInstalled
bool
BMimeType::IsInstalled() const
{
	return false;	// not implemented
}

// GetSupertype
status_t
BMimeType::GetSupertype(BMimeType *super_type) const
{
	return NOT_IMPLEMENTED;
}

// ==
bool
BMimeType::operator==(const BMimeType &type) const
{
	return false;	// not implemented
}

// ==
bool
BMimeType::operator==(const char *type) const
{
	return false;	// not implemented
}

// Contains
bool
BMimeType::Contains(const BMimeType *type) const
{
	return false;	// not implemented
}

// Install
status_t
BMimeType::Install()
{
	return NOT_IMPLEMENTED;
}

// Delete
status_t
BMimeType::Delete()
{
	return NOT_IMPLEMENTED;
}

// GetIcon
status_t
BMimeType::GetIcon(BBitmap *icon, icon_size) const
{
	return NOT_IMPLEMENTED;
}

// GetPreferredApp
status_t
BMimeType::GetPreferredApp(char *signature, app_verb verb) const
{
	return NOT_IMPLEMENTED;
}

// GetAttrInfo
status_t
BMimeType::GetAttrInfo(BMessage *info) const
{
	return NOT_IMPLEMENTED;
}

// GetFileExtensions
status_t
BMimeType::GetFileExtensions(BMessage *extensions) const
{
	return NOT_IMPLEMENTED;
}

//! Fetches the MIME type's short description from the MIME database
/*! The string pointed to by \c description must be long enough to
	hold the short description; a length of \c B_MIME_TYPE_LENGTH+1 is
	recommended.
	
	\param description Pointer to a pre-allocated string into which the long description is copied. If
	                   the function fails, the contents of the string are undefined.
	\return
	- B_OK: Success
	- B_ENTRY_NOT_FOUND: No short description exists for the given type
	- "error code": Failure
*/
status_t
BMimeType::GetShortDescription(char *description) const
{
	return NOT_IMPLEMENTED;
}

//! Fetches the MIME type's long description from the MIME database
/*! The string pointed to by \c description must be long enough to
	hold the long description; a length of \c B_MIME_TYPE_LENGTH+1 is
	recommended.

	\param description Pointer to a pre-allocated string into which the long description is copied. If
	                   the function fails, the contents of the string are undefined.
	\return
	- B_OK: Success
	- B_ENTRY_NOT_FOUND: No long description exists for the given type
	- "error code": Failure
*/
status_t
BMimeType::GetLongDescription(char *description) const
{
	return NOT_IMPLEMENTED;
}

// GetSupportingApps
status_t
BMimeType::GetSupportingApps(BMessage *signatures) const
{
	return NOT_IMPLEMENTED;
}

// SetIcon
status_t
BMimeType::SetIcon(const BBitmap *icon, icon_size)
{
	return NOT_IMPLEMENTED;
}

// SetPreferredApp
status_t
BMimeType::SetPreferredApp(const char *signature, app_verb verb)
{
	return NOT_IMPLEMENTED;
}

// SetAttrInfo
status_t
BMimeType::SetAttrInfo(const BMessage *info)
{
	return NOT_IMPLEMENTED;
}

// SetFileExtensions
status_t
BMimeType::SetFileExtensions(const BMessage *extensions)
{
	return NOT_IMPLEMENTED;
}

//! Sets the short description field for the MIME type
/*! The string pointed to by \c description must be of
	length less than or equal to \c B_MIME_TYPE_LENGTH characters.
	
	\note If the MIME type is not installed, it will first be installed, and then
	the short description will be set.

	\param description Pointer to a pre-allocated string containing the new short description
	\return
	- B_OK: Success
	- "error code": Failure
*/
status_t
BMimeType::SetShortDescription(const char *description)
{
	return NOT_IMPLEMENTED;
}

//! Sets the long description field for the MIME type
/*! The string pointed to by \c description must be of
	length less than or equal to \c B_MIME_TYPE_LENGTH characters.
	
	\note If the MIME type is not installed, it will first be installed, and then
	the long description will be set.

	\param description Pointer to a pre-allocated string containing the new long description
	\return
	- B_OK: Success
	- "error code": Failure
*/
status_t
BMimeType::SetLongDescription(const char *description)
{
	return NOT_IMPLEMENTED;
}

// GetInstalledSupertypes
status_t
BMimeType::GetInstalledSupertypes(BMessage *super_types)
{
	return NOT_IMPLEMENTED;
}

// GetInstalledTypes
status_t
BMimeType::GetInstalledTypes(BMessage *types)
{
	return NOT_IMPLEMENTED;
}

// GetInstalledTypes
status_t
BMimeType::GetInstalledTypes(const char *super_type, BMessage *subtypes)
{
	return NOT_IMPLEMENTED;
}

// GetWildcardApps
status_t
BMimeType::GetWildcardApps(BMessage *wild_ones)
{
	return NOT_IMPLEMENTED;
}

// IsValid
bool
BMimeType::IsValid(const char *string)
{
	return false;	// not implemented
}

// GetAppHint
status_t
BMimeType::GetAppHint(entry_ref *ref) const
{
	return NOT_IMPLEMENTED;
}

// SetAppHint
status_t
BMimeType::SetAppHint(const entry_ref *ref)
{
	return NOT_IMPLEMENTED;
}

// GetIconForType
status_t
BMimeType::GetIconForType(const char *type, BBitmap *icon, icon_size which) const
{
	return NOT_IMPLEMENTED;
}

// SetIconForType
status_t
BMimeType::SetIconForType(const char *type, const BBitmap *icon, icon_size which)
{
	return NOT_IMPLEMENTED;
}

// GetSnifferRule
status_t
BMimeType::GetSnifferRule(BString *result) const
{
	return NOT_IMPLEMENTED;
}

// SetSnifferRule
status_t
BMimeType::SetSnifferRule(const char *)
{
	return NOT_IMPLEMENTED;
}

// CheckSnifferRule
status_t
BMimeType::CheckSnifferRule(const char *rule, BString *parseError)
{
	return NOT_IMPLEMENTED;
}

// GuessMimeType
status_t
BMimeType::GuessMimeType(const entry_ref *file, BMimeType *result)
{
	return NOT_IMPLEMENTED;
}

// GuessMimeType
status_t
BMimeType::GuessMimeType(const void *buffer, int32 length, BMimeType *result)
{
	return NOT_IMPLEMENTED;
}

// GuessMimeType
status_t
BMimeType::GuessMimeType(const char *filename, BMimeType *result)
{
	return NOT_IMPLEMENTED;
}

// StartWatching
status_t
BMimeType::StartWatching(BMessenger target)
{
	return NOT_IMPLEMENTED;
}

// StopWatching
status_t
BMimeType::StopWatching(BMessenger target)
{
	return NOT_IMPLEMENTED;
}

// SetType
status_t
BMimeType::SetType(const char *MIME_type)
{
	return NOT_IMPLEMENTED;
}


void BMimeType::_ReservedMimeType1() {}
void BMimeType::_ReservedMimeType2() {}
void BMimeType::_ReservedMimeType3() {}

// =
BMimeType &
BMimeType::operator=(const BMimeType &)
{
	return *this;	// not implemented
}

// copy constructor
BMimeType::BMimeType(const BMimeType &)
{
}

