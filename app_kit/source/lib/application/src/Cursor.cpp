// BCursor
// by Frans van Nispen (xlr8@tref.nl)
//
// As BeOS only supports 16x16 monochrome cursors, and I would like
// to see a nice shadowes one, we will need to extend this one.
#include <stdio.h>
#include "app/Cursor.h"

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

BCursor::BCursor(const void *cursorData)
{
//	uint8 data[68];
//	memcpy(&data, cursorData, 68);
}

// undefined on BeOS
BCursor::BCursor(BMessage *data)
{
}

BCursor::~BCursor()
{
}

// not implemented on BeOS
status_t BCursor::Archive(BMessage *into, bool deep = true) const
{
	return B_OK;
}

// not implemented on BeOS
BArchivable	*BCursor::Instantiate(BMessage *data)
{
	return NULL;
}

status_t BCursor::Perform(perform_code d, void *arg)
{
	printf("perform %d\n", (int)d);
	return B_OK;
}

void BCursor::_ReservedCursor1()	{}
void BCursor::_ReservedCursor2()	{}
void BCursor::_ReservedCursor3()	{}
void BCursor::_ReservedCursor4()	{}

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
#endif