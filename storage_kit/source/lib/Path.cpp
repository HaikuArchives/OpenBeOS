//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Path.cpp
//  Description: An absolute pathname wrapper class
//---------------------------------------------------------------------
#include "Path.h"

#ifdef USE_OPENBEOS_NAMESPACE
using namespace OpenBeOS;
#endif



BPath::BPath() {
}
	
BPath::BPath(const char *dir, const char *leaf = NULL, bool normalize = false) {
}
	
BPath::BPath(const BDirectory *dir, const char *leaf, bool normalize = false) {
}

BPath::BPath(const BPath &path) {
}

BPath::BPath(const BEntry *entry) {
}

BPath::BPath(const entry_ref *ref) {
}

BPath::~BPath() {
}

/*

status_t BPath::InitCheck() const {
}

status_t BPath::SetTo(const char *path, const char *leaf = NULL, bool normalize = false) {
}
	
status_t BPath::SetTo(const BDirectory *dir, const char *path, bool normalize = false) {
}
	
status_t BPath::SetTo(const BEntry *entry) {
}
	
status_t BPath::SetTo(const entry_ref *ref) {
}

status_t BPath::Append(const char *path, bool normalize = false) {
}
	
void BPath::Unset() {
}
	
const char *BPath::Path() const {
}
	
const char *BPath::Leaf() const {
}

status_t BPath::GetParent(BPath *) const {
}
	
bool BPath::operator==(const BPath &item) const {
}

bool BPath::operator==(const char *path) const {
}

bool BPath::operator!=(const BPath &item) const {
}

bool BPath::operator!=(const char *path) const {
}
	

BPath& BPath::operator=(const BPath &item) {
}

BPath& BPath::operator=(const char *path) {
}

	
bool BPath::IsFixedSize() const {
	return false;
}
	
type_code BPath::TypeCode() const {
	return B_REF_TYPE;
}
	
ssize_t BPath::FlattenedSize() const {
	return -1;
}
	
status_t BPath::Flatten(void *buffer, ssize_t size) const {
	return B_ERROR;
}
	
bool BPath::AllowsTypeCode(type_code code) const {
	return (code == B_REF_TYPE);
}
	
status_t BPath::Unflatten(type_code c, const void *buf, ssize_t size) {
	return B_ERROR;
}

*/
