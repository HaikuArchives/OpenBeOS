//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Directory.h
//---------------------------------------------------------------------
#ifndef __sk_directory_h__
#define __sk_directory_h__

#include <Node.h>
#include <EntryList.h>
#include "kernel_interface.h"


#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

//class	BEntry;
//class	BFile;
//class	BSymLink;
//struct	entry_ref;

//! A directory in the filesystem
/*! Provides an interface for manipulating directories and their contents. */
class BDirectory : public BNode, BEntryList {
public:
	/*! Creates and uninitialized Directory object */
	BDirectory();

	BDirectory(const BEntry *entry);
	BDirectory(const entry_ref *ref);

	/*! Creates a Directory object that refers to the directory given by path */
	BDirectory(const char *path);

/*

						BDirectory(const BDirectory *dir, const char *path);
						BDirectory(const node_ref *ref);
						BDirectory(const BDirectory &dir);

virtual					~BDirectory();

		status_t		SetTo(const entry_ref *ref);
		status_t		SetTo(const BEntry *entry);
		status_t		SetTo(const char *path);
		status_t		SetTo(const BDirectory *dir, const char *path);
		status_t		SetTo(const node_ref *ref);

*/

		status_t		GetEntry(BEntry *entry) const;

/*
		bool			IsRootDirectory() const;

		status_t		FindEntry(const char *path, BEntry *entry,
								  bool traverse = false) const;

		bool			Contains(const char *path, 
								 int32 node_flags = B_ANY_NODE) const;
		bool			Contains(const BEntry *entry,
								 int32 node_flags = B_ANY_NODE) const;

		status_t		GetStatFor(const char *path, struct stat *st) const;
*/

virtual	status_t		GetNextEntry(BEntry *entry, bool traverse = false);
virtual	status_t		GetNextRef(entry_ref *ref);
virtual	int32			GetNextDirents(struct dirent *buf, size_t length,
							int32 count = INT_MAX);
virtual	status_t		Rewind();
virtual	int32			CountEntries();

/*
		status_t		CreateDirectory(const char *path, BDirectory *dir);
		status_t		CreateFile(const char *path, BFile *file, 
								   bool failIfExists = false);
		status_t		CreateSymLink(const char *path, const char *content,
							BSymLink *link);

		BDirectory &	operator=(const BDirectory &dir);

private:

friend class		BEntry;
friend class		BVolume;

virtual	void		_ErectorDirectory1();
virtual	void		_ErectorDirectory2();
virtual	void		_ErectorDirectory3();
virtual	void		_ErectorDirectory4();
virtual	void		_ErectorDirectory5();
virtual	void		_ErectorDirectory6();
		uint32		_erectorData[7];

virtual void		close_fd();
		status_t	set_fd(int fd);
		
*/

//		int			fDirFd;
		/*! The directory this object represents. This member replaces fDirFd from the R5
			implementation. */
		StorageKit::Dir fDir;
	
};

#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif

#endif	// __sk_directory_h__