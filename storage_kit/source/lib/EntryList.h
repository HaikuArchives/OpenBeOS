//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: EntryList.h
//---------------------------------------------------------------------
#ifndef __sk_entry_list_h__
#define __sk_entry_list_h__

#include <dirent.h>
#include <SupportDefs.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

// Forward declarations
class BEntry;
class entry_ref;

//! Interface for iterating through a list of filesystem entries
/*! Defines a general interface for iterating through a list of entries (i.e.
	files in a folder
	
	@author <a href='mailto:tylerdauwalder@users.sf.net'>Tyler Dauwalder</a>
	@author Be Inc.
	@version 0.0.0
*/
class BEntryList {
public:
	BEntryList();
	virtual ~BEntryList();
	
	/*! Places the next entry in the list in entry, traversing symlinks if traverse
		is true. Returns B_OK if successful, B_ENTRY_NOT_FOUND when at the end of
		the list. */
	virtual status_t GetNextEntry(BEntry *entry, bool traverse = false) = 0;
	
	/*! Places an entry_ref to the next entry in the list into ref, traversing symlinks
		if traverse is true. Returns B_OK if successful, B_ENTRY_NOT_FOUND when at the
		end of the list. */
	virtual status_t GetNextRef(entry_ref *ref) = 0;
	
	/*! Reads a number of entries into the array of dirent structures pointed to by buf.
		Reads as many but no more than count entries, as many entries as remain, or as
		many entries as will fit into the array at buf with given length (in bytes),
		whichever is smallest. Returns 0 (claims the BeBook). */
	virtual int32 GetNextDirents(struct dirent *buf, size_t length, int32 count = INT_MAX) = 0;

	/*! Rewinds the list pointer to the beginning of the list. */
	virtual status_t Rewind() = 0;
	
	/*! Returns the number of entries in the list */
	virtual int32 CountEntries() = 0;
	
private:
	/*! Currently unused */
	virtual	void _ReservedEntryList1();

	/*! Currently unused */
	virtual	void _ReservedEntryList2();

	/*! Currently unused */
	virtual	void _ReservedEntryList3();

	/*! Currently unused */
	virtual	void _ReservedEntryList4();

	/*! Currently unused */
	virtual	void _ReservedEntryList5();

	/*! Currently unused */
	virtual	void _ReservedEntryList6();

	/*! Currently unused */
	virtual	void _ReservedEntryList7();

	/*! Currently unused */
	virtual	void _ReservedEntryList8();
	
};

#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif

#endif	// __sk_entry_list_h__
