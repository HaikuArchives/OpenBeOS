// Very, very basic BString implementation -- neither binary compatible nor
// complete. Just here to allow for a complete BNode implementation.
// To be replaced by the OpenBeOS version to be provided by the IK Team.

#include <stdio.h>
#include <string.h>

#include "String.h"

// constructor
BString::BString()
	   : fData(NULL)
{
	_Init();
}

// constructor
BString::BString(const char *str)
	   : fData(NULL)
{
	SetTo(str);
}

// constructor
BString::BString(const char *str, int32 charCount)
	   : fData(NULL)
{
	SetTo(str, charCount);
}

// copy constructor
BString::BString(const BString &str)
	   : fData(NULL)
{
	*this = str;
}

// destructor
BString::~BString()
{
	delete[] fData;
}

// SetTo
BString &
BString::SetTo(const char *str)
{
	_Init(str);
	return *this;
}

// SetTo
BString &
BString::SetTo(const char *str, int32 charCount)
{
	_Init(str, charCount);
	return *this;
}

// SetTo
BString &
BString::SetTo(const BString &str)
{
	return (*this = str);
}

// String
const char *
BString::String() const
{
	return fData;
}

// Length
int32
BString::Length() const
{
	return strlen(fData);
}

// LockBuffer
char *
BString::LockBuffer(int32 maxLength)
{
	int32 len = Length();
	if (maxLength > len)
		_Init(fData, maxLength);
	return fData;
}

// UnlockBuffer
BString &
BString::UnlockBuffer(int32 length)
{
	if (length < 0)
		length = 0;
	fData[length] = 0;	// null terminate
}

// =
BString &
BString::operator=(const BString& str)
{
	_Init(str.String());
	return *this;
}

// ==
bool
BString::operator==(const BString& str)
{
	return (strcmp(fData, str.fData) == 0);
}

// !=
bool
BString::operator!=(const BString& str)
{
	return !(*this == str);
}

// _Init
//
// len -- length of the maximal string that shall fit into the new allocation,
//		  if < 0, strlen(str) is used instead
void
BString::_Init(const char *str, int32 len)
{
//printf("BString::_Init(%p, %ld)\n", str, len);
	// copy the supplied string
	char *newData = NULL;
	if (!str)
		str = "";
	if (len < 0)
		len = strlen(str);
	newData = new char[len + 1];
	strncpy(newData, str, len);
	newData[len] = 0;
	// delete the old data and set the new
	delete[] fData;
	fData = newData;
//printf("BString::_Init() done\n");
}

