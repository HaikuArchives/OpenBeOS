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

#ifndef _PRINTSERVER_H
#define _PRINTSERVER_H

class PrintServerApp;

// -----------------------------------------------------------------------------
#include <Application.h>
#include "print_server.h"

// -----------------------------------------------------------------------------
class Printer;
class PrintServer : public BApplication
{
	typedef BApplication Inherited;
public:
	PrintServer();
	bool QuitRequested();
	void MessageReceived(BMessage* msg);	
	void HandleScriptingCommand(BMessage* msg);
		
		// PrintServer.Scripting.cpp
	status_t GetSupportedSuites(BMessage *message);
	Printer* GetPrinterFromSpecifier(int32 action, BMessage* specifier);
	BHandler* ResolveSpecifier(BMessage* message, int32 index, BMessage* specifier,
								int32 what, const char *property);
private:
	Printer* fDefaultPrinter;

	status_t ScanForPrinters();
	status_t RetrieveDefaultPrinter();
	status_t StoreDefaultPrinter();
	status_t InstallNodeMonitor();
	status_t RemoveNodeMonitor();
};
// -----------------------------------------------------------------------------

#endif
