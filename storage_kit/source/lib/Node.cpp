//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Node.cpp
//---------------------------------------------------------------------

#include <Node.h>

BNode::BNode() {
}

BNode::BNode(const entry_ref *ref) {
}

BNode::BNode(const BEntry *entry) {
}

BNode::BNode(const char *path) {
}

BNode::BNode(const BDirectory *dir, const char *path) {
}

BNode::BNode(const BNode &node) {
}

BNode::~BNode() {
}

status_t
BNode::InitCheck() const {
	return B_ERROR;
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
	return B_ERROR;
}

status_t
BNode::SetTo(const BDirectory *dir, const char *path) {
	return B_ERROR;
}

void
BNode::Unset() {
}

status_t
BNode::Lock() {
	return B_ERROR;
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
	return 0;
}

ssize_t
BNode::ReadAttr(const char *attr, type_code type, off_t offset,
void *buffer, size_t len) {
	return 0;
}

status_t
BNode::RemoveAttr(const char *name) {
	return B_ERROR;
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
	return B_ERROR;
}

status_t
BNode::RewindAttrs() {
	return B_ERROR;
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
	return -1;
}


void BNode::_RudeNode1() { }
void BNode::_RudeNode2() { }
void BNode::_RudeNode3() { }
void BNode::_RudeNode4() { }
void BNode::_RudeNode5() { }
void BNode::_RudeNode6() { }

status_t
BNode::set_fd(int fd) {
	return B_ERROR;
}

void
BNode::close_fd() {
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

