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

#include "Transport.h"
#include "Driver.h"

#include "print_server.h"

#include <PropertyInfo.h>
#include <Message.h>

static property_info prop_list[] = { 
	{ "Name", {B_GET_PROPERTY, 0}, {B_DIRECT_SPECIFIER, 0}, "Get Name of Printer" }, 
	{ "DriverName", {B_GET_PROPERTY, 0}, {B_DIRECT_SPECIFIER, 0}, "Get Name of Printer Driver" }, 
	{ "TransportName", {B_GET_PROPERTY, 0}, {B_DIRECT_SPECIFIER, 0}, "Get Transport driver name" },
	{ "Comments", {B_GET_PROPERTY, 0}, {B_DIRECT_SPECIFIER, 0}, "Get Comments about Printer" },
	{ "Status", {B_GET_PROPERTY, 0}, {B_DIRECT_SPECIFIER, 0}, "Get Printer Status" }, 
	{ "Job", {B_GET_PROPERTY, 0}, {B_INDEX_SPECIFIER, 0}, "Get Spooled Printer Jobs" }, 
	{ "Job", {B_COUNT_PROPERTIES, 0}, {B_DIRECT_SPECIFIER, 0}, "Get Number Of Spooled Jobs" }, 
	
	{ "PageSetup", {B_EXECUTE_PROPERTY, 0}, {B_DIRECT_SPECIFIER, 0}, "Show Page Setup Dialog" },
	{ "JobSetup", {B_EXECUTE_PROPERTY, 0}, {B_DIRECT_SPECIFIER, 0}, "Show Job Setup Dialog" },

	{0}	// terminate list 
}; 

// -----------------------------------------------------------------------------
void Printer::HandleScriptingCommand(BMessage* msg)
{
	switch(msg->what)
	{
		case B_EXECUTE_PROPERTY:
			{
				BString propName;
				status_t error = B_OK;
				
				BMessage spec;
				int32 idx;
				msg->GetCurrentSpecifier(&idx,&spec);

				if (spec.FindString("property", &propName) == B_OK)
				{
					BMessage reply(B_REPLY);
/*					
					if (propName == "PageSetup")
						fDriver->DoConfigPage(&fSpoolDir, &reply);
					else if (propName == "JobSetup")
						fDriver->DoConfigJob(&fSpoolDir, &reply);
					else
*/						error = B_BAD_SCRIPT_SYNTAX;
						
					reply.AddInt32("error", error);
					
					msg->SendReply(&reply);
				}
				else	// probably empty specifier
						// (No Printer attrib specified)
					Inherited::MessageReceived(msg);
			}
			break;

		case B_GET_PROPERTY:
			{
				BString propName;
				status_t error = B_OK;
				
				BMessage spec;
				int32 idx;
				msg->GetCurrentSpecifier(&idx,&spec);

				if (spec.FindString("property", &propName) == B_OK)
				{
					BMessage reply(B_REPLY);
					
					if (propName == "Name")
						reply.AddString("result", Name());
					else if (propName == "DriverName")
						reply.AddString("result", fDriver->Name());
					else if (propName == "TransportName")
						reply.AddString("result", fTransport->Name());
					else if (propName == "Comments")
						reply.AddString("result", fComments.String());
					else
						error = B_BAD_SCRIPT_SYNTAX;

					reply.AddInt32("error", error);

					msg->SendReply(&reply);
				}
				else	// probably empty specifier
						// (No PrinterConfig attrib specified)
					Inherited::MessageReceived(msg);
			}
			break;
		default:
			Inherited::MessageReceived(msg);
	}
}

// -----------------------------------------------------------------------------
BHandler*
Printer::ResolveSpecifier(BMessage* msg, int32 index,
						BMessage* spec, int32 form, const char* prop)
{
	BPropertyInfo prop_info(prop_list); 

	switch ( prop_info.FindMatch(msg, 0/*index*/, spec, form, prop) )
	{
		case B_ERROR:	// Not found!
			break;
		
		default:
			return this;
	}

	return Inherited::ResolveSpecifier(msg, index, spec, form, prop); 
}

// -----------------------------------------------------------------------------
status_t
Printer::GetSupportedSuites(BMessage* msg)
{
	BPropertyInfo prop_info(prop_list);
	
	msg->AddString("suites", B_PSRV_PRINTER_SUITE);	
	msg->AddFlat("messages", &prop_info); 
	
	return Inherited::GetSupportedSuites(msg); 
}
