//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//---------------------------------------------------------------------
/*!
	\file Query.cpp
	BQuery implementation.
*/
#include <Query.h>

enum {
	NOT_IMPLEMENTED	= B_ERROR,
};

// constructor
BQuery::BQuery()
{
}

// destructor
BQuery::~BQuery()
{
}

// Clear
status_t
BQuery::Clear()
{
	return NOT_IMPLEMENTED;
}

// PushAttr
void
BQuery::PushAttr(const char *attrName)
{
}

// PushOp
void
BQuery::PushOp(query_op op)
{
}

// PushUInt32
void
BQuery::PushUInt32(uint32 value)
{
}

// PushInt32
void
BQuery::PushInt32(int32 value)
{
}

// PushUInt64
void
BQuery::PushUInt64(uint64 value)
{
}

// PushInt64
void
BQuery::PushInt64(int64 value)
{
}

// PushFloat
void
BQuery::PushFloat(float value)
{
}

// PushDouble
void
BQuery::PushDouble(double value)
{
}

// PushString
void
BQuery::PushString(const char *value, bool caseInsensitive)
{
}

// PushDate
status_t
BQuery::PushDate(const char *date)
{
	return NOT_IMPLEMENTED;
}

// SetVolume
status_t
BQuery::SetVolume(const BVolume *volume)
{
	return NOT_IMPLEMENTED;
}

// SetPredicate
status_t
BQuery::SetPredicate(const char *expression)
{
	return NOT_IMPLEMENTED;
}

// SetTarget
status_t
BQuery::SetTarget(BMessenger messenger)
{
	return NOT_IMPLEMENTED;
}

// IsLive
bool
BQuery::IsLive() const
{
	return false;	// not implemented
}

// GetPredicate
status_t
BQuery::GetPredicate(char *buffer, size_t length)
{
	return NOT_IMPLEMENTED;
}

// GetPredicate
status_t
BQuery::GetPredicate(BString *predicate)
{
	return NOT_IMPLEMENTED;
}

// PredicateLength
size_t
BQuery::PredicateLength()
{
	return 0;	// not implemented
}

// TargetDevice
dev_t
BQuery::TargetDevice() const
{
	return NOT_IMPLEMENTED;
}

// Fetch
status_t
BQuery::Fetch()
{
	return NOT_IMPLEMENTED;
}


// BEntryList interface

// GetNextEntry
status_t
BQuery::GetNextEntry(BEntry *entry, bool traverse)
{
	return NOT_IMPLEMENTED;
}

// GetNextRef
status_t
BQuery::GetNextRef(entry_ref *ref)
{
	return NOT_IMPLEMENTED;
}

// GetNextDirents
int32
BQuery::GetNextDirents(struct dirent *buf, size_t length, int32 count)
{
	return 0;	// not implemented
}

// Rewind
status_t
BQuery::Rewind()
{
	return NOT_IMPLEMENTED;
}

// CountEntries
int32
BQuery::CountEntries()
{
	return 0;	// not implemented
}


// FBC
void BQuery::_ReservedQuery1() {}
void BQuery::_ReservedQuery2() {}
void BQuery::_ReservedQuery3() {}
void BQuery::_ReservedQuery4() {}
void BQuery::_ReservedQuery5() {}
void BQuery::_ReservedQuery6() {}
