//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Node.h
//---------------------------------------------------------------------
#ifndef __sk_node_h__
#define __sk_node_h__

#include <Statable.h>
#include "kernel_interface.h"

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

//---------------------------------------------------------------
// node_ref
//---------------------------------------------------------------

//! Reference structure to a particular vnode on a particular device
/*! <b>node_ref</b> - A node reference.

	@author <a href="mailto:tylerdauwalder@users.sf.net">Tyler Dauwalder</a>
	@author Be Inc.
	@version 0.0.0
*/
struct node_ref {
	/*! Creates an uninitialized node_ref object. */
	node_ref();	
	/*! Creates a copy of the given node_ref object. */
	node_ref(const node_ref &ref);
	
	bool operator==(const node_ref &ref) const;
	bool operator!=(const node_ref &ref) const;
	node_ref& operator=(const node_ref &ref);
	
	dev_t device;
	ino_t node;
};


// I'll uncomment these forward declarations as needed:
//class BStatable;
//class BEntry;
//class BDirectory;
//struct entry_ref;



//---------------------------------------------------------------
// BNode
//---------------------------------------------------------------

//! A BNode represents a chunk of data in the filesystem.
/*! The BNode class provides an interface for manipulating the data and attributes
	belonging to filesystem entries. The BNode is unaware of the name that refers
	to it in the filesystem (i.e. its entry); a BNode is solely concerned with
	the entry's data and attributes.


	@author <a href='mailto:tylerdauwalder@users.sf.net'>Tyler Dauwalder</a>
	@author Be Inc.	
	@version 0.0.0

*/
class BNode : public BStatable {
public:

	/*! Creates an uninitialized BNode object */
	BNode();
	
	/*! Creates a BNode object and initializes it to the specified
		filesystem entry. */
	BNode(const entry_ref *ref);
	
	/*! Creates a BNode object and initializes it to the specified
		filesystem entry. */
	BNode(const BEntry *entry);
	
	/*! Creates a BNode object and initializes it to the entry referred
		to by the specified path. */
	BNode(const char *path);
	
	/*! Creates a BNode object and initializes it to the entry referred
		to by the specified path rooted in the specified directory. */
	BNode(const BDirectory *dir, const char *path);
	
	/*! Creates a copy of the given BNode. */
	BNode(const BNode &node);
	
	/*! Destroys the object */
	virtual ~BNode();
	


	/*! Checks whether the object has been properly initialized or not. */
	status_t InitCheck() const;
	

	/*! Fills in the given stat structure with <code>stat()</code>
		information for this object. */		
	virtual status_t GetStat(struct stat *st) const;
	

	/*! Reinitializes the object to the specified filesystem entry. */
	status_t SetTo(const entry_ref *ref);

	/*! Reinitializes the object to the specified filesystem entry. */
	status_t SetTo(const BEntry *entry);
	
	/*! Reinitializes the object to the entry referred to by the specified path. */
	status_t SetTo(const char *path);
	
	/*! Reinitializes the object to the entry referred to by the specified path
		rooted in the specified directory. */
	status_t SetTo(const BDirectory *dir, const char *path);
	
	/*! Returns the object to an uninitialized state. */
	void Unset();

	/*! Attains an exclusive lock on the data referred to by this node, so that
		it may not be modified by any other objects or methods. */
	status_t Lock();
	
	/*! Unlocks the node. */
	status_t Unlock();
	
	/*! Immediately performs any pending disk actions on the node. */
	status_t Sync();

	/*! Write the <code>len</code> bytes of data from <code>buffer</code> to
		the attribute specified by <code>name</code> after erasing any data
		that existed previously. The type specified by <code>type</code> <i>is</i>
		remembered, and may be queried with GetAttrInfo(). The value of
		<code>offset</code> is currently ignored. Returns the number of
		bytes actually written. */
	ssize_t WriteAttr(const char *name, type_code type, off_t offset,
		const void *buffer, size_t len);

	/*! Reads the data of the attribute given by <code>name</code> into
		the buffer specified by <code>buffer</code> with length specified
		by <code>len</code>. Returns the number of bytes read. <code>type</code>
		and <code>offset</code>	are currently ignored. */
	ssize_t ReadAttr(const char *name, type_code type, off_t offset,
		void *buffer, size_t len) const;
		
	
	/*! Deletes the attribute given by <code>name</code>. */
	status_t RemoveAttr(const char *name);
	
	/*! Moves the attribute given by <code>oldname</code> to <code>newname</code>.
		If <code>newname</code> already exists, the current data is clobbered. */
	status_t RenameAttr(const char *oldname, const char *newname);
	
