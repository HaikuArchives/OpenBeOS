//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Entry.cpp
//  Description:  A file location wrapper class
//---------------------------------------------------------------------
#include "Entry.h"

#ifdef USE_OPENBEOS_NAMESPACE
using namespace OpenBeOS;
#endif

BEntry::BEntry(const BDirectory *dir, const char *path, bool traverse = false){
};

BEntry::BEntry(const entry_ref *ref, bool traverse = false){
};

BEntry::BEntry(const char *path, bool traverse = false){
};

BEntry::BEntry(const BEntry &entry){
};

BEntry::~BEntry(){
};

status_t BEntry::InitCheck() const{
};

bool BEntry::Exists() const{
	return false;
};

status_t BEntry::GetStat(struct stat *st) const{
};

status_t BEntry::SetTo(const BDirectory *dir, const char *path, bool traverse = false){
};
				  
status_t BEntry::SetTo(const entry_ref *ref, bool traverse = false){
};

status_t BEntry::SetTo(const char *path, bool traverse = false){
};

void BEntry::Unset(){
};

status_t BEntry::GetRef(entry_ref *ref) const {
};

status_t BEntry::GetPath(BPath *path) const {
};

status_t BEntry::GetParent(BEntry *entry) const {
};

status_t BEntry::GetParent(BDirectory *dir) const {
};

status_t BEntry::GetName(char *buffer) const {
};

status_t BEntry::Rename(const char *path, bool clobber = false) {
};

status_t BEntry::MoveTo(BDirectory *dir, const char *path = NULL, bool clobber = false) {
};

status_t BEntry::Remove() {
};

bool		BEntry::operator==(const BEntry &item) const {
};

bool		BEntry::operator!=(const BEntry &item) const {
};

BEntry &	BEntry::operator=(const BEntry &item) {
};

// FBC 
void BEntry::_PennyEntry1(){};
void BEntry::_PennyEntry2(){};
void BEntry::_PennyEntry3(){};
void BEntry::_PennyEntry4(){};
void BEntry::_PennyEntry5(){};
void BEntry::_PennyEntry6(){};

status_t BEntry::set_stat(struct stat &st, uint32 what){
};

status_t BEntry::move(int fd, const char *path){
};

status_t BEntry::set(int fd, const char *path, bool traverse){
};

status_t BEntry::clear(){
};