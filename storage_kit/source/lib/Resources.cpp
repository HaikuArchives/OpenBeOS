//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//---------------------------------------------------------------------
/*!
	\file Resources.cpp
	BResources implementation.
*/
#include <Resources.h>

enum {
	NOT_IMPLEMENTED	= B_ERROR,
};

// constructor
/*!	\brief Creates an unitialized BResources object.
*/
BResources::BResources()
{
}

// constructor
/*!	\brief Creates a BResources object that represents the resources of the
	supplied file.
	If the \a clobber argument is \c true, the data of the file are erased
	and it is turned into an empty resource file. Otherwise \a file
	must refer either to a resource file or to an executable (ELF or PEF
	binary). If the file has been opened \c B_READ_ONLY, only read access
	to its resources is possible.
	The BResources object makes a copy of \a file, that is the caller remains
	owner of the BFile object.
	\param file the file
	\param clobber if \c true, the \a file is truncated to size 0
*/
BResources::BResources(BFile *file, bool clobber)
{
}

// destructor
/*!	\brief Frees all resources associated with this object
	Calls Sync() before doing so to make sure that the changes are written
	back to the file.
*/
BResources::~BResources()
{
}

// SetTo
/*!	\brief Re-initialized the BResources object to represent the resources of
	the supplied file.
	What happens, if \a clobber is \c true, depends on the type of the file.
	If the file is capable of containing resources, that is, is a resource
	file or an executable (ELF or PEF), its resources are removed. Otherwise
	the file's data are erased and it is turned into an empty resource file.
	If \a clobber is \c false, \a file must refer to a file that is capable
	of containing resources.
	If the file has been opened \c B_READ_ONLY, only read access
	to its resources is possible.
	The BResources object makes a copy of \a file, that is the caller remains
	owner of the BFile object.
	\param file the file
	\param clobber if \c true, the \a file is truncated to size 0
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: \c NULL or uninitialized \a file.
	- \c B_ERROR: Failed to initialize the object (for whatever reason).
*/
status_t
BResources::SetTo(BFile *file, bool clobber)
{
	return NOT_IMPLEMENTED;
}

// Unset
/*!	\brief Returns the BResources object to an uninitialized state.
	If the object represented resources that had been modified, the data are
	written back to the file.
	\note This method extends the BeOS R5 API.
*/
void
BResources::Unset()
{
}

// InitCheck
/*!	\brief Returns the result of the last initialization (via constructor or
	SetTo()).
	\return The result of the last initialization (\see SetTo()).
	\note This method extends the BeOS R5 API.
*/
status_t
BResources::InitCheck() const
{
	return NOT_IMPLEMENTED;
}

// File
/*!	\brief Returns a reference to the BResources' BFile object.
	\return a reference to the object's BFile.
*/
const BFile &
BResources::File() const
{
	return fFile;
}

// LoadResource
/*!	\brief Loads a resource identified by type and ID into memory.
	A resource is loaded into memory only once. A second call with the same
	parameters will result in the same pointer. The BResources object is the
	owner of the allocated memory and the pointer to it will be valid until
	the object is destroyed or the resource is removed or modified.
	\param type the type of the resource to be loaded
	\param id the ID of the resource to be loaded
	\param outSize a pointer to a variable into which the size of the resource
		   shall be written
	\return A pointer to the resource data, if everything went fine, or
			\c NULL, if the file does not have a resource that matchs the
			parameters or an error occured.
*/
const void *
BResources::LoadResource(type_code type, int32 id, size_t *outSize)
{
	return NULL;	// not implemented
}

// LoadResource
/*!	\brief Loads a resource identified by type and name into memory.
	A resource is loaded into memory only once. A second call with the same
	parameters will result in the same pointer. The BResources object is the
	owner of the allocated memory and the pointer to it will be valid until
	the object is destroyed or the resource is removed or modified.
	\param type the type of the resource to be loaded
	\param name the name of the resource to be loaded
	\param outSize a pointer to a variable into which the size of the resource
		   shall be written
	\return A pointer to the resource data, if everything went fine, or
			\c NULL, if the file does not have a resource that matches the
			parameters or an error occured.
	\note Since a type and name pair may not identify a resource uniquely,
		  this method always returns the first resource that matches the
		  parameters, that is the one with the least index.
*/
const void *
BResources::LoadResource(type_code type, const char *name, size_t *outSize)
{
	return NULL;	// not implemented
}

// PreloadResourceType
/*!	\brief Loads all resources of a certain type into memory.
	For performance reasons it might be useful to do that. If \a type is
	0, all resources are loaded.
	\param type of the resources to be loaded
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_FILE: The resource map is empty???
	- The negative of the number of errors occured.
*/
status_t
BResources::PreloadResourceType(type_code type)
{
	return NOT_IMPLEMENTED;
}

