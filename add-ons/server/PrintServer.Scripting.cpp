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

#include "PrintServer.h"
#include "Printer.h"

#include <PropertyInfo.h>

static property_info prop_list[] = {
	{
		"Printer",
		{ B_GET_PROPERTY, B_SET_PROPERTY, B_EXECUTE_PROPERTY },
		{ B_NAME_SPECIFIER, B_INDEX_SPECIFIER, B_REVERSE_INDEX_SPECIFIER },
		"Find the specified printer",
		0, // extra data
		//uint32		types[10];
		//compound_type	ctypes[3];
		//uint32		_reserved[10];
	},{
		"Printer",
		{ B_CREATE_PROPERTY },
		{ B_DIRECT_SPECIFIER },
		"Create a new printer",
		0,
		{ 0 },
		{ { {
			{ "Name", B_STRING_TYPE },
			{ "Driver", B_STRING_TYPE },
			{ "Transport", B_STRING_TYPE }
		} } }
	},{
		"Printers",
		{ B_COUNT_PROPERTIES },
		{ B_DIRECT_SPECIFIER },
		"Return the number of printers",
		0,
		//uint32		types[10];
		//compound_type	ctypes[3];
		//uint32		_reserved[10];
	},{
		"SelectedPrinter",
		{ B_GET_PROPERTY },
		{ B_DIRECT_SPECIFIER },
		"Return currently selected printer",
		0,
		//uint32		types[10];
		//compound_type	ctypes[3];
		//uint32		_reserved[10];
	}
};

// -----------------------------------------------------------------------------
void
PrintServer::HandleScriptingCommand(BMessage* msg)
{
	switch(msg->what) {
		case B_GET_PROPERTY:
		case B_SET_PROPERTY:
		case B_CREATE_PROPERTY:
		case B_DELETE_PROPERTY:
		case B_COUNT_PROPERTIES:
		case B_EXECUTE_PROPERTY:
			break;
	}
}

// -----------------------------------------------------------------------------
Printer*
PrintServer::GetPrinterFromSpecifier(int32 action, BMessage* specifier)
{
	Printer* printer = NULL;
	BString name;
	int32 idx;
	
	switch (action)
	{
		case B_INDEX_SPECIFIER:
			if (specifier->FindInt32("index", &idx) == B_OK)
				printer = Printer::At(idx);
			break;
	
		case B_REVERSE_INDEX_SPECIFIER:
			if (specifier->FindInt32("index", &idx) == B_OK)
				printer = Printer::At((Printer::CountPrinters()-1) - idx);
			break;
	
		case B_NAME_SPECIFIER:
			if (specifier->FindString("name", &name) == B_OK)
				printer = Printer::Find(name.String());
			break;
	}
	
	return printer;
}

// -----------------------------------------------------------------------------
BHandler*
PrintServer::ResolveSpecifier(BMessage *message, int32 index,
				BMessage *specifier, int32 what, const char *property)
{
	BPropertyInfo prop_info(prop_list); 
	Printer* printer;

	switch(prop_info.FindMatch(message, 0, specifier, what, property)) {
		case B_ERROR:	// Not found!
			break;

		case 0:			// Return Printer handler
			{
				message->PopSpecifier();
				printer = this->GetPrinterFromSpecifier(what,specifier);
				
				if (printer != NULL && message->what == B_DELETE_PROPERTY)
				{
					Printer::Remove(printer);
					printer->Delete();
					delete printer;

					BMessage reply(B_REPLY);
					reply.AddInt32("error", B_NO_ERROR);
					message->SendReply(&reply);
				}
				else
					return printer;
			}
			break;

		case 1:			// Create new Printer and return it
			{
				Printer* printerConfig = new Printer;
				printerConfig->Create(message);

				BMessage reply(B_REPLY);
				reply.AddInt32("error", B_NO_ERROR);
				message->SendReply(&reply);
			}
			break;

		default:			// Return this!
			return this;
	}

	return Inherited::ResolveSpecifier(message, index, specifier, what, property); 
}
 
// -----------------------------------------------------------------------------
status_t
PrintServer::GetSupportedSuites(BMessage *message)
{
	message->AddString("suites", B_PSRV_PRINTSERVER_SUITE); 
	
	BPropertyInfo prop_info(prop_list); 
	message->AddFlat("messages", &prop_info); 
	return BHandler::GetSupportedSuites(message);
}
