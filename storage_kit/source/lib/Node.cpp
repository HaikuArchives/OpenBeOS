//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Node.cpp
//---------------------------------------------------------------------

#include <Node.h>
#include <errno.h>
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
	return B_ERROR;
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

		set_fd(StorageKit::open(path, StorageKit::READ));
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
	return B_ERROR;
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
	if (fCStatus == B_NO_INIT)
		return B_FILE_ERROR;
	else
		return StorageKit::remove_attr(fFd, name);
}


status_t
BNode::RenameAttr(const char *oldname, const char *newname) {
	return B_ERROR;
}


status_t
BNode::GetAttrInfo(const char *name, struct attr_info *info) {
	return B_ERROR;
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

status_t
BNode::WriteAttrString(const char *name, const BString *data) {
	return B_ERROR;
}

status_t
BNode::ReadAttrString(const char *name, BString *result) const {
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
	return B_ERROR;
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

/*
status_t
BNode::InitAttrDir() {
	// Make sure we have an attr file descriptor
	if (InitAttrFd() != B_OK)
		return B_FILE_ERROR;
		
	// Open the attr directory if necessary
	if (fAttrDir == NULL) 
		fAttrDir = StorageKit::open_attr_dir(fAttrFd);
		
	return (fAttrDir != NULL) ? B_OK : B_FILE_ERROR;		
}
*/

