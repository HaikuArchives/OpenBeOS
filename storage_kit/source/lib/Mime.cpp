//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//---------------------------------------------------------------------
/*!
	\file Mime.cpp
	Mime type C functions implementation.
*/

#include <Mime.h>

enum {
	NOT_IMPLEMENTED	= B_ERROR,
};

// update_mime_info
int
update_mime_info(const char *path, int recursive, int synchronous, int force)
{
	return NOT_IMPLEMENTED;
}

// create_app_meta_mime
status_t
create_app_meta_mime(const char *path, int recursive, int synchronous,
					 int force)
{
	return NOT_IMPLEMENTED;
}

// get_device_icon
status_t
get_device_icon(const char *dev, void *icon, int32 size)
{
	return NOT_IMPLEMENTED;
}