// Sync
/*!	\brief Writes all changes to the resources to the file.
	Since AddResource() and RemoveResource() may change the resources only in
	memory, this method can be used to make sure, that all changes are
	actually written to the file.
	The BResources object's destructor calls Sync() before cleaning up.
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_FILE: The resource map is empty???
	- \c B_NOT_ALLOWED: The file is opened read only.
	- \c B_FILE_ERROR: A file error occured.
	- \c B_IO_ERROR: An error occured while writing the resources.
	\note When a resource is written to the file, its data are converted
		  to the endianess of the file, and when reading a resource, the
		  data are converted to the host's endianess. This does of course
		  only work for known types, i.e. those that swap_data() is able to
		  cope with.
*/
status_t
BResources::Sync()
{
	return NOT_IMPLEMENTED;
}

// MergeFrom
/*!	\brief Adds the resources of the supplied file to this file's resources.
	\param fromFile the file whose resources shall be copied
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: \c NULL \a fromFile.
	- \c B_BAD_FILE: The resource map is empty???
	- \c B_FILE_ERROR: A file error occured.
	- \c B_IO_ERROR: An error occured while writing the resources.
*/
status_t
BResources::MergeFrom(BFile *fromFile)
{
	return NOT_IMPLEMENTED;
}

// WriteTo
/*!	\brief Writes the resources to a new file.
	The resources formerly contained in the target file (if any) are erased.
	When the method returns, the BResources object refers to the new file.
	\param file the file the resources shall be written to.
	\return
	- \c B_OK: Everything went fine.
	- a specific error code.
	\note If the resources have been modified, but not Sync()ed, the old file
		  remains unmodified.
*/
status_t
BResources::WriteTo(BFile *file)
{
	return NOT_IMPLEMENTED;
}

// AddResource
/*!	\brief Adds a new resource to the file.
	If a resource with the same type and ID does already exist, it is
	replaced. The caller keeps the ownership of the supplied chunk of memory
	containing the resource data.
	Supplying an empty name (\c "") is equivalent to supplying a \c NULL name.
	\param type the type of the resource
	\param id the ID of the resource
	\param data the resource data
	\param length the size of the data in bytes
	\param name the name of the resource (may be \c NULL)
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: \c NULL \a data
	- \c B_NOT_ALLOWED: The file is opened read only.
	- \c B_FILE_ERROR: A file error occured.
	- \c B_NO_MEMORY: Not enough memory for that operation.
*/
status_t
BResources::AddResource(type_code type, int32 id, const void *data,
						size_t length, const char *name)
{
	return NOT_IMPLEMENTED;
}

// HasResource
/*!	\brief Returns whether the file contains a resource with a certain
	type and ID.
	\param type the resource type
	\param id the ID of the resource
	\return \c true, if the file contains a matching resource, \false otherwise
*/
bool
BResources::HasResource(type_code type, int32 id)
{
	return false;	// not implemented
}

// HasResource
/*!	\brief Returns whether the file contains a resource with a certain
	type and name.
	\param type the resource type
	\param name the name of the resource
	\return \c true, if the file contains a matching resource, \false otherwise
*/
bool
BResources::HasResource(type_code type, const char *name)
{
	return false;	// not implemented
}

// GetResourceInfo
/*!	\brief Returns information about a resource identified by an index.
	\param byIndex the index of the resource in the file
	\param typeFound a pointer to a variable the type of the found resource
		   shall be written into
	\param idFound a pointer to a variable the ID of the found resource
		   shall be written into
	\param nameFound a pointer to a variable the name pointer of the found
		   resource shall be written into
	\param lengthFound a pointer to a variable the data size of the found
		   resource shall be written into
	\return \c true, if a matching resource could be found, false otherwise
*/
bool
BResources::GetResourceInfo(int32 byIndex, type_code *typeFound,
							int32 *idFound, const char **nameFound,
							size_t *lengthFound)
{
	return false;	// not implemented
}

// GetResourceInfo
/*!	\brief Returns information about a resource identified by a type and an
	index.
	\param byType the resource type
	\param andIndex the index into a array of resources of type \a byType
	\param idFound a pointer to a variable the ID of the found resource
		   shall be written into
	\param nameFound a pointer to a variable the name pointer of the found
		   resource shall be written into
	\param lengthFound a pointer to a variable the data size of the found
		   resource shall be written into
	\return \c true, if a matching resource could be found, false otherwise
*/
bool
BResources::GetResourceInfo(type_code byType, int32 andIndex, int32 *idFound,
							const char **nameFound, size_t *lengthFound)
{
	return false;	// not implemented
}

// GetResourceInfo
/*!	\brief Returns information about a resource identified by a type and an ID.
	\param byType the resource type
	\param andID the resource ID
	\param nameFound a pointer to a variable the name pointer of the found
		   resource shall be written into
	\param lengthFound a pointer to a variable the data size of the found
		   resource shall be written into
	\return \c true, if a matching resource could be found, false otherwise
*/
bool
BResources::GetResourceInfo(type_code byType, int32 andID,
							const char **nameFound, size_t *lengthFound)
{
	return false;	// not implemented
}

// GetResourceInfo
/*!	\brief Returns information about a resource identified by a type and a
	name.
	\param byType the resource type
	\param andName the resource name
	\param idFound a pointer to a variable the ID of the found resource
		   shall be written into
	\param lengthFound a pointer to a variable the data size of the found
		   resource shall be written into
	\return \c true, if a matching resource could be found, false otherwise
*/
bool
BResources::GetResourceInfo(type_code byType, const char *andName,
							int32 *idFound, size_t *lengthFound)
{
	return false;	// not implemented
}

