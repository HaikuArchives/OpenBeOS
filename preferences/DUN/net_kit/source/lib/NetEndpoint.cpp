#include <Archivable.h>

#include <NetAddress.h>
#include <NetBuffer.h>

#include "NetEndpoint.h"

BNetEndpoint::BNetEndpoint(int proto = SOCK_STREAM) {
}

BNetEndpoint::~BNetEndpoint() {
}

status_t BNetEndpoint::InitCheck() {
}

BNetEndpoint &BNetEndpoint::operator=(const BNetEndpoint &) {
}

BNetEndpoint::BNetEndpoint(const BNetEndpoint &) {
}

BNetEndpoint::BNetEndpoint(BMessage *archive) {
}

status_t BNetEndpoint::Archive(BMessage *into, bool deep = true) const {
}

BArchivable *BNetEndpoint::Instantiate(BMessage *archive) {
}

status_t BNetEndpoint::SetProtocol(int proto) {
}

int BNetEndpoint::SetOption(int32 opt, int32 lev, const void *data, unsigned int datasize) {
}

int BNetEndpoint::SetNonBlocking(bool on = true) {
}

int BNetEndpoint::SetReuseAddr(bool on = true) {
}

const BNetAddress &BNetEndpoint::LocalAddr() {
}

const BNetAddress &BNetEndpoint::RemoteAddr() {
}

int BNetEndpoint::Socket() const {
}

void BNetEndpoint::Close() {
}

status_t BNetEndpoint::Bind(const BNetAddress &addr) {
}

status_t BNetEndpoint::Bind(int port = 0) {
}

status_t BNetEndpoint::Connect(const BNetAddress &addr) {
}

status_t BNetEndpoint::Connect(const char *addr, int port) {
}

status_t BNetEndpoint::Listen(int backlog = 5) {
}

BNetEndpoint *BNetEndpoint::Accept(int32 timeout = -1) {
}

int BNetEndpoint::Error() const {
}

char *BNetEndpoint::ErrorStr() const {
}

int32 BNetEndpoint::Send(const void *buf, size_t size, int flags = 0) {
}

int32 BNetEndpoint::Send(BNetBuffer &pack, int flags = 0) {
}

int32 BNetEndpoint::SendTo(const void *buf, size_t size, const BNetAddress &to, int flags = 0) {
}

int32 BNetEndpoint::SendTo(BNetBuffer &pack, const BNetAddress &to, int flags = 0) {
}

void BNetEndpoint::SetTimeout(bigtime_t usec) {
}

int32 BNetEndpoint::Receive(void *buf, size_t size, int flags = 0) {
}

int32 BNetEndpoint::Receive(BNetBuffer &pack, size_t size, int flags = 0) {
}

int32 BNetEndpoint::ReceiveFrom(void *buf, size_t size, BNetAddress &from, int flags = 0) {
}

int32 BNetEndpoint::ReceiveFrom(BNetBuffer &pack, size_t size, BNetAddress &from, int flags = 0) {
}

bool BNetEndpoint::IsDataPending(bigtime_t usec_timeout = 0) {
}

void BNetEndpoint::_ReservedBNetEndpointFBCCruft1() {
}

void BNetEndpoint::_ReservedBNetEndpointFBCCruft2() {
}

void BNetEndpoint::_ReservedBNetEndpointFBCCruft3() {
}

void BNetEndpoint::_ReservedBNetEndpointFBCCruft4() {
}

void BNetEndpoint::_ReservedBNetEndpointFBCCruft5() {
}

void BNetEndpoint::_ReservedBNetEndpointFBCCruft6() {
}

