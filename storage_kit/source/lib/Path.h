//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Path.h
//  Description: An absolute pathname wrapper class
//---------------------------------------------------------------------
#ifndef __sk_path_h__
#define __sk_path_h__

#include <sys/types.h>
#include <SupportDefs.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

// Forward declarations
class BDirectory;
class BEntry;
struct entry_ref;

//! An absolute pathname wrapper class
/*! Manages conversion to and from various means of expressing path names, as
	well as any necessary resource allocation and deallocation. */
class BPath /*: BFlattenable */ {
public:

	//! Default constructor.
	/*! Creates an uninitialized BPath object. */
	BPath();
	
	//! Constructor accepting a path name and optional leaf name.
	/*! Creates a BPath object and initializes it to the specified path or
		path and filename combination. */
	BPath(const char *dir, const char *leaf = NULL, bool normalize = false);
	
	//! Constructor accepting a BDirectory and the name of a leaf in that directory.
	/*! Creates a BPath object and initializes it to the specified directory
		and filename combination. */
	BPath(const BDirectory *dir, const char *leaf, bool normalize = false);
	
	//! Copy constructor.
	/*! Creates a copy of the given BPath object. */
	BPath(const BPath &path);
	
	//! Constructor accepting a BEntry.
	/*! Creates a BPath object and initializes it to the filesystem entry
		specified by the given BEntry object. */
	BPath(const BEntry *entry);
	
	//! Constructor accepting an entry_ref structure.
	/*! Creates a BPath object and initializes it to the filesystem entry
		specified by the given entry_ref struct. */
	BPath(const entry_ref *ref);
	
	//! Destructor.
	/*! Destroys the BPath object and frees any of its associated resources. */
	virtual ~BPath();

	//! Returns the status of the most recent construction or SetTo() call
	status_t InitCheck() const;

	//! Reinitializes the BPath to the specified path or path and file name combination.
	/*! Reinitializes the BPath to the specified path or path and file name combination. */
	status_t SetTo(const char *path, const char *leaf = NULL, bool normalize = false);
	
	//! Reinitializes the BPath to the specified directory and relative path combination. 
	/*! Reinitializes the BPath to the specified directory and relative path combination. */
	status_t SetTo(const BDirectory *dir, const char *path, bool normalize = false);
	
	//! Reinitializes the BPath to the specified filesystem entry.
	/*! Reinitializes the BPath to the specified filesystem entry. */
	status_t SetTo(const BEntry *entry);
	
	//! Reinitializes the BPath to the specified filesystem entry.
	/*! Reinitializes the BPath to the specified filesystem entry. */
	status_t SetTo(const entry_ref *ref);	
	

		
private:

};


#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif

#endif	// __sk_path_h__