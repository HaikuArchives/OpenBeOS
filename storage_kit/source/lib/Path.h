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

/*! A pathname wrapper class that manages conversion to and from various means of expressing pathnames, as
	well as any necessary resource allocation and deallocation. */
class BPath /*: BFlattenable */ {
public:

	/*! Creates an uninitialized BPath object. */
	BPath();
	
	/*! Creates a BPath object and initializes it to the specified path or path and filename combination. */
	BPath(const char *dir, const char *leaf = NULL, bool normalize = false);
	
	/*! Creates a BPath object and initializes it to the specified directory and filename combination. */
	BPath(const BDirectory *dir, const char *leaf, bool normalize = false);
	
	/*! Creates a copy of the given BPath object. */
	BPath(const BPath &path);
	
	/*! Creates a BPath object and initializes it to the filesystem entry specified by the given BEntry object. */
	BPath(const BEntry *entry);
	
	/*! Creates a BPath object and initializes it to the filesystem entry specified by the given entry_ref struct. */
	BPath(const entry_ref *ref);
	
	/*! Destroys the BPath object and frees any of its associated resources. */
	virtual ~BPath();

	/*! Returns the status of the most recent construction or SetTo() call */
	status_t InitCheck() const;

	/*! Reinitializes the object to the specified path or path and file name combination. */
	status_t SetTo(const char *path, const char *leaf = NULL, bool normalize = false);
	
	/*! Reinitializes the object to the specified directory and relative path combination. */
	status_t SetTo(const BDirectory *dir, const char *path, bool normalize = false);
	
	/*! Reinitializes the object to the specified filesystem entry. */
	status_t SetTo(const BEntry *entry);
	
	/*! Reinitializes the object to the specified filesystem entry. */
	status_t SetTo(const entry_ref *ref);	
	

	/*! Appends the given (relative) path to the end of the current path. This call fails if the
		path is absolute or the object to which you're appending is uninitialized. */
	status_t Append(const char *path, bool normalize = false);
	
	/*! Returns the object to an uninitialized state. The object frees any resources it
		allocated and marks itself as uninitialized. */
	void Unset();
	
	/*! Returns the object's complete path name. */
	const char *Path() const;
	
	/*! Returns the leaf portion of the object's path name. */
	const char *Leaf() const;

	/*! Sets calls the argument's SetTo() method with the name of the object's parent directory. */
	status_t GetParent(BPath *) const;
	
	/*! Performs a simple (string-wise) comparison of paths. */
	bool operator==(const BPath &item) const;

	/*! Performs a simple (string-wise) comparison of paths. */
	bool operator==(const char *path) const;

	/*! Performs a simple (string-wise) comparison of paths. */
	bool operator!=(const BPath &item) const;

	/*! Performs a simple (string-wise) comparison of paths. */
	bool operator!=(const char *path) const;
	
	/*! Initializes the object to be a copy of the argument. */
	BPath& operator=(const BPath &item);

	/*! Initializes the object to be a copy of the argument. */
	BPath& operator=(const char *path);
	
	/*! Returns false. */
	virtual bool IsFixedSize() const;
	
	/*! Returns B_REF_TYPE. */
	virtual type_code TypeCode() const;
	
	/*! Returns the size of the entry_ref structure that represents the flattened pathname. */
	virtual ssize_t FlattenedSize() const;
	
	/*! Converts the object's pathname to an entry_ref and writes it into buffer. */
	virtual status_t Flatten(void *buffer, ssize_t size) const;
	
	/*! Returns true if code is B_REF_TYPE, and false otherwise. */
	virtual bool AllowsTypeCode(type_code code) const;
	
	/*! Initializes the BPath with the flattened entry_ref data that's found in buffer. The type code must be B_REF_TYPE. */
	virtual status_t Unflatten(type_code c, const void *buf, ssize_t size);
	
private:
	/*! Currently unused. */
	virtual void _WarPath1();

	/*! Currently unused. */
	virtual void _WarPath2();

	/*! Currently unused. */
	virtual void _WarPath3();
	
	/*! Currently unused. */
	uint32 _warData[4];
	
	/*! (Probably used to free fName, but I'm not sure yet) */
	status_t clear();	
	
	/*! Pointer to the object's path name string */
	char *fName;
	
	/*! (I'm not sure what this is yet) */ 
	status_t fCStatus;
	
};


#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif

#endif	// __sk_path_h__