/*	BBitmapStream.cpp
 */

#include "BitmapStream.h"
#include <Bitmap.h>
#define DEBUG 1
#include <Debug.h>
#include <string.h>


BBitmapStream::BBitmapStream(
	BBitmap *		bitmap)
{
	fMap = bitmap;
	fDetached = false;
	fPosition = 0;
	fSize = 0;
	/*	Extract header if needed */
	if (fMap) {
		fHeader.magic = B_TRANSLATOR_BITMAP;
		fHeader.bounds = fMap->Bounds();
		fHeader.rowBytes = fMap->BytesPerRow();
		fHeader.colors = fMap->ColorSpace();
		fHeader.dataSize = uint32( (fHeader.bounds.Height()+1)*fHeader.rowBytes );
		fSize = sizeof(BBitmap)+fHeader.dataSize;
	}
}


BBitmapStream::~BBitmapStream()
{
	if (fMap && !fDetached)
		delete fMap;
}


status_t
BBitmapStream::ReadAt(
	off_t			pos,
	void *			buffer,
	size_t			size)
{
	if (!fMap)
		return B_ERROR;
	if (!size)
		return B_NO_ERROR;
	if (pos >= fSize)
		return B_ERROR;

	long toRead;
	void *source;

	if (fPosition < sizeof(BBitmap)) {
		toRead = sizeof(BBitmap)-pos;
		source = ((char *)&fHeader)+pos;
	} else {
		toRead = fSize-pos;
		source = ((char *)fMap->Bits())+fPosition-sizeof(BBitmap);
	}
	if (toRead > size)
		toRead = size;
	memcpy(buffer, source, toRead);
	return toRead;
}


status_t
BBitmapStream::WriteAt(
	off_t			pos,
	const void *	data,
	size_t			size)
{
	if (!size)
		return B_NO_ERROR;
	ssize_t written = 0;
	while (size > 0) {
		long toWrite;
		void *dest;
		/*	We depend on writing the header separately in detecting changes to it */
		if (pos < sizeof(BBitmap)) {
			toWrite = sizeof(BBitmap)-pos;
			dest = ((char *)&fHeader)+pos;
		} else {
			toWrite = fHeader.dataSize-pos+sizeof(BBitmap);
			dest = ((char *)fMap->Bits())+pos-sizeof(BBitmap);
		}
		if (toWrite > size)
			toWrite = size;
		if (!toWrite && size)	//	i e we've been told to write too much
			return B_BAD_VALUE;
		memcpy(dest, data, toWrite);
		pos += toWrite;
		written += toWrite;
		data = ((char *)data)+toWrite;
		size -= toWrite;
		if (pos > fSize)
			fSize = pos;
		/*	If we change the header, the rest goes */
		if (pos == sizeof(BBitmap)) {
			if (fMap && ((fMap->Bounds() != fHeader.bounds) ||
					(fMap->ColorSpace() != fHeader.colors) ||
					(fMap->BytesPerRow() != fHeader.rowBytes))) {
				if (!fDetached)	//	if someone detached, we don't delete
					delete fMap;
				fMap = NULL;
			}
			if (!fMap) {
				fHeader.bounds.PrintToStream();
				if ((fHeader.bounds.left > 0.0) || (fHeader.bounds.top > 0.0))
					DEBUGGER("non-origin bounds!");
				fMap = new BBitmap(fHeader.bounds, fHeader.colors);
				if (fMap->BytesPerRow() != fHeader.rowBytes) {
					return B_MISMATCHED_VALUES;
				}
			}
			if (fMap) {
				fSize = sizeof(BBitmap)+fMap->BitsLength();
			}
		}
	}
	return written;
}


off_t
BBitmapStream::Seek(	//	returns 0 for success
	off_t			position,
	uint32			whence)
{
	if (whence == SEEK_CUR)
		position += fPosition;
	if (whence == SEEK_END)
		position += fSize;
	if (position < 0)
		return B_BAD_VALUE;
	if (position > fSize)
		return B_BAD_VALUE;
	fPosition = position;
	return fPosition;
}


off_t
BBitmapStream::Position() const
{
	return fPosition;
}


off_t
BBitmapStream::Size() const
{
	return fSize;
}


status_t
BBitmapStream::SetSize(	//	returns 0 for success
	off_t			size)
{
	if (size < 0)
		return B_BAD_VALUE;
	if (fMap && (size > fHeader.dataSize+sizeof(BBitmap)))
		return B_BAD_VALUE;
	/*	Problem:
	 *	What if someone calls SetSize() before writing the header, so we don't know what 
	 *	bitmap to create?
	 *	Solution:
	 *	We assume people will write the header before any data, 
	 *	so SetSize() is really not going to do anything.
	 */
	if (fMap)	//	if we checked that the size was OK
		fSize = size;
	return B_NO_ERROR;
}


status_t
BBitmapStream::DetachBitmap(
	BBitmap * *	outBitmap)
{
	outBitmap = NULL;
	if (!fMap)
		return B_BAD_VALUE;
	if (fDetached)
		return B_ERROR;
	fDetached = true;
	*outBitmap = fMap;
	return B_NO_ERROR;
}

