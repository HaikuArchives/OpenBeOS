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

#include <FindDirectory.h>
#include <NodeMonitor.h>
#include <Directory.h>
#include <NodeInfo.h>
#include <Entry.h>
#include <Path.h>
#include <Node.h>

// -----------------------------------------------------------------------------
int main()
{
	new PrintServer;
		be_app->Run();
	delete be_app;
	
	return 0;
}

// -----------------------------------------------------------------------------
PrintServer::PrintServer()
	: Inherited(B_PSRV_APP_SIGNATURE)
{
	ScanForPrinters();
	InstallNodeMonitor();
}

// -----------------------------------------------------------------------------
bool PrintServer::QuitRequested()
{
	bool quitingIsOk = Inherited::QuitRequested();
	if (quitingIsOk) {
		RemoveNodeMonitor();
	}

	return quitingIsOk;
}

// -----------------------------------------------------------------------------
void PrintServer::MessageReceived(BMessage* msg)
{
	int32 opcode;

	switch(msg->what) {
		case B_GET_PROPERTY:
		case B_SET_PROPERTY:
		case B_CREATE_PROPERTY:
		case B_DELETE_PROPERTY:
		case B_COUNT_PROPERTIES:
		case B_EXECUTE_PROPERTY:
			HandleScriptingCommand(msg);
			break;
		
		case B_NODE_MONITOR:		// node monitor notification
			if (msg->FindInt32("opcode", &opcode) == B_OK) {
					// we found a valid opcode
				switch(opcode) {
					case B_ENTRY_CREATED:	// an entry was created
						printf("PrintServer: B_ENTRY_CREATED\n");
						break;

					case B_ENTRY_REMOVED:
						printf("PrintServer: B_ENTRY_REMOVED\n");
						break;
					
					case B_ENTRY_MOVED:
						printf("PrintServer: B_ENTRY_MOVED\n");
						break;
						
					case B_STAT_CHANGED:
						printf("PrintServer: B_STAT_CHANGED\n");
						break;

					case B_ATTR_CHANGED:
						printf("PrintServer: B_ATTR_CHANGED\n");
						break;
				}
			}
			break;
			
		default:
			Inherited::MessageReceived(msg);
	}
}

// -----------------------------------------------------------------------------
status_t PrintServer::InstallNodeMonitor()
{
	status_t rc = B_OK;
	BPath path;
	
		// Find the ~/config/settings/printers/ directory
	if ((rc=find_directory(B_USER_PRINTERS_DIRECTORY, &path, true)) == B_OK) {
		BNode node(path.Path());
		node_ref nref;
		
		if ((rc=node.GetNodeRef(&nref)) == B_OK) {
			rc = watch_node(&nref, B_WATCH_DIRECTORY, this);
		}
	}
	
	return rc;
}

// -----------------------------------------------------------------------------
status_t PrintServer::RemoveNodeMonitor()
{
	status_t rc = B_OK;
	BPath path;
	
		// Find the ~/config/settings/printers/ directory
	if ((rc=find_directory(B_USER_PRINTERS_DIRECTORY, &path, true)) == B_OK) {
		BNode node(path.Path());
		node_ref nref;
		
		if ((rc=node.GetNodeRef(&nref)) == B_OK) {
			rc = watch_node(&nref, B_STOP_WATCHING, this);
		}
	}
	
	return rc;
}

// -----------------------------------------------------------------------------
status_t PrintServer::ScanForPrinters()
{
	status_t rc = B_OK;
	char mimetype[256];
	BEntry entry;
	BPath path;
	BNode node;
	
		// Find the ~/config/settings/printers/ directory
	if ((rc=find_directory(B_USER_PRINTERS_DIRECTORY, &path, true)) == B_OK) {
		BDirectory printersDir(path.Path());
		
			// Walk through all entries in this directory
		while((rc=printersDir.GetNextEntry(&entry)) == B_OK) {
			if ((rc=node.SetTo(&entry)) == B_OK) {
				BNodeInfo info(&node);
				
					// If this node has the right mimetype.....
				if ((rc=info.GetType(mimetype)) == B_OK &&
					::strcmp(mimetype, B_PSRV_PRINTER_FILETYPE) == 0) {
						// Create a Printer object from this node.
					Printer::CreateFrom(node);
				}
			}
			else
				break;
		}
		
			// If we ended the while-loop because no (more) entries were found,
			//	  return B_OK
		if (rc == B_ENTRY_NOT_FOUND)
			rc = B_OK;
	}
	
	return rc;
}
