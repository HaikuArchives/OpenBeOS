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

status_t BPath::InitCheck() const {
	return B_ERROR;
}

status_t BPath::SetTo(const char *path, const char *leaf = NULL, bool normalize = false) {
	return B_ERROR;
}
	
status_t BPath::SetTo(const BDirectory *dir, const char *path, bool normalize = false) {
	return B_ERROR;
}
	
status_t BPath::SetTo(const BEntry *entry) {
	return B_ERROR;
}
	
status_t BPath::SetTo(const entry_ref *ref) {
	return B_ERROR;
}

