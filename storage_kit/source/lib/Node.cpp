//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Node.cpp
//---------------------------------------------------------------------

#include <Node.h>
#include <errno.h>
#include <fs_attr.h> // for struct attr_info
#include "kernel_interface.h"
#include "Error.h"


#include <iostream>

BNode::BNode() : fFd(-1), fAttrFd(-1), fAttrDir(NULL), fCStatus(B_NO_INIT) {
}

BNode::BNode(const entry_ref *ref) {
}

BNode::BNode(const BEntry *entry) {
}

BNode::BNode(const char *path) : fFd(-1), fAttrFd(-1), fAttrDir(NULL), fCStatus(B_NO_INIT)  {
	SetTo(path);
}

BNode::BNode(const BDirectory *dir, const char *path) {
}

BNode::BNode(const BNode &node) {
}

BNode::~BNode() {
	close_fd();
}

status_t
BNode::InitCheck() const {
	return fCStatus;
}

status_t
BNode::GetStat(struct stat *st) const {
	return (fCStatus != B_OK) ? fCStatus : StorageKit::get_stat(fFd, st) ;
}

status_t
BNode::SetTo(const entry_ref *ref) {
	return B_ERROR;
}

status_t
BNode::SetTo(const BEntry *entry) {
	return B_ERROR;
}

status_t
BNode::SetTo(const char *path) {
	try {

		set_fd(StorageKit::open(path, StorageKit::READ_WRITE));
		fCStatus = B_OK;

	} catch (StorageKit::EEntryNotFound *e) {

		set_fd(-1);
		fCStatus = B_ENTRY_NOT_FOUND;
		delete e;

	}	
}

status_t
BNode::SetTo(const BDirectory *dir, const char *path) {
	return B_ERROR;
}

void
BNode::Unset() {
	close_fd();
}

status_t
BNode::Lock() {
	if (fCStatus == B_NO_INIT)
		return B_NO_INIT;

	StorageKit::FileLock lock;
	try {
		StorageKit::lock(fFd, StorageKit::READ, &lock);
	} catch (StorageKit::Error *e) {
		delete e;
		return B_BUSY;
	}
	
	return B_OK;
}

status_t
BNode::Unlock() {
	return B_ERROR;
}

status_t
BNode::Sync() {
	return (fCStatus != B_OK) ? fCStatus : StorageKit::sync(fFd) ;
}

ssize_t
BNode::WriteAttr(const char *attr, type_code type, off_t offset,
const void *buffer, size_t len) {
	if (fCStatus == B_NO_INIT)
		return B_FILE_ERROR;
	else {
		ssize_t result = StorageKit::write_attr(fFd, attr, type, offset, buffer, len);
		return (result >= 0) ? result : errno ;
	}
}

ssize_t
BNode::ReadAttr(const char *attr, type_code type, off_t offset,
void *buffer, size_t len) const {
	if (fCStatus == B_NO_INIT)
		return B_FILE_ERROR;
	else {
		ssize_t result = StorageKit::read_attr(fFd, attr, type, offset, buffer, len);
		return (result >= 0) ? result : errno ;
	}
}

status_t
BNode::RemoveAttr(const char *name) {
	return (fCStatus == B_NO_INIT) ? B_FILE_ERROR : StorageKit::remove_attr(fFd, name);
}


status_t
BNode::RenameAttr(const char *oldname, const char *newname) {

	if (fCStatus != B_OK)
		return B_FILE_ERROR;

	attr_info info;
	status_t result;
	ssize_t size;
	
	// Figure out how much data there is
	result = GetAttrInfo(oldname, &info);
	if (result != B_OK)
		return B_BAD_VALUE;	// This is what R5::BNode returns...
		
	// Alloc a buffer
	void *data = new char[info.size];
	if (data == NULL)
		return B_NO_MEMORY;
		
	// Read in the data
	size = ReadAttr(oldname, B_STRING_TYPE, 0, data, info.size);
	if (size != info.size) {
		delete data;		
		return size;
	}
		
	// Write it to the new attribute
	size = WriteAttr(newname, B_STRING_TYPE, 0, data, size);
	if (size != info.size) {
		delete data;		
		return size;
	}
	
	// We're done with the buffer now.
	delete data;
	
	// Remove the old attribute
	return RemoveAttr(oldname);
	
}


