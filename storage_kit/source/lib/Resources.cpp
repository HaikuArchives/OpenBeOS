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
BResources::BResources()
{
}

// constructor
BResources::BResources(BFile *file, bool clobber)
{
}

// destructor
BResources::~BResources()
{
}

// SetTo
status_t
BResources::SetTo(BFile *file, bool clobber)
{
	return NOT_IMPLEMENTED;
}

// File
const BFile &
BResources::File() const
{
	return fFile;
}

// LoadResource
const void *
BResources::LoadResource(type_code type, int32 id, size_t *outSize)
{
	return NULL;	// not implemented
}

// LoadResource
const void *
BResources::LoadResource(type_code type, const char *name, size_t *outSize)
{
	return NULL;	// not implemented
}

// PreloadResourceType
status_t
BResources::PreloadResourceType(type_code type)
{
	return NOT_IMPLEMENTED;
}

// Sync
status_t
BResources::Sync()
{
	return NOT_IMPLEMENTED;
}

// MergeFrom
status_t
BResources::MergeFrom(BFile *fromFile)
{
	return NOT_IMPLEMENTED;
}

// WriteTo
status_t
BResources::WriteTo(BFile *file)
{
	return NOT_IMPLEMENTED;
}

// AddResource
status_t
BResources::AddResource(type_code type, int32 id, const void *data,
						size_t length, const char *name)
{
	return NOT_IMPLEMENTED;
}

// HasResource
bool
BResources::HasResource(type_code type, int32 id)
{
	return false;	// not implemented
}

// HasResource
bool
BResources::HasResource(type_code type, const char *name)
{
	return false;	// not implemented
}

// GetResourceInfo
bool
BResources::GetResourceInfo(int32 byIndex, type_code *typeFound,
							int32 *idFound, const char **nameFound,
							size_t *lengthFound)
{
	return false;	// not implemented
}

// GetResourceInfo
bool
BResources::GetResourceInfo(type_code byType, int32 andIndex, int32 *idFound,
							const char **nameFound, size_t *lengthFound)
{
	return false;	// not implemented
}

// GetResourceInfo
bool
BResources::GetResourceInfo(type_code byType, int32 andID,
							const char **nameFound, size_t *lengthFound)
{
	return false;	// not implemented
}

// GetResourceInfo
bool
BResources::GetResourceInfo(type_code byType, const char *andName,
							int32 *idFound, size_t *lengthFound)
{
	return false;	// not implemented
}

// GetResourceInfo
bool
BResources::GetResourceInfo(const void *byPointer, type_code *typeFound,
							int32 *idFound, size_t *lengthFound,
							const char **nameFound)
{
	return false;	// not implemented
}

// RemoveResource
status_t
BResources::RemoveResource(const void *resource)
{
	return NOT_IMPLEMENTED;
}

// RemoveResource
status_t
BResources::RemoveResource(type_code type, int32 id)
{
	return NOT_IMPLEMENTED;
}


// deprecated

// WriteResource
status_t
BResources::WriteResource(type_code type, int32 id, const void *data,
						  off_t offset, size_t length)
{
	return NOT_IMPLEMENTED;
}

// ReadResource
status_t
BResources::ReadResource(type_code type, int32 id, void *data, off_t offset,
						 size_t length)
{
	return NOT_IMPLEMENTED;
}

// FindResource
void *
BResources::FindResource(type_code type, int32 id, size_t *lengthFound)
{
	return NULL;	// not implemented
}

// FindResource
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

