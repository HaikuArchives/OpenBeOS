#include <Archivable.h>

#include "NetBuffer.h"

BNetBuffer::BNetBuffer(size_t size = 0) {
}

BNetBuffer::~BNetBuffer() {
}

status_t BNetBuffer::InitCheck() {
	return B_ERROR;
}

BNetBuffer::BNetBuffer(BMessage *archive) {
}

status_t BNetBuffer::Archive(BMessage *into, bool deep = true) const {
	return B_ERROR;
}

BArchivable *BNetBuffer::Instantiate(BMessage *archive) {
	return NULL;
}

BNetBuffer::BNetBuffer(const BNetBuffer &) {
}

BNetBuffer &BNetBuffer::operator=(const BNetBuffer &) {
	return *this;
}


status_t BNetBuffer::AppendInt8(int8) {
	return B_ERROR;
}

status_t BNetBuffer::AppendUint8(uint8) {
	return B_ERROR;
}

status_t BNetBuffer::AppendInt16(int16) {
	return B_ERROR;
}

status_t BNetBuffer::AppendUint16(uint16) {
	return B_ERROR;
}

status_t BNetBuffer::AppendInt32(int32) {
	return B_ERROR;
}

status_t BNetBuffer::AppendUint32(uint32) {
	return B_ERROR;
}

status_t BNetBuffer::AppendFloat(float) {
	return B_ERROR;
}

status_t BNetBuffer::AppendDouble(double) {
	return B_ERROR;
}

status_t BNetBuffer::AppendString(const char *) {
	return B_ERROR;
}

status_t BNetBuffer::AppendData(const void *, size_t) {
	return B_ERROR;
}

status_t BNetBuffer::AppendMessage(const BMessage &) {
	return B_ERROR;
}

status_t BNetBuffer::AppendInt64(int64) {
	return B_ERROR;
}

status_t BNetBuffer::AppendUint64(uint64) {
	return B_ERROR;
}

status_t BNetBuffer::RemoveInt8(int8 &) {
	return B_ERROR;
}

status_t BNetBuffer::RemoveUint8(uint8 &) {
	return B_ERROR;
}

status_t BNetBuffer::RemoveInt16(int16 &) {
	return B_ERROR;
}

status_t BNetBuffer::RemoveUint16(uint16 &) {
	return B_ERROR;
}

status_t BNetBuffer::RemoveInt32(int32 &) {
	return B_ERROR;
}

status_t BNetBuffer::RemoveUint32(uint32 &) {
	return B_ERROR;
}

status_t BNetBuffer::RemoveFloat(float &) {
	return B_ERROR;
}

status_t BNetBuffer::RemoveDouble(double &) {
	return B_ERROR;
}

status_t BNetBuffer::RemoveString(char *, size_t) {
	return B_ERROR;
}

status_t BNetBuffer::RemoveData(void *, size_t) {
	return B_ERROR;
}

status_t BNetBuffer::RemoveMessage(BMessage &) {
	return B_ERROR;
}

status_t BNetBuffer::RemoveInt64(int64 &) {
	return B_ERROR;
}

status_t BNetBuffer::RemoveUint64(uint64 &) {
	return B_ERROR;
}
 
unsigned char *BNetBuffer::Data() const {
	return NULL;
}

size_t BNetBuffer::Size() const {
	return 0;
}

size_t BNetBuffer::BytesRemaining() const {
	return 0;
}

void BNetBuffer::_ReservedBNetBufferFBCCruft1() {
}

void BNetBuffer::_ReservedBNetBufferFBCCruft2() {
}

void BNetBuffer::_ReservedBNetBufferFBCCruft3() {
}

void BNetBuffer::_ReservedBNetBufferFBCCruft4() {
}

void BNetBuffer::_ReservedBNetBufferFBCCruft5() {
}

void BNetBuffer::_ReservedBNetBufferFBCCruft6() {
}

