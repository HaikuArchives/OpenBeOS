//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: SymLink.cpp
//---------------------------------------------------------------------
#include <SymLink.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

enum {
	NOT_IMPLEMENTED	= B_ERROR,
};

// constructor
//! Creates an uninitialized BSymLink object.
/*!	\todo Implement! */
BSymLink::BSymLink()
		: BNode()
{
}

// copy constructor
//! Creates a copy of the supplied BSymLink.
/*!	\param link the BSymLink object to be copied
	\todo Implement!
*/
BSymLink::BSymLink(const BSymLink &link)
		: BNode()
{
	*this = link;
}

// constructor
/*! \brief Creates a BSymLink and initializes it to the symbolic link referred
	to by the supplied entry_ref.
	\param ref the entry_ref referring to the symbolic link
	\todo Implement!
*/
BSymLink::BSymLink(const entry_ref *ref)
		: BNode()
{
}

// constructor
/*! \brief Creates a BSymLink and initializes it to the symbolic link referred
	to by the supplied BEntry.
	\param entry the BEntry referring to the symbolic link
	\todo Implement!
*/
BSymLink::BSymLink(const BEntry *entry)
		: BNode()
{
}

// constructor
/*! \brief Creates a BSymLink and initializes it to the symbolic link referred
	to by the supplied path name.
	\param path the symbolic link's path name 
	\todo Implement!
*/
BSymLink::BSymLink(const char *path)
		: BNode()
{
}

// constructor
/*! \brief Creates a BSymLink and initializes it to the symbolic link referred
	to by the supplied path name relative to the specified BDirectory.
	\param dir the BDirectory, relative to which the symbolic link's path name
		   is given
	\param path the symbolic link's path name relative to \a dir
	\todo Implement!
*/
BSymLink::BSymLink(const BDirectory *dir, const char *path)
		: BNode()
{
}

// destructor
//! Frees all allocated resources.
/*! If the BSymLink is properly initialized, the symbolic link's file
	descriptor is closed.
	\todo Implement!
*/
BSymLink::~BSymLink()
{
}

// ReadLink
//! Reads the contents of the symbolic link into a buffer.
/*!	\param buf the buffer
	\param size the size of the buffer
	\return
	- the number of bytes written into the buffer
	- \c B_BAD_VALUE: \c NULL \a buf or the object doesn't refer to a symbolic
	  link.
	- \c B_FILE_ERROR: The object is not initialized.
	- some other error code
	\todo Implement!
*/
ssize_t
BSymLink::ReadLink(char *buf, size_t size)
{
	return NOT_IMPLEMENTED;
}

// MakeLinkedPath
/*!	\brief Combines a directory path and the contents of this symbolic link to
	an absolute path.
	\param dirPath the path name of the directory
	\param path the BPath object to be set to the resulting path name
	\return
	- \c the length of the resulting path name,
	- \c B_BAD_VALUE: \c NULL \a dirPath or \a path or the object doesn't
		 refer to a symbolic link.
	- \c B_FILE_ERROR: The object is not initialized.
	- \c B_NAME_TOO_LONG: The resulting path name is too long.
	- some other error code
	\todo Implement!
*/
ssize_t
BSymLink::MakeLinkedPath(const char *dirPath, BPath *path)
{
	return NOT_IMPLEMENTED;
}

// MakeLinkedPath
/*!	\brief Combines a directory path and the contents of this symbolic link to
	an absolute path.
	\param dir the BDirectory referring to the directory
	\param path the BPath object to be set to the resulting path name
	\return
	- \c the length of the resulting path name,
	- \c B_BAD_VALUE: \c NULL \a dir or \a path or the object doesn't
		 refer to a symbolic link.
	- \c B_FILE_ERROR: The object is not initialized.
	- \c B_NAME_TOO_LONG: The resulting path name is too long.
	- some other error code
	\todo Implement!
*/
ssize_t
BSymLink::MakeLinkedPath(const BDirectory *dir, BPath *path)
{
	return NOT_IMPLEMENTED;
}

// IsAbsolute
//!	Returns whether this BSymLink refers to an absolute link.
/*!	/return
	- \c true, if the object is properly initialized and the symbolic link it
	  refers to is an absolute link,
	- \c false, otherwise.
	\todo Implement!
*/
bool
BSymLink::IsAbsolute()
{
	return false;	// not implemented
}

// =
//! Assigns another BSymLink to this BSymLink.
/*!	If the other BSymLink is uninitialized, this one will be too. Otherwise
	it will refer to the same symbolic link, unless an error occurs.
	\param link the original BSymLink
	\return a reference to this BSymLink
*/
/*BSymLink &
BSymLink::operator=(const BSymLink &link)
{
	Unset();
	if (link.InitCheck() == B_OK) {
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
*/

void BSymLink::_ReservedSymLink1() {}
void BSymLink::_ReservedSymLink2() {}
void BSymLink::_ReservedSymLink3() {}
void BSymLink::_ReservedSymLink4() {}
void BSymLink::_ReservedSymLink5() {}
void BSymLink::_ReservedSymLink6() {}


#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif
