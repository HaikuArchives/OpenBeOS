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
/*!	\brief Creates an uninitialized BQuery.
*/
BQuery::BQuery()
{
}

// destructor
/*!	\brief Frees all resources associated with the object.
*/
BQuery::~BQuery()
{
}

// Clear
/*!	\brief Resets the object to a uninitialized state.
	\return \c B_OK
*/
status_t
BQuery::Clear()
{
	return NOT_IMPLEMENTED;
}

// PushAttr
/*!	\brief Pushes an attribute name onto the BQuery's predicate stack.
	\param attrName the attribute name
	\return
	- \c B_OK: Everything went fine.
	- \c B_NO_MEMORY: Not enough memory.
	- \c B_NOT_ALLOWED: PushAttribute() was called after Fetch().
	\note In BeOS R5 this method returns \c void. That is checking the return
		  value will render your code source and binary incompatible!
		  Calling PushXYZ() after a Fetch() does change the predicate on R5,
		  but it doesn't affect the active query and the newly created
		  predicate can not even be used for the next query, since in order
		  to be able to reuse the BQuery object for another query, Clear() has
		  to be called and Clear() also deletes the predicate.
*/
status_t
BQuery::PushAttr(const char *attrName)
{
	return NOT_IMPLEMENTED;
}

// PushOp
/*!	\brief Pushes an operator onto the BQuery's predicate stack.
	\param op the code representing the operator
	\return
	- \c B_OK: Everything went fine.
	- \c B_NO_MEMORY: Not enough memory.
	- \c B_NOT_ALLOWED: PushOp() was called after Fetch().
	\note In BeOS R5 this method returns \c void. That is checking the return
		  value will render your code source and binary incompatible!
		  Calling PushXYZ() after a Fetch() does change the predicate on R5,
		  but it doesn't affect the active query and the newly created
		  predicate can not even be used for the next query, since in order
		  to be able to reuse the BQuery object for another query, Clear() has
		  to be called and Clear() also deletes the predicate.
*/
status_t
BQuery::PushOp(query_op op)
{
	return NOT_IMPLEMENTED;
}

// PushUInt32
/*!	\brief Pushes a uint32 value onto the BQuery's predicate stack.
	\param value the value
	\return
	- \c B_OK: Everything went fine.
	- \c B_NO_MEMORY: Not enough memory.
	- \c B_NOT_ALLOWED: PushUInt32() was called after Fetch().
	\note In BeOS R5 this method returns \c void. That is checking the return
		  value will render your code source and binary incompatible!
		  Calling PushXYZ() after a Fetch() does change the predicate on R5,
		  but it doesn't affect the active query and the newly created
		  predicate can not even be used for the next query, since in order
		  to be able to reuse the BQuery object for another query, Clear() has
		  to be called and Clear() also deletes the predicate.
*/
status_t
BQuery::PushUInt32(uint32 value)
{
	return NOT_IMPLEMENTED;
}

// PushInt32
/*!	\brief Pushes an int32 value onto the BQuery's predicate stack.
	\param value the value
	\return
	- \c B_OK: Everything went fine.
	- \c B_NO_MEMORY: Not enough memory.
	- \c B_NOT_ALLOWED: PushInt32() was called after Fetch().
	\note In BeOS R5 this method returns \c void. That is checking the return
		  value will render your code source and binary incompatible!
		  Calling PushXYZ() after a Fetch() does change the predicate on R5,
		  but it doesn't affect the active query and the newly created
		  predicate can not even be used for the next query, since in order
		  to be able to reuse the BQuery object for another query, Clear() has
		  to be called and Clear() also deletes the predicate.
*/
status_t
BQuery::PushInt32(int32 value)
{
	return NOT_IMPLEMENTED;
}

// PushUInt64
/*!	\brief Pushes a uint64 value onto the BQuery's predicate stack.
	\param value the value
	\return
	- \c B_OK: Everything went fine.
	- \c B_NO_MEMORY: Not enough memory.
	- \c B_NOT_ALLOWED: PushUInt64() was called after Fetch().
	\note In BeOS R5 this method returns \c void. That is checking the return
		  value will render your code source and binary incompatible!
		  Calling PushXYZ() after a Fetch() does change the predicate on R5,
		  but it doesn't affect the active query and the newly created
		  predicate can not even be used for the next query, since in order
		  to be able to reuse the BQuery object for another query, Clear() has
		  to be called and Clear() also deletes the predicate.
*/
status_t
BQuery::PushUInt64(uint64 value)
{
	return NOT_IMPLEMENTED;
}

// PushInt64
/*!	\brief Pushes an int64 value onto the BQuery's predicate stack.
	\param value the value
	\return
	- \c B_OK: Everything went fine.
	- \c B_NO_MEMORY: Not enough memory.
	- \c B_NOT_ALLOWED: PushInt64() was called after Fetch().
	\note In BeOS R5 this method returns \c void. That is checking the return
		  value will render your code source and binary incompatible!
		  Calling PushXYZ() after a Fetch() does change the predicate on R5,
		  but it doesn't affect the active query and the newly created
		  predicate can not even be used for the next query, since in order
		  to be able to reuse the BQuery object for another query, Clear() has
		  to be called and Clear() also deletes the predicate.
*/
status_t
BQuery::PushInt64(int64 value)
{
	return NOT_IMPLEMENTED;
}

// PushFloat
/*!	\brief Pushes a float value onto the BQuery's predicate stack.
	\param value the value
	\return
	- \c B_OK: Everything went fine.
	- \c B_NO_MEMORY: Not enough memory.
	- \c B_NOT_ALLOWED: PushFloat() was called after Fetch().
	\note In BeOS R5 this method returns \c void. That is checking the return
		  value will render your code source and binary incompatible!
		  Calling PushXYZ() after a Fetch() does change the predicate on R5,
		  but it doesn't affect the active query and the newly created
		  predicate can not even be used for the next query, since in order
		  to be able to reuse the BQuery object for another query, Clear() has
		  to be called and Clear() also deletes the predicate.
*/
status_t
BQuery::PushFloat(float value)
{
	return NOT_IMPLEMENTED;
}

// PushDouble
/*!	\brief Pushes a double value onto the BQuery's predicate stack.
	\param value the value
	\return
	- \c B_OK: Everything went fine.
	- \c B_NO_MEMORY: Not enough memory.
	- \c B_NOT_ALLOWED: PushDouble() was called after Fetch().
	\note In BeOS R5 this method returns \c void. That is checking the return
		  value will render your code source and binary incompatible!
		  Calling PushXYZ() after a Fetch() does change the predicate on R5,
		  but it doesn't affect the active query and the newly created
		  predicate can not even be used for the next query, since in order
		  to be able to reuse the BQuery object for another query, Clear() has
		  to be called and Clear() also deletes the predicate.
*/
status_t
BQuery::PushDouble(double value)
{
	return NOT_IMPLEMENTED;
}

// PushString
/*!	\brief Pushes a string value onto the BQuery's predicate stack.
	\param value the value
	\param caseInsensitive \c true, if the case of the string should be
		   ignored, \c false otherwise
	\return
	- \c B_OK: Everything went fine.
	- \c B_NO_MEMORY: Not enough memory.
	- \c B_NOT_ALLOWED: PushString() was called after Fetch().
	\note In BeOS R5 this method returns \c void. That is checking the return
		  value will render your code source and binary incompatible!
		  Calling PushXYZ() after a Fetch() does change the predicate on R5,
		  but it doesn't affect the active query and the newly created
		  predicate can not even be used for the next query, since in order
		  to be able to reuse the BQuery object for another query, Clear() has
		  to be called and Clear() also deletes the predicate.
*/
status_t
BQuery::PushString(const char *value, bool caseInsensitive)
{
	return NOT_IMPLEMENTED;
}

// PushDate
/*!	\brief Pushes a date value onto the BQuery's predicate stack.
	The supplied date can be any string understood by the POSIX function
	parsedate().
	\param date the date string
	\return
	- \c B_OK: Everything went fine.
	- \c B_ERROR: Error parsing the string.
	- \c B_NOT_ALLOWED: PushDate() was called after Fetch().
	\note Calling PushXYZ() after a Fetch() does change the predicate on R5,
		  but it doesn't affect the active query and the newly created
		  predicate can not even be used for the next query, since in order
		  to be able to reuse the BQuery object for another query, Clear() has
		  to be called and Clear() also deletes the predicate.
*/
status_t
BQuery::PushDate(const char *date)
{
	return NOT_IMPLEMENTED;
}

// SetVolume
/*!	\brief Sets the BQuery's volume.
	A query is restricted to one volume. This method sets this volume. It
	fails, if called after Fetch(). To reuse a BQuery object it has to be
	reset via Clear().
	\param volume the volume
	\return
	- \c B_OK: Everything went fine.
	- \c B_NOT_ALLOWED: SetVolume() was called after Fetch().
*/
status_t
BQuery::SetVolume(const BVolume *volume)
{
	return NOT_IMPLEMENTED;
}

// SetPredicate
/*!	\brief Sets the BQuery's predicate.
	A predicate can be set either using this method or constructing one on
	the predicate stack. The two methods can not be mixed. The letter one
	has precedence over this one.
	The method fails, if called after Fetch(). To reuse a BQuery object it has
	to be reset via Clear().
	\param predicate the predicate string
	\return
	- \c B_OK: Everything went fine.
	- \c B_NOT_ALLOWED: SetPredicate() was called after Fetch().
	- \c B_NO_MEMORY: Insufficient memory to store the predicate.
*/
status_t
BQuery::SetPredicate(const char *expression)
{
	return NOT_IMPLEMENTED;
}

// SetTarget
/*!	\brief Sets the BQuery's target and makes the query live.
	The query update messages are sent to the specified target. They might
	roll in immediately after calling Fetch().
	This methods fails, if called after Fetch(). To reuse a BQuery object it
	has to be reset via Clear().
	\return
	- \c B_OK: Everything went fine.
	- \c B_BAD_VALUE: \a messenger was not properly initialized.
	- \c B_NOT_ALLOWED: SetTarget() was called after Fetch().
*/
status_t
BQuery::SetTarget(BMessenger messenger)
{
	return NOT_IMPLEMENTED;
}

// IsLive
/*!	\brief Returns whether the query associated with this object is live.
	\return \c true, if the query is live, \c false otherwise
*/
bool
BQuery::IsLive() const
{
	return false;	// not implemented
}

// GetPredicate
/*!	\brief Returns the BQuery's predicate.
	Regardless of whether the predicate has been constructed using the
	predicate stack or set via SetPredicate(), this method returns a
	string representation.
	\param buffer a pointer to a buffer into which the predicate shall be
		   written
	\param length the size of the provided buffer
	\return
	- \c B_OK: Everything went fine.
	- \c B_NO_INIT: The predicate isn't set.
	- \c B_BAD_VALUE: \a buffer is \c NULL or too short.
	\note This method causes the predicate stack to be evaluated and cleared.
		  You can't interleave Push*() and GetPredicate() calls.
*/
status_t
BQuery::GetPredicate(char *buffer, size_t length)
{
	return NOT_IMPLEMENTED;
}

// GetPredicate
/*!	\brief Returns the BQuery's predicate.
	Regardless of whether the predicate has been constructed using the
	predicate stack or set via SetPredicate(), this method returns a
	string representation.
	\param predicate a pointer to a BString which shall be set to the
		   predicate string
	\return
	- \c B_OK: Everything went fine.
	- \c B_NO_INIT: The predicate isn't set.
	- \c B_BAD_VALUE: \c NULL \a predicate.
	\note This method causes the predicate stack to be evaluated and cleared.
		  You can't interleave Push*() and GetPredicate() calls.
*/
status_t
BQuery::GetPredicate(BString *predicate)
{
	return NOT_IMPLEMENTED;
}

// PredicateLength
/*!	\brief Returns the length of the BQuery's predicate string.
	Regardless of whether the predicate has been constructed using the
	predicate stack or set via SetPredicate(), this method returns the length
	of its string representation.
	\return
	- the length of the predicate string or
	- 0, if an error occured
	\note This method causes the predicate stack to be evaluated and cleared.
		  You can't interleave Push*() and PredicateLength() calls.
*/
size_t
BQuery::PredicateLength()
{
	return 0;	// not implemented
}

