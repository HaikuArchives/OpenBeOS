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

#include "Statable.h"

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

class	BDirectory;
class	BPath;
struct	entry_ref;

_IMPEXP_BE status_t	get_ref_for_path(const char *path, entry_ref *ref);
_IMPEXP_BE bool operator<(const entry_ref & a, const entry_ref & b);

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
	\class BEntry
	\brief An Entry represents a location in a file system.
	
	The BEntry class defines objects that represent "locations" in the file system
	hierarchy.  Each location (or entry) is given as a name within a directory. For
	example, when you create a BEntry thus.
	
	BEntry entry("/boot/home/fido");
	
	...you're telling the BEntry object to represent the location of the file 	called fido within the directory "/
	boot/home".
*/

class BEntry : public BStatable {
public:
	/*! Creates an uninitialized BEntry object. */
	BEntry();
	
	/*! Creates a BEntry object initialised to path or a dir path combination, resolves symlinks if traverse is true */
	BEntry(const BDirectory *dir, const char *path, bool traverse = false);
	
	/*! Creates a BEntry object initialised to the entry_ref, resolves symlinks if traverse is true */
	BEntry(const entry_ref *ref, bool traverse = false);
	
	/*! Creates a BEntry object initialised to the path, resolves symlinks if traverse is true */
	BEntry(const char *path, bool traverse = false);
	
	/*! Creates a BEntry object initialised another entry - copy constructor*/
	BEntry(const BEntry &entry);
	
	/*! Destructor, frees all previously allocated resources */
	virtual ~BEntry();
	
	/*! Returns the status of the most recent construction or SetTo() call */
	status_t InitCheck() const;
	
	/*! Returns true if the Entry exists in the filesytem, false otherwise. */
	bool Exists() const;
	
	/*! Gets a stat structure for the Entry */
	virtual status_t GetStat(struct stat *st) const;
	
	/*! Reinitialises the BEntry to the path or directory path combination, resolves symlinks if traverse is true */
	status_t SetTo(const BDirectory *dir, const char *path, bool traverse = false);
	
	/*! Reinitialises the BEntry to the entry_ref, resolves symlinks if traverse is true */
	status_t SetTo(const entry_ref *ref, bool traverse = false);
	
	/*! Reinitialises the BEntry object to the path, resolves symlinks if traverse is true */
	status_t SetTo(const char *path, bool traverse = false);
	
	/*! Reinitialises the BEntry to an uninitialised BEntry object */
	void Unset();
	
	/*! Gets an entry_ref structure from the BEntry */
	status_t GetRef(entry_ref *ref) const;
	
	/*! Gets the path for the BEntry */
	status_t GetPath(BPath *path) const;
	
	/*! Gets the parent of the BEntry */
	status_t GetParent(BEntry *entry) const;
	
	/*! Gets the parent of the BEntry as a BDirectory */
	status_t GetParent(BDirectory *dir) const;
	
	/*! Gets the name of the */
	status_t GetName(char *buffer) const;
	
	/*! Renames the BEntry to path, replacing an existing entry if clobber is true. */
	status_t Rename(const char *path, bool clobber = false);
	
	/*! Moves the BEntry to path or dir path combination, replacing an existing entry if clober is true. */
	status_t MoveTo(BDirectory *dir, const char *path = NULL, bool clobber = false);
	
	/*! Removes the entry from the file system */
	status_t Remove();
	
	/*! Equality operator */
	bool operator==(const BEntry &item) const;
	
	/*! Inequality operator */
	bool operator!=(const BEntry &item) const;
	
	/*! Reinitialises the BEntry to be a copy of the arguement */
	BEntry & operator=(const BEntry &item);
		
private:
	friend class BNode;
	friend class BDirectory;
	friend class BFile;
	friend class BSymLink;
	
	/*! Currently unused. */
	virtual	void _PennyEntry1();
	
	/*! Currently unused. */
	virtual	void _PennyEntry2();
	
	/*! Currently unused. */
	virtual	void _PennyEntry3();
	
	/*! Currently unused. */
	virtual	void _PennyEntry4();
	
	/*! Currently unused. */
	virtual	void _PennyEntry5();
	
	/*! Currently unused. */
	virtual	void _PennyEntry6();
	
	/*! Currently unused. */
	uint32 _pennyData[4];

	/*! Updates the BEntry with the data from the stat structure according to the mask. */
	virtual	status_t set_stat(struct stat &st, uint32 what);

	/*! Probably called by MoveTo. */
	status_t move(int fd, const char *path);
			
	/*! Probably used by all of the constructors and SetTo's to match the file descriptor to the path. */
	status_t set(int fd, const char *path, bool traverse);

	/*! Probably called by Unset() */
	status_t clear();

	/*! File Descriptor for BEntries that Exist. */
	int	fDfd;
	
	/*! Probably the full name of the BEntry */
	char *fName;
	
	/*! Probably the status_t result of the most recent function call */
	status_t fCStatus;
};

#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif

#endif	// __sk_path_h__