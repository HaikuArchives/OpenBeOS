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
BSymLink::BSymLink()
{
}

// constructor
BSymLink::BSymLink(const entry_ref *ref)
{
}

// constructor
BSymLink::BSymLink(const BEntry *entry)
{
}

// constructor
BSymLink::BSymLink(const char *path)
{
}

// constructor
BSymLink::BSymLink(const BDirectory *dir, const char *path)
{
}

// copy constructor
BSymLink::BSymLink(const BSymLink &link)
{
}

// destructor
BSymLink::~BSymLink()
{
}

// ReadLink
ssize_t
BSymLink::ReadLink(char *buf, size_t length) const
{
	return NOT_IMPLEMENTED;
}

// MakeLinkedPath
ssize_t
BSymLink::MakeLinkedPath(const char *dirPath, BPath *path)
{
	return NOT_IMPLEMENTED;
}

// MakeLinkedPath
ssize_t
BSymLink::MakeLinkedPath(const BDirectory *dir, BPath *path)
{
	return NOT_IMPLEMENTED;
}

// IsAbsolute
bool
BSymLink::IsAbsolute()
{
	return false;	// not implemented
}


void BSymLink::_ReservedSymLink1() {}
void BSymLink::_ReservedSymLink2() {}
void BSymLink::_ReservedSymLink3() {}
void BSymLink::_ReservedSymLink4() {}
void BSymLink::_ReservedSymLink5() {}
void BSymLink::_ReservedSymLink6() {}


#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif
