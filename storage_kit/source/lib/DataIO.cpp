// Quick and dirty implementation of the BDataIO and BPositionIO classes.
// Just here to be able to compile and test BFile.
// To be replaced by the OpenBeOS version to be provided by the IK Team.

#include <stdio.h>

#include <DataIO.h>
#include <SupportDefs.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

// BDataIO

// constructor
BDataIO::BDataIO()
{
}

// disabled copy constructor
BDataIO::BDataIO(const BDataIO &)
{
}

// destructor
BDataIO::~BDataIO()
{
}

void BDataIO::_ReservedDataIO1() {}
void BDataIO::_ReservedDataIO2() {}
void BDataIO::_ReservedDataIO3() {}
void BDataIO::_ReservedDataIO4() {}

// disabled =
BDataIO &
BDataIO::operator=(const BDataIO &)
{
	return *this;
}


// BPositionIO

// constructor
BPositionIO::BPositionIO()
	: BDataIO()
{
}

// destructor
BPositionIO::~BPositionIO()
{
}

// Read
ssize_t
BPositionIO::Read(void *buffer, size_t size)
{
	ssize_t result = B_ERROR;
	off_t position = Position();
	if (position < 0)
		result = (ssize_t)position;
	else {
		result = ReadAt(position, buffer, size);
		if (result > 0) {
			// Seek to new position.
			off_t newPosition = Seek(position + result, SEEK_SET);
			if (newPosition < 0)
				result = (ssize_t)newPosition;
			else if (newPosition != position + result)
				result = B_ERROR;
		}
	}
	return result;
}

// Write
ssize_t
BPositionIO::Write(const void *buffer, size_t size)
{
	ssize_t result = B_ERROR;
	off_t position = Position();
	if (position < 0)
		result = (ssize_t)position;
	else {
		result = WriteAt(position, buffer, size);
		if (result > 0) {
			// Seek to new position.
			off_t newPosition = Seek(position + result, SEEK_SET);
			if (newPosition < 0)
				result = (ssize_t)newPosition;
			else if (newPosition != position + result)
				result = B_ERROR;
		}
	}
	return result;
}

// SetSize
status_t
BPositionIO::SetSize(off_t size)
{
	return B_ERROR;
}

void BPositionIO::_ReservedPositionIO1() {}
void BPositionIO::_ReservedPositionIO2() {}
void BPositionIO::_ReservedPositionIO3() {}
void BPositionIO::_ReservedPositionIO4() {}

#ifdef USE_OPENBEOS_NAMESPACE
}
#endif

