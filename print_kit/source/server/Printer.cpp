/*

Copyright (c) 2001 OpenBeOS. Written by I.R. Adema.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include "Printer.h"

#include "Driver.h"
#include "Transport.h"

#include "print_server.h"

#include <FindDirectory.h>
#include <Application.h>
#include <Directory.h>
#include <NodeInfo.h>
#include <Path.h>
#include <Node.h>

// -----------------------------------------------------------------------------
BObjectList<Printer> Printer::sPrinters;

// -----------------------------------------------------------------------------
status_t
Printer::Create(BMessage* params)
{
	status_t rc = B_OK;
	BString str;
	
		// Get name from archive and set it
	if (params->FindString("Name", &str) == B_OK)
		SetName(str.String());

		// Get selected transport
	if (params->FindString("Transport", &str) == B_OK)
		fTransport = Transport::Find(str.String());
		
		// Get printer comments
	if (params->FindString("Comments", &str) == B_OK)
		fComments = str;

		// Get selected driver
	if (params->FindString("Driver", &str) == B_OK)
		fDriver = Driver::Find(str.String());

	BPath path;
		// Find directory containing printer definitions
	if (find_directory(B_USER_PRINTERS_DIRECTORY,&path,true,NULL) == B_OK)
	{
		BDirectory printersDir(path.Path());
		BDirectory spoolDir;

			// Create printer spool directory
		if (printersDir.CreateDirectory(Name(), &spoolDir) == B_OK)
		{
			fSpoolDir = spoolDir;
			BString name(Name());
			spoolDir.WriteAttrString(B_PSRV_PRINTER_ATTR_DRV_NAME,  &fDriver->Name());
			spoolDir.WriteAttrString(B_PSRV_PRINTER_ATTR_PRT_NAME,  &name);
			spoolDir.WriteAttr      (B_PSRV_PRINTER_ATTR_STATE, 0,  B_STRING_TYPE, "free", 5);
			spoolDir.WriteAttrString(B_PSRV_PRINTER_ATTR_TRANSPORT, &fTransport->Name());
			spoolDir.WriteAttrString(B_PSRV_PRINTER_ATTR_COMMENTS,  &fComments);
			
			BNodeInfo info(&spoolDir);
			info.SetType(B_PSRV_PRINTER_FILETYPE);
		}
	}
	
	return rc;
}

// -----------------------------------------------------------------------------
status_t
Printer::Delete()
{
	status_t err = B_OK;
	node_ref noderef;
	BDirectory dir;
	BEntry entry;
	BPath path;
	
	if ((err=fSpoolDir.GetNodeRef(&noderef)) == B_OK &&
		(err=dir.SetTo(&noderef)) == B_OK &&
		(err=entry.SetTo(&dir,NULL)) == B_OK &&
		(err=entry.GetPath(&path)) == B_OK)
		::rmdir(path.Path());

	return err;
}

// -----------------------------------------------------------------------------
status_t
Printer::ReadSettingsFrom(const BNode& node)
{
	status_t rc = B_OK;
	BString s;

	if ((rc=node.ReadAttrString(B_PSRV_PRINTER_ATTR_PRT_NAME, &s)) != B_OK)
		return rc; // if name could not be read, bail out
	
	SetName(s.String()); // set new name
		
	if ((rc=node.ReadAttrString(B_PSRV_PRINTER_ATTR_DRV_NAME, &s)) != B_OK)
		return rc; // if driver name could not be read, bail out

	if ((fDriver=Driver::Find(s.String())) == NULL)
		return B_BAD_VALUE; // if driver cannot be found, bail out
		
	if ((rc=node.ReadAttrString(B_PSRV_PRINTER_ATTR_TRANSPORT, &s)) != B_OK)
		return rc; // if transport name could not be read, bail out

	if ((fTransport=Transport::Find(s.String())) == NULL)
		return B_BAD_VALUE; // if transport cannot be found, bail out

	node.ReadAttrString(B_PSRV_PRINTER_ATTR_COMMENTS,		&fComments);
	node.ReadAttrString(B_PSRV_PRINTER_ATTR_STATE,			&fState);
	node.ReadAttrString(B_PSRV_PRINTER_ATTR_TRANSPORT_ADDR,	&fTransportAddr);
	node.ReadAttrString(B_PSRV_PRINTER_ATTR_CNX,			&fConnection);	
/*
	node.ReadAttrString(B_PSRV_PRINTER_ATTR_PNP,			&fPNP);	
	node.ReadAttrString(B_PSRV_PRINTER_ATTR_MDL,			&fMDL);	
*/
	return rc;
}

// -----------------------------------------------------------------------------

#if 0
#pragma mark ----- Statics ------
#endif

// -----------------------------------------------------------------------------
Printer*
Printer::CreateFrom(const BNode& node)
{
	Printer* printer = new Printer;
	
		if (printer->ReadSettingsFrom(node) == B_OK)
		{
			sPrinters.AddItem(printer);
			be_app->AddHandler(printer);
		}
		else
		{
			delete printer;
			printer = NULL;
		}

	return printer;
}

// -----------------------------------------------------------------------------
uint32
Printer::CountPrinters()
{
	return sPrinters.CountItems();
}

// -----------------------------------------------------------------------------
Printer*
Printer::Find(const char* name)
{
	for (int32 idx=0; idx < sPrinters.CountItems(); idx++)
		if (strcmp(sPrinters.ItemAt(idx)->Name(), name) == 0)
			return sPrinters.ItemAt(idx);

	return NULL;
}

// -----------------------------------------------------------------------------
Printer*
Printer::At(uint32 index)
{
	return sPrinters.ItemAt(index);
}


// -----------------------------------------------------------------------------
void
Printer::Remove(Printer* printer)
{
	sPrinters.RemoveItem(printer);
}

// -----------------------------------------------------------------------------
void
Printer::MessageReceived(BMessage* msg)
{
	switch(msg->what) {
		case B_GET_PROPERTY:
		case B_SET_PROPERTY:
		case B_CREATE_PROPERTY:
		case B_DELETE_PROPERTY:
		case B_COUNT_PROPERTIES:
		case B_EXECUTE_PROPERTY:
			this->HandleScriptingCommand(msg);
			break;
	}
}
