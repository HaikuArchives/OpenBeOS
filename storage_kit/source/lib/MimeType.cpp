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
/*!	\brief Creates an uninitialized BMimeType object.
*/
BMimeType::BMimeType()
{
}

// constructor
/*!	\brief Creates a BMimeType object and initializes it to the supplied
	MIME type.
	The supplied string must specify a valid MIME type or supertype.
	\see SetTo() for further information.
	\param mimeType The MIME string.
*/
BMimeType::BMimeType(const char *mimeType)
{
}

// destructor
/*!	\brief Frees all resources associated with this object.
*/
BMimeType::~BMimeType()
{
}

// SetTo
/*!	\brief Initializes this object to the supplied MIME type.
	The supplied string must specify a valid MIME type or supertype.
	Valid MIME types are given by the following grammar:
	MIMEType	::= Supertype "/" [ Subtype ]
	Supertype	::= "application" | "audio" | "image" | "message"
					| "multipart" | "text" | "video"
	Subtype		::= MIMEChar MIMEChar*
	MIMEChar	::= any character except white spaces and '/', '<', '>', '@',
					',', ';', ':', '"', '(', ')', '[', ']', '?', '='

	Currently the supertype is not restricted to one of the seven types given,
	but can be an arbitrary string (obeying the same rule as the subtype).
	Nevertheless it is a very bad idea to use another supertype.
	The supplied MIME string is copied; the caller retains the ownership.
	\param mimeType The MIME string.
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: \c NULL or invalid \a mimeString.
	- \c B_NO_MEMORY: Insufficient memory to copy the MIME string.
*/
status_t
BMimeType::SetTo(const char *mimeType)
{
	return NOT_IMPLEMENTED;
}

// Unset
/*!	\brief Returns the object to an uninitialized state.
*/
void
BMimeType::Unset()
{
}

// InitCheck
/*!	Returns the result of the most recent constructor or SetTo() call.
	\return
	- \c B_OK: The object is properly initialized.
	- A specific error code otherwise.
*/
status_t
BMimeType::InitCheck() const
{
	return NOT_IMPLEMENTED;
}

// Type
/*!	\brief Returns the MIME string represented by this object.
	\return The MIME string, if the object is properly initialized, \c NULL
			otherwise.
*/
const char *
BMimeType::Type() const
{
	return NULL;	// not implemented
}

// IsValid
/*!	\brief Returns whether the object represents a valid MIME type.
	\see SetTo() for further information.
	\return \c true, if the object is properly initialized, \c false
			otherwise.
*/
bool
BMimeType::IsValid() const
{
	return false;	// not implemented
}

// IsSupertypeOnly
/*!	\brief Returns whether this objects represents a supertype.
	\return \c true, if the object is properly initialized and represents a
			supertype, \c false otherwise.
*/
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
/*!	\brief Returns the supertype of the MIME type represented by this object.
	The supplied object is initialized to this object's supertype. If this
	BMimeType is not properly initialized, the supplied object will be Unset().
	\param superType A pointer to the BMimeType object that shall be
		   initialized to this object's supertype.
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: \c NULL \a superType or this object is not initialized.
*/
status_t
BMimeType::GetSupertype(BMimeType *superType) const
{
	return NOT_IMPLEMENTED;
}

// ==
/*!	\brief Returns whether this and the supplied MIME type are equal.
	Two BMimeType objects are said to be equal, if they represent the same
	MIME string, ignoring case, or if both are not initialized.
	\param type The BMimeType to be compared with.
	\return \c true, if the objects are equal, \c false otherwise.
*/
bool
BMimeType::operator==(const BMimeType &type) const
{
	return false;	// not implemented
}

// ==
/*!	\brief Returns whether this and the supplied MIME type are equal.
	A BMimeType objects equals a MIME string, if its MIME string equals the
	latter one, ignoring case, or if it is uninitialized and the MIME string
	is \c NULL.
	\param type The MIME string to be compared with.
	\return \c true, if the MIME types are equal, \c false otherwise.
*/
bool
BMimeType::operator==(const char *type) const
{
	return false;	// not implemented
}

// Contains
/*!	\brief Returns whether this MIME type is a supertype of or equals the
	supplied one.
	\param type The MIME type.
	\return \c true, if this MIME type is a supertype of or equals the
			supplied one, \c false otherwise.
*/
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
//! Fetches the signature of the MIME type's preferred application from the MIME database
/*! The preferred app is the application that's used to access a file when, for example, the user
	double-clicks the file in a Tracker window. Unless the file identifies in its attributes a
	"custom" preferred app, Tracker will ask the file type database for the preferred app
	that's associated with the file's type.
	
	The string pointed to by \c signature must be long enough to
	hold the preferred applications signature; a length of \c B_MIME_TYPE_LENGTH+1 is
	recommended.
	
	\param signature Pointer to a pre-allocated string into which the signature of the preferred app is copied. If
	                   the function fails, the contents of the string are undefined.
	\param verb \c app_verb value that specifies the type of access for which you are requesting the preferred app.
	            Currently, the only supported app verb is \c B_OPEN.
	\return
	- B_OK: Success
	- B_ENTRY_NOT_FOUND: No preferred app exists for the given type and app_verb
	- "error code": Failure
*/
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

