//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//---------------------------------------------------------------------
/*!
	\file Resources.h
	BResources interface declaration.
*/

#ifndef _sk_resources_h_
#define _sk_resources_h_

#include <File.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

/*!
	\class BResources
	\brief Represent the resources in a file
	
	Provides an interface for accessing and manipulating resources.

	\author <a href='mailto:bonefish@users.sf.net'>Ingo Weinhold</a>
	
	\version 0.0.0
*/
class BResources {
public:
	BResources();
	BResources(BFile *file, bool clobber = false);
	virtual ~BResources();

	status_t SetTo(BFile *file, bool clobber = false);

	const BFile &File() const;

	const void *LoadResource(type_code type, int32 id, size_t *outSize);
	const void *LoadResource(type_code type, const char *name,
							 size_t *outSize);

	status_t PreloadResourceType(type_code type = 0);

	status_t Sync();
	status_t MergeFrom(BFile *fromFile);
	status_t WriteTo(BFile *file);

	status_t AddResource(type_code type, int32 id, const void *data,
						 size_t length, const char *name = NULL);

	bool HasResource(type_code type, int32 id);
	bool HasResource(type_code type, const char *name);

	bool GetResourceInfo(int32 byIndex, type_code *typeFound, int32 *idFound,
						 const char **nameFound, size_t *lengthFound);
	bool GetResourceInfo(type_code byType, int32 andIndex, int32 *idFound,
						 const char **nameFound, size_t *lengthFound);
	bool GetResourceInfo(type_code byType, int32 andID,
						 const char **nameFound, size_t *lengthFound);
	bool GetResourceInfo(type_code byType, const char *andName, int32 *idFound,
						 size_t *lengthFound);
	bool GetResourceInfo(const void *byPointer, type_code *typeFound,
						 int32 *idFound, size_t *lengthFound,
						 const char **nameFound);

	status_t RemoveResource(const void *resource);
	status_t RemoveResource(type_code type, int32 id);


	// deprecated

	status_t WriteResource(type_code type, int32 id, const void *data,
						   off_t offset, size_t length);

	status_t ReadResource(type_code type, int32 id, void *data, off_t offset,
						  size_t length);

	void *FindResource(type_code type, int32 id, size_t *lengthFound);
	void *FindResource(type_code type, const char *name, size_t *lengthFound);

private:
	// FBC
	virtual void _ReservedResources1();
	virtual void _ReservedResources2();
	virtual void _ReservedResources3();
	virtual void _ReservedResources4();
	virtual void _ReservedResources5();
	virtual void _ReservedResources6();
	virtual void _ReservedResources7();
	virtual void _ReservedResources8();

private:
	BFile	fFile;
	bool	fReadOnly;
	bool	fDirty;
	bool	_pad[2];
	uint32	_unused1[2];
	uint32	_reserved[3];	// FBC
};


#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif

#endif	// _sk_resources_h_
