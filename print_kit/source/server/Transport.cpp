/*****************************************************************************/
// Transport.cpp
//
// Version: 1.0.0 Development
//
// Implements the object representing a single transport addon. Only one of
// these objects is created per addon. It is called iso directly calling
// image.h calls.
//
//
// This application and all source files used in its construction, except 
// where noted, are licensed under the MIT License, and have been written 
// and are:
//
// Copyright (c) 2001 OpenBeOS Project
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included 
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
/*****************************************************************************/

#include "Transport.h"

#include <FindDirectory.h>

// -----------------------------------------------------------------------------
BObjectList<Transport> Transport::sTransports;

// -----------------------------------------------------------------------------
static status_t GetAddonPath(const BString& name, directory_which which, BPath& outPath)
{
	status_t err = B_OK;
	
	if ((err=find_directory(which, &outPath)) == B_OK &&
		(err=outPath.Append("Print/transport")) == B_OK &&
		(err=outPath.Append(name.String())) == B_OK)
	{
		struct stat buf;
		err = stat(outPath.Path(), &buf);
	}
	
	return err;
}

// -----------------------------------------------------------------------------
Transport::Transport(const BPath& path)
	: fPath(path)
{
}

// -----------------------------------------------------------------------------
Transport* Transport::Find(const char* name)
{
	Transport* transport = NULL;

		// Bail out early if the addon is already found
	for (int32 idx=0; idx < sTransports.CountItems(); idx++)
		if (sTransports.ItemAt(idx)->Name() == BString(name))
			return sTransports.ItemAt(idx);

		// Now try to locate the addon
	BPath addonPath;
	if (::GetAddonPath(name, B_USER_ADDONS_DIRECTORY, addonPath) != B_OK)
		if (::GetAddonPath(name, B_COMMON_ADDONS_DIRECTORY, addonPath) != B_OK)
			::GetAddonPath(name, B_BEOS_ADDONS_DIRECTORY, addonPath);

		// If we can load the addon...
	image_id imid = ::load_add_on(addonPath.Path());
	if (imid > 0) {
		::printf("Loaded addon %s\n", addonPath.Path());
		unload_add_on(imid);

			// Create the object, and store it in our list
		transport = new Transport(addonPath);
		sTransports.AddItem(transport);
	}
	
	return transport;
}