// TargetDevice
/*!	\brief Returns the device ID identifying the BQuery's volume.
	\return the device ID of the BQuery's volume or \c B_NO_INIT, if the
			volume isn't set.
	
*/
dev_t
BQuery::TargetDevice() const
{
	return NOT_IMPLEMENTED;
}

// Fetch
/*!	\brief Tells the BQuery to start fetching entries satisfying the predicate.
	After Fetch() has been called GetNextEntry(), GetNextRef() and
	GetNextDirents() can be used to retrieve the enties. Live query updates
	may be sent immediately after this method has been called.
	Fetch() fails, if it has already been called. To reuse a BQuery object it
	has to be reset via Clear().
	\return
	- \c B_OK: Everything went fine.
	- \c B_NO_INIT: The predicate or the volume aren't set.
	- \c B_BAD_VALUE: The predicate is invalid.
	- \c B_NOT_ALLOWED: Fetch() has already been called.
*/
status_t
BQuery::Fetch()
{
	return NOT_IMPLEMENTED;
}


// BEntryList interface

// GetNextEntry
/*!	\brief Returns the BQuery's next entry as a BEntry.
	Places the next entry in the list in \a entry, traversing symlinks if
	\a traverse is \c true.
	\param entry a pointer to a BEntry to be initialized with the found entry
	\param traverse specifies whether to follow it, if the found entry
		   is a symbolic link.
	\note The iterator used by this method is the same one used by
		  GetNextRef() and GetNextDirents().
	\return
	- \c B_OK if successful,
	- \c B_ENTRY_NOT_FOUND when at the end of the list,
	- \c B_BAD_VALUE: The queries predicate includes unindexed attributes.
	- \c B_FILE_ERROR: Fetch() has not been called before.
*/
status_t
BQuery::GetNextEntry(BEntry *entry, bool traverse)
{
	return NOT_IMPLEMENTED;
}

// GetNextRef
/*!	\brief Returns the BQuery's next entry as an entry_ref.
	Places an entry_ref to the next entry in the list into \a ref.
	\param ref a pointer to an entry_ref to be filled in with the data of the
		   found entry
	\note The iterator used by this method is the same one used by
		  GetNextEntry() and GetNextDirents().
	\return
	- \c B_OK if successful,
	- \c B_ENTRY_NOT_FOUND when at the end of the list,
	- \c B_BAD_VALUE: The queries predicate includes unindexed attributes.
	- \c B_FILE_ERROR: Fetch() has not been called before.
*/
status_t
BQuery::GetNextRef(entry_ref *ref)
{
	return NOT_IMPLEMENTED;
}

// GetNextDirents
/*!	\brief Returns the BQuery's next entries as dirent structures.
	Reads a number of entries into the array of dirent structures pointed to by
	\a buf. Reads as many but no more than \a count entries, as many entries as
	remain, or as many entries as will fit into the array at \a buf with given
	length \a length (in bytes), whichever is smallest.
	\param buf a pointer to a buffer to be filled with dirent structures of
		   the found entries
	\param length the maximal number of entries to be read.
	\note The iterator used by this method is the same one used by
		  GetNextEntry() and GetNextRef().
	\return
	- The number of dirent structures stored in the buffer, 0 when there are
	  no more entries to be read.
	- \c B_BAD_VALUE: The queries predicate includes unindexed attributes.
	- \c B_FILE_ERROR: Fetch() has not been called before.
*/
int32
BQuery::GetNextDirents(struct dirent *buf, size_t length, int32 count)
{
	return 0;	// not implemented
}

// Rewind
/*!	\brief Unimplemented method of the BEntryList interface.
	\return \c B_ERROR.
*/
status_t
BQuery::Rewind()
{
	return NOT_IMPLEMENTED;
}

// CountEntries
/*!	\brief Unimplemented method of the BEntryList interface.
	\return 0.
*/
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