status_t
BNode::GetAttrInfo(const char *name, struct attr_info *info) const {
	return (fCStatus == B_NO_INIT) ? B_FILE_ERROR : StorageKit::stat_attr(fFd, name, info) ;
}

status_t
BNode::GetNextAttrName(char *buffer) {
	// We're allowed to assume buffer is at least
	// B_BUFFER_NAME_LENGTH chars long, but NULLs
	// are not acceptable.
	if (buffer == NULL)
		return B_BAD_VALUE;	// /new R5 crashed when passed NULL

	if (InitAttrDir() != B_OK)
		return B_ENTRY_NOT_FOUND;
		
	StorageKit::DirEntry *entry = StorageKit::read_attr_dir(fAttrDir);
	if (entry == NULL) {
		buffer[0] = 0;
		return B_ENTRY_NOT_FOUND;
	} else {
		strncpy(buffer, entry->d_name, B_ATTR_NAME_LENGTH);
		return B_OK;
	}
}

status_t
BNode::RewindAttrs() {
	if (InitAttrDir() != B_OK)
		return B_BAD_ADDRESS;	// This is what R5::BNode actually returns. Go figure...	
	
	StorageKit::rewind_attr_dir(fAttrDir);

	return B_OK;
}

// We need an OpenBeOS BString implementation before we can actually test
// WriteAttrString() and ReadAttrString().

status_t
BNode::WriteAttrString(const char *name, const BString *data) {
/*	return (data == NULL) ? B_BAD_VALUE :
		WriteAttr(name, B_STRING_TYPE, 0, data->String(), data->Length()); */
		
	return B_ERROR;
}

status_t
BNode::ReadAttrString(const char *name, BString *result) const {
/*	if (result == NULL)
		return B_BAD_VALUE;
	
	attr_info info;
	status_t r;
	
	r = GetAttrInfo(name, &info);
	if (r != B_OK)
		return r;
		
	char *data = result->LockBuffer(info.size+1);
		// Lock the string's buffer so we can meddle with it
	
	ssize_t bytes = ReadAttr(name, B_STRING_TYPE, 0, data, info.size);
		// Read the attribute
		
	// Check for failure
	if (bytes < 0) {
		r = bytes;
		bytes = 0;	// In this instance, we simply clear the string
	} else {
		r = B_OK;
	}
	
	data[bytes] = 0;
		// Null terminate the new string just to be sure (since it *is*
		// possible to read and write non-NULL-terminated strings)
		
	result->UnlockBuffer(bytes+1);
	
	return r; */
	
	return B_ERROR;
}

BNode&
BNode::operator=(const BNode &node) {
	return *this;
}

bool
BNode::operator==(const BNode &node) const {
	return false;
}

bool
BNode::operator!=(const BNode &node) const {
	return true;
}

int
BNode::Dup() {
	return StorageKit::dup(fFd);
}


void BNode::_RudeNode1() { }
void BNode::_RudeNode2() { }
void BNode::_RudeNode3() { }
void BNode::_RudeNode4() { }
void BNode::_RudeNode5() { }
void BNode::_RudeNode6() { }

status_t
BNode::set_fd(int fd) {
	if (fFd != -1)
		close_fd();
		
	fFd = fd;
}

void
BNode::close_fd() {
	if (fAttrDir != NULL) {
		StorageKit::close_attr_dir(fAttrDir);
		fAttrDir = NULL;
	}

	if (fFd != -1) {
		close(fFd);
		fFd = -1;
	}	
		
	fCStatus = B_NO_INIT;	
}

status_t
BNode::clear_virtual() {
	return B_ERROR;
}

status_t
BNode::clear() {
	return B_ERROR;
}

status_t
BNode::set_stat(struct stat &st, uint32 what) {
	if (fCStatus != B_OK)
		return B_FILE_ERROR;

	return StorageKit::set_stat(fFd, st, what);
}

status_t
BNode::set_to(const entry_ref *ref, bool traverse) {
	return B_ERROR;
}

status_t
BNode::set_to(const BEntry *entry, bool traverse) {
	return B_ERROR;
}

status_t
BNode::set_to(const char *path, bool traverse) {
	return B_ERROR;
}

status_t
BNode::set_to(const BDirectory *dir, const char *path, bool traverse) {
	return B_ERROR;
}

status_t
BNode::InitAttrDir() {
	if (fCStatus == B_OK && fAttrDir == NULL)
		fAttrDir = StorageKit::open_attr_dir(fFd);

	return fCStatus;	
}