// GetShortDescription
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

// GetLongDescription
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
//! Sets the preferred application for the MIME type
/*! The preferred app is the application that's used to access a file when, for example, the user
	double-clicks the file in a Tracker window. Unless the file identifies in its attributes a
	"custom" preferred app, Tracker will ask the file type database for the preferred app
	that's associated with the file's type.
	
	The string pointed to by \c signature must be of
	length less than or equal to \c B_MIME_TYPE_LENGTH characters.
	
	\note If the MIME type is not installed, it will first be installed, and then
	the preferred app will be set.

	\param signature Pointer to a pre-allocated string containing the signature of the new preferred app.
	\param verb \c app_verb value that specifies the type of access for which you are setting the preferred app.
	            Currently, the only supported app verb is \c B_OPEN.
	\return
	- B_OK: Success
	- "error code": Failure
*/
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

// SetShortDescription
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

// SetLongDescription
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
/*!	\brief Returns whether the given string represents a valid MIME type.
	\see SetTo() for further information.
	\return \c true, if the given string represents a valid MIME type.
*/
bool
BMimeType::IsValid(const char *string)
{
	return false;	// not implemented
}

// GetAppHint
//! Fetches an \c entry_ref that serves as a hint as to where the MIME type's preferred application might live
/*! The app hint is a path that identifies the executable that should be used when launching an application
	that has this signature. For example, when Tracker needs to launch an app of type \c "application/YourAppHere",
	it asks the database for the application hint. This hint is converted to an \c entry_ref before it is passed
	to the caller. Of course, the path may not point to an application, or it might point to an application
	with the wrong signature (and so on); that's why this is merely a hint.

	The \c entry_ref pointed to by \c ref must be pre-allocated.

	\param ref Pointer to a pre-allocated \c entry_ref into which the location of the app hint is copied. If
	                   the function fails, the contents of the \c entry_ref are undefined.
	\return
	- B_OK: Success
	- B_ENTRY_NOT_FOUND: No app hint exists for the given type
	- "error code": Failure
*/
status_t
BMimeType::GetAppHint(entry_ref *ref) const
{
	return NOT_IMPLEMENTED;
}

// SetAppHint
//! Sets the app hint field for the MIME type
/*! The app hint is a path that identifies the executable that should be used when launching an application
	that has this signature. For example, when Tracker needs to launch an app of type \c "application/YourAppHere",
	it asks the database for the application hint. This hint is converted to an \c entry_ref before it is passed
	to the caller. Of course, the path may not point to an application, or it might point to an application
	with the wrong signature (and so on); that's why this is merely a hint.

	The \c entry_ref pointed to by \c ref must be pre-allocated. It must be a valid \c entry_ref (i.e. 
	<code>entry_ref(-1, -1, "some_file")</code> will trigger an error), but it need not point to an existing file, nor need
	it actually point to an application. That's not to say that it shouldn't; such an \c entry_ref would
	render the app hint useless.

	\param ref Pointer to a pre-allocated \c entry_ref containting the location of the new app hint
	\return
	- B_OK: Success
	- "error code": Failure
*/
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
/*!	\brief Starts monitoring the MIME database for a given target.
	Until StopWatching() is called for the target, an update message is sent
	to it, whenever the MIME database changes.
	\param target A BMessenger identifying the target for the update messages.
	\return
	- \c B_OK: Everything went fine.
	- An error code otherwise.
*/
status_t
BMimeType::StartWatching(BMessenger target)
{
	return NOT_IMPLEMENTED;
}

// StopWatching
/*!	\brief Stops monitoring the MIME database for a given target (previously
	started via StartWatching()).
	\param target A BMessenger identifying the target for the update messages.
	\return
	- \c B_OK: Everything went fine.
	- An error code otherwise.
*/
status_t
BMimeType::StopWatching(BMessenger target)
{
	return NOT_IMPLEMENTED;
}

// SetType
/*!	\brief Initializes this object to the supplied MIME type.
	\deprecated This method has the same semantics as SetTo().
				Use SetTo() instead.
*/
status_t
BMimeType::SetType(const char *mimeType)
{
	return NOT_IMPLEMENTED;
}


void BMimeType::_ReservedMimeType1() {}
void BMimeType::_ReservedMimeType2() {}
void BMimeType::_ReservedMimeType3() {}

// =
/*!	\brief Unimplemented assignment operator.
*/
BMimeType &
BMimeType::operator=(const BMimeType &)
{
	return *this;	// not implemented
}

// copy constructor
/*!	\brief Unimplemented copy constructor.
*/
BMimeType::BMimeType(const BMimeType &)
{
}

