//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: SymLink.h
//---------------------------------------------------------------------
#ifndef __sk_sym_link_h__
#define __sk_sym_link_h__

#include <Node.h>
#include <StorageDefs.h>
#include "kernel_interface.h"

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

/*!
	\class BSymLink
	\brief A symbolic link in the filesystem
	
	Provides an interface for manipulating symbolic links.

	\author <a href='mailto:bonefish@users.sf.net'>Ingo Weinhold</a>
	
	\version 0.0.0
*/
class BSymLink : public BNode {
public:
	BSymLink();
	BSymLink(const entry_ref *ref);
	BSymLink(const BEntry *entry);
	BSymLink(const char *path);
	BSymLink(const BDirectory *dir, const char *path);
	BSymLink(const BSymLink &link);

	virtual ~BSymLink();

	ssize_t ReadLink(char *buf, size_t length) const;

	ssize_t MakeLinkedPath(const char *dirPath, BPath *path);
	ssize_t MakeLinkedPath(const BDirectory *dir, BPath *path);

	bool IsAbsolute();

private:
	virtual void _ReservedSymLink1();
	virtual void _ReservedSymLink2();
	virtual void _ReservedSymLink3();
	virtual void _ReservedSymLink4();
	virtual void _ReservedSymLink5();
	virtual void _ReservedSymLink6();

	uint32 _reservedData[4];
};

#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif

#endif	// __sk_sym_link_h__