// GetResourceInfo
/*!	\brief Returns information about a resource identified by a data pointer.
	\param byPointer the pointer to the resource data (formely returned by
		   LoadResource())
	\param typeFound a pointer to a variable the type of the found resource
		   shall be written into
	\param idFound a pointer to a variable the ID of the found resource
		   shall be written into
	\param lengthFound a pointer to a variable the data size of the found
		   resource shall be written into
	\param nameFound a pointer to a variable the name pointer of the found
		   resource shall be written into
	\return \c true, if a matching resource could be found, false otherwise
*/
bool
BResources::GetResourceInfo(const void *byPointer, type_code *typeFound,
							int32 *idFound, size_t *lengthFound,
							const char **nameFound)
{
	return false;	// not implemented
}

// RemoveResource
/*!	\brief Removes a resource identified by its data pointer.
	\param resource the pointer to the resource data (formely returned by
		   LoadResource())
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: \c NULL or invalid (not pointing to any resource data of
	  this file) \a resource.
	- \c B_NOT_ALLOWED: The file is opened read only.
	- \c B_FILE_ERROR: A file error occured.
	- \c B_ERROR: An error occured while removing the resource.
*/
status_t
BResources::RemoveResource(const void *resource)
{
	return NOT_IMPLEMENTED;
}

// RemoveResource
/*!	\brief Removes a resource identified by type and ID.
	\param type the type of the resource
	\param id the ID of the resource
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: No such resource.
	- \c B_NOT_ALLOWED: The file is opened read only.
	- \c B_FILE_ERROR: A file error occured.
	- \c B_ERROR: An error occured while removing the resource.
*/
status_t
BResources::RemoveResource(type_code type, int32 id)
{
	return NOT_IMPLEMENTED;
}


// deprecated

// WriteResource
/*!	\brief Writes data into an existing resource.
	If writing the data would exceed the bounds of the resource, it is
	enlarged respectively. If \a offset is past the end of the resource,
	padding with unspecified data is inserted.
	\param type the type of the resource
	\param id the ID of the resource
	\param data the data to be written
	\param offset the byte offset relative to the beginning of the resource at
		   which the data shall be written
	\param length the size of the data to be written
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: \a type and \a id do not identify an existing resource or
	  \c NULL \a data.
	- \c B_NO_MEMORY: Not enough memory for this operation.
	- other error codes.
	\deprecated Always use AddResource().
*/
status_t
BResources::WriteResource(type_code type, int32 id, const void *data,
						  off_t offset, size_t length)
{
	return NOT_IMPLEMENTED;
}

// ReadResource
/*!	\brief Reads data from an existing resource.
	If more data than existing are requested, this method does not fail. It
	will then read only the existing data. As a consequence an offset past
	the end of the resource will not cause the method to fail, but no data
	will be read at all.
	\param type the type of the resource
	\param id the ID of the resource
	\param data a pointer to a buffer into which the data shall be read
	\param offset the byte offset relative to the beginning of the resource
		   from which the data shall be read
	\param length the size of the data to be read
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: \a type and \a id do not identify an existing resource or
	  \c NULL \a data.
	- \c B_NO_MEMORY: Not enough memory for this operation.
	- other error codes.
	\deprecated Use LoadResource() only.
*/
status_t
BResources::ReadResource(type_code type, int32 id, void *data, off_t offset,
						 size_t length)
{
	return NOT_IMPLEMENTED;
}

// FindResource
/*!	\brief Finds a resource by type and ID and returns a copy of its data.
	The caller is responsible for free()ing the returned memory.
	\param type the type of the resource
	\param id the ID of the resource
	\param lengthFound a pointer to a variable into which the size of the
		   resource data shall be written
	\return
	- a pointer to the resource data, if everything went fine,
	- \c NULL, if an error occured.
	\deprecated Use LoadResource().
*/
void *
BResources::FindResource(type_code type, int32 id, size_t *lengthFound)
{
	return NULL;	// not implemented
}

// FindResource
/*!	\brief Finds a resource by type and name and returns a copy of its data.
	The caller is responsible for free()ing the returned memory.
	\param type the type of the resource
	\param name the name of the resource
	\param lengthFound a pointer to a variable into which the size of the
		   resource data shall be written
	\return
	- a pointer to the resource data, if everything went fine,
	- \c NULL, if an error occured.
	\deprecated Use LoadResource().
*/
void *
BResources::FindResource(type_code type, const char *name, size_t *lengthFound)
{
	return NULL;	// not implemented
}


// FBC
void BResources::_ReservedResources1() {}
void BResources::_ReservedResources2() {}
void BResources::_ReservedResources3() {}
void BResources::_ReservedResources4() {}
void BResources::_ReservedResources5() {}
void BResources::_ReservedResources6() {}
void BResources::_ReservedResources7() {}
void BResources::_ReservedResources8() {}