	/*! Fills in the pre-allocated attr_info struct pointed to by <code>info</code>
		with usefule information about the attribute specified by <code>name</code>. */
	status_t GetAttrInfo(const char *name, struct attr_info *info) const;
	
	/*! Every BNode maintains a pointer to its list of attributes. GetNextAttrName()
		retrieves the name of the attribute that the pointer is currently pointing to,
		and then bumps the pointer to the next attribute. The name is copied into the
		buffer, which should be at least B_ATTR_NAME_LENGTH characters long. The copied
		name is NULL-terminated. When you've asked for every name in the list, GetNextAttrName()
		returns B_ENTRY_NOT_FOUND. If you pass a NULL pointer, it returns B_BAD_VALUE.
		If successful, it returns B_OK.
	 */
	status_t GetNextAttrName(char *buffer);

	/*! Resets the object's attribute pointer to the first attribute in the list. Returns
		B_BAD_ADDRESS on failure, B_OK on success. */
	status_t RewindAttrs();
	
	/*! Writes the specified string to the specified attribute, clobbering any
		previous data. */
	status_t WriteAttrString(const char *name, const BString *data);
	
	/*! Reads the data of the specified attribute into the pre-allocated <code>result</code>. */
	status_t ReadAttrString(const char *name, BString *result) const;

	/*! Reinitializes the object as a copy of the <code>node</code>. */
	BNode& operator=(const BNode &node);
	
	/*! Two BNode objects are said to be equal if they're set to the same node, or if they're both B_NO_INIT. */
	bool operator==(const BNode &node) const;

	/*! @see operator==() */
	bool operator!=(const BNode &node) const;
	
	/*! Returns a POSIX file descriptor to the node this object refers to. Remember
		to call close() on the file descriptor when you're through with it. */
	int Dup();  // This should be "const" but R5's is not... Ugggh.

private:

	friend class BEntry;
	friend class BVolume;
	friend class BFile;
	friend class BDirectory;
	friend class BSymLink;

	/*! (currently unused) */
	virtual void _RudeNode1(); 	

	/*! (currently unused) */
	virtual void _RudeNode2();

	/*! (currently unused) */
	virtual void _RudeNode3();

	/*! (currently unused) */
	virtual void _RudeNode4();

	/*! (currently unused) */
	virtual void _RudeNode5();

	/*! (currently unused) */
	virtual void _RudeNode6();
	
	/*! Attribute directory */
	StorageKit::Dir fAttrDir;	
	
	/*! (currently unused) */
	uint32 rudeData[3];
	
	
	/*! Used by each implementation (i.e. BNode, BFile, BDirectory, etc.) to set
		the node's file descriptor. This allows each subclass to use the various
		file-type specific system calls for opening file descriptors. */
	status_t set_fd(int fd);
	
	/*! To be implemented by subclasses to close the file descriptor using the
		proper system call for the given file-type. This implementation calls
		StorageKit::close(fFd) and also StorageKit::close_attr_dir(fAttrDir)
		if necessary. */
	virtual void close_fd();

	/*! (currently unused) */
	status_t clear_virtual();
	
	/*! (currently unused) */
	status_t clear();
	
	
	/*! Modifies a certain setting for this node based on what and the
		corresponding value in st. Inherited from and called by BStatable. */
	virtual status_t set_stat(struct stat &st, uint32 what);
	

	/*! (currently unused) */
	status_t set_to(const entry_ref *ref, bool traverse = false);
	
	/*! (currently unused) */
	status_t set_to(const BEntry *entry, bool traverse = false);
	
	/*! (currently unused) */
	status_t set_to(const char *path, bool traverse = false);
	
	/*! (currently unused) */
	status_t set_to(const BDirectory *dir, const char *path, bool traverse = false);
	

	/*! File descriptor for the given node. */
	int fFd;
	
	/*! This appears to be passed to the attribute directory functions
		like a StorageKit::Dir would be, but it's actually a file descriptor.
		Best I can figure, the R5 syscall for reading attributes must've
		just taken a file descriptor. Depending on what our kernel ends up
		providing, this may or may not be replaced with an Dir*  */
	int fAttrFd;
	
	/*! The object's initialization status. */
	status_t fCStatus;
	
	/*! Verifies that the BNode has been properly initialized, and then
		(if necessary) opens the attribute directory on the node's file
		descriptor, storing it in fAttrDir. */
	status_t InitAttrDir();
	

	
};



#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif

#endif	// __sk_node_h__