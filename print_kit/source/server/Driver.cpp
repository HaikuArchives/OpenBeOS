/*****************************************************************************/
// Driver.cpp
//
// Version: 1.0.0 Development
//
// Implements the object representing a single printer driver. Only one of
// these objects is created per driver. It is called iso directly calling
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

#include "Driver.h"

#include <FindDirectory.h>

// -----------------------------------------------------------------------------

typedef BMessage* (*config_func_t)(BNode* spoolDir, BMessage* msg);
typedef char* (*add_printer_func_t)(const char* printer_name);
typedef BMessage* (*take_job_func_t)(BFile* spool_file, BNode* spool_dir, BMessage* msg);
typedef BMessage* (*defaults_func_t)();

// -----------------------------------------------------------------------------
BObjectList<Driver> Driver::sDrivers;

// -----------------------------------------------------------------------------
static status_t GetDriverPath(const BString& name, directory_which which, BPath& outPath)
{
	status_t err = B_OK;
	
	if ((err=find_directory(which, &outPath)) == B_OK &&
		(err=outPath.Append("Print")) == B_OK &&
		(err=outPath.Append(name.String())) == B_OK)
	{
		struct stat buf;
		err = stat(outPath.Path(), &buf);
	}
	
	return err;
}

// -----------------------------------------------------------------------------
Driver::Driver(const BPath& path)
	: fPath(path)
{
}

// -----------------------------------------------------------------------------
Driver* Driver::Find(const char* name)
{
	Driver* driver = NULL;

		// Bail out early if the driver is already found
	for (int32 idx=0; idx < sDrivers.CountItems(); idx++)
		if (sDrivers.ItemAt(idx)->Name() == BString(name))
			return sDrivers.ItemAt(idx);
	
		// Now try to locate the driver
	BPath driverPath;
	if (::GetDriverPath(name, B_USER_ADDONS_DIRECTORY, driverPath) != B_OK)
		if (::GetDriverPath(name, B_COMMON_ADDONS_DIRECTORY, driverPath) != B_OK)
			::GetDriverPath(name, B_BEOS_ADDONS_DIRECTORY, driverPath);

		// If we can load the driver...
	image_id imid = ::load_add_on(driverPath.Path());
	if (imid > 0) {
		::printf("Loaded addon %s\n", driverPath.Path());
		unload_add_on(imid);

			// Create the object, and store it in our list
		driver = new Driver(driverPath);
		sDrivers.AddItem(driver);
	}
	
	return driver;
}

// -----------------------------------------------------------------------------
status_t Driver::DoConfigPage(BNode* spoolDir, BMessage* msg)
{
	config_func_t config_page_func;
	status_t rc = B_OK;

	image_id imid = ::load_add_on(fPath.Path());
	if (imid > 0) {
		BMessage* settings;
		
		defaults_func_t func;
		if (::get_image_symbol(imid, "default_settings", B_SYMBOL_TYPE_TEXT, (void**)&func) == B_OK)
			settings = (*func)();
		else
			settings = msg;

		if (::get_image_symbol(imid, "config_page", B_SYMBOL_TYPE_TEXT, (void**)&config_page_func) == B_OK)
			(*config_page_func)(spoolDir,settings);

		::unload_add_on(imid);
	}
	else
		rc = imid;
	
	return rc;
}

// -----------------------------------------------------------------------------
status_t Driver::DoConfigJob(BNode* spoolDir, BMessage* msg)
{
	config_func_t config_page_func;
	status_t rc = B_OK;

	image_id imid = ::load_add_on(fPath.Path());
	if (imid > 0) {
		if (::get_image_symbol(imid, "config_job", B_SYMBOL_TYPE_TEXT, (void**)&config_page_func) == B_OK)
			(*config_page_func)(spoolDir,msg);

		::unload_add_on(imid);
	}
	else
		rc = imid;
	
	return rc;
}
