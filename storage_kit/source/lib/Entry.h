//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Entry.h
//  Description: A file location wrapper class
//---------------------------------------------------------------------
#ifndef __sk_entry_h__
#define __sk_entry_h__

#include <sys/types.h>
#include <sys/stat.h>
#include <SupportDefs.h>

#include <Statable.h>
#include "kernel_interface.h"

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

class	BDirectory;
class	BPath;
struct	entry_ref;

//_IMPEXP_BE status_t	get_ref_for_path(const char *path, entry_ref *ref);
//_IMPEXP_BE bool operator<(const entry_ref & a, const entry_ref & b);

struct entry_ref {
						entry_ref();
  						entry_ref(dev_t dev, ino_t dir, const char *name);
						entry_ref(const entry_ref &ref);
						~entry_ref();
	
  	status_t 			set_name(const char *name);

	bool				operator==(const entry_ref &ref) const;
	bool				operator!=(const entry_ref &ref) const;
	entry_ref &			operator=(const entry_ref &ref);

	dev_t				device;
	ino_t				directory;
	char				*name;
};

/*!
	@class BEntry
	@brief An Entry represents a location in a file system.
	
	The BEntry class defines objects that represent "locations" in the file system
	hierarchy.  Each location (or entry) is given as a name within a directory. For
	example, when you create a BEntry thus.
	
	BEntry entry("/boot/home/fido");
	
	...you're telling the BEntry object to represent the location of the file 	called fido within the directory "/
	boot/home".
	
	@author <a href='mailto:tylerdauwalder@users.sf.net'>Tyler Dauwalder</a>
	@author <a href='mailto:scusack@users.sf.net'>Simon Cusack</a>
	@author Be Inc.
	
	@version 0.0.0
*/
class BEntry : public BStatable {
public:
	BEntry();
	BEntry(const BDirectory *dir, const char *path, bool traverse = false);
	BEntry(const entry_ref *ref, bool traverse = false);
	BEntry(const char *path, bool traverse = false);
	BEntry(const BEntry &entry);
	virtual ~BEntry();
	
	status_t InitCheck() const;
	bool Exists() const;
	
	virtual status_t GetStat(struct stat *st) const;
	
	status_t SetTo(const BDirectory *dir, const char *path, bool traverse = false);
	status_t SetTo(const entry_ref *ref, bool traverse = false);
	status_t SetTo(const char *path, bool traverse = false);
	void Unset();
	
	status_t GetRef(entry_ref *ref) const;
	status_t GetPath(BPath *path) const;
	status_t GetParent(BEntry *entry) const;
	status_t GetParent(BDirectory *dir) const;
	status_t GetName(char *buffer) const;
	status_t Rename(const char *path, bool clobber = false);
	
	status_t MoveTo(BDirectory *dir, const char *path = NULL, bool clobber = false);
	status_t Remove();
	
	bool operator==(const BEntry &item) const;
	bool operator!=(const BEntry &item) const;
	
	BEntry & operator=(const BEntry &item);
		
private:
	friend class BNode;
	friend class BDirectory;
	friend class BFile;
	friend class BSymLink;
	friend class EntryTest;
	
	virtual	void _PennyEntry1();
	virtual	void _PennyEntry2();
	virtual	void _PennyEntry3();
	virtual	void _PennyEntry4();
	virtual	void _PennyEntry5();
	virtual	void _PennyEntry6();
	
	/*! The entry's parent directory. This may replace fDfd once we
		get the new kernel. */
	StorageKit::Dir fDir;

	/*! Currently unused. */
	uint32 _pennyData[3];

	virtual	status_t set_stat(struct stat &st, uint32 what);
	status_t move(int fd, const char *path);
//	status_t set(int fd, const char *path, bool traverse);
	status_t set(StorageKit::Dir dir, const char *path, bool traverse);
	status_t clear();

	/*! File descriptor for the entry's parent directory. */
	int	fDfd;
	
	/*! Leaf name of the entry. */
	char *fLeaf;
	
	/*! Probably the status_t result of the most recent function call */
	status_t fCStatus;
	
	status_t SetLeaf(const char *leaf);

	static bool SplitPathInTwain(const char* fullPath, char *&path, char *&leaf);
};

#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif

#endif	// __sk_path_h__