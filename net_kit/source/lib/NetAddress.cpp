#include <Archivable.h>

#include "NetAddress.h"

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

BNetAddress::BNetAddress(BMessage *archive) {
}

BNetAddress::~BNetAddress() {
}

status_t BNetAddress::Archive(BMessage *into, bool deep = true) const {
	return B_UNSUPPORTED;
}

BArchivable *BNetAddress::Instantiate(BMessage *archive) {
	return NULL;
}
 
BNetAddress::BNetAddress(const char *hostname = 0, unsigned short port = 0) {
}

BNetAddress::BNetAddress(const struct sockaddr_in &sa) {
}

BNetAddress::BNetAddress(in_addr addr, int port = 0) {
}

BNetAddress::BNetAddress(uint32 addr, int port = 0 ) {
}

BNetAddress::BNetAddress(const BNetAddress &) {
}

BNetAddress::BNetAddress(const char *hostname, const char *protocol, const char *service) {
}

BNetAddress &BNetAddress::operator=(const BNetAddress &) {
	return *this;
}

status_t BNetAddress::InitCheck() {
	return B_ERROR;
}

status_t BNetAddress::SetTo(const char *hostname, const char *protocol, const char *service) {
	return B_ERROR;
}

status_t BNetAddress::SetTo(const char *hostname = 0, unsigned short port = 0) {
	return B_ERROR;
}

status_t BNetAddress::SetTo(const struct sockaddr_in &sa) {
	return B_ERROR;
}

status_t BNetAddress::SetTo(in_addr addr, int port = 0) {
	return B_ERROR;
}

status_t BNetAddress::SetTo(uint32 addr=INADDR_ANY, int port = 0) {
	return B_ERROR;
}

status_t BNetAddress::GetAddr(char *hostname = 0, unsigned short *port = 0) const {
	return B_ERROR;
}

status_t BNetAddress::GetAddr(struct sockaddr_in &sa) const {
	return B_ERROR;
}

status_t BNetAddress::GetAddr(in_addr &addr, unsigned short *port = 0) const {
	return B_ERROR;
}

void BNetAddress::_ReservedBNetAddressFBCCruft1() {
}

void BNetAddress::_ReservedBNetAddressFBCCruft2() {
}

void BNetAddress::_ReservedBNetAddressFBCCruft3() {
}

void BNetAddress::_ReservedBNetAddressFBCCruft4() {
}

void BNetAddress::_ReservedBNetAddressFBCCruft5() {
}

void BNetAddress::_ReservedBNetAddressFBCCruft6() {
}

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
#endif
