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

#ifndef _PRINTER_H
#define _PRINTER_H

class Printer;

// -----------------------------------------------------------------------------

#include <Handler.h>
#include <String.h>
#include <Node.h>

#include "ObjectList.h"

// -----------------------------------------------------------------------------

class Driver;
class Transport;

// -----------------------------------------------------------------------------
class Printer : public BHandler
{
	typedef BHandler Inherited;
public:
	status_t Create(BMessage* params);
	status_t Delete();

		// Printer.Scripting.cpp
	void MessageReceived(BMessage* msg);
	status_t GetSupportedSuites(BMessage* msg);
	void HandleScriptingCommand(BMessage* msg);
	BHandler* ResolveSpecifier(BMessage* msg, int32 index,
						BMessage* spec, int32 form, const char* prop);
						
		// The following static methods are the only way
		//     to create a new Printer object. 
	static Printer* CreateFrom(const BNode& node);
	
	static uint32 CountPrinters();
	static Printer* Find(const char* name);
	static Printer* At(uint32 index);
	static void Remove(Printer* printer);

private:
	status_t ReadSettingsFrom(const BNode& node);

	Transport*	fTransport;
	Driver*		fDriver;
	BNode		fSpoolDir;
	BString		fTransportAddr;
	BString		fConnection;
	BString 	fComments;
	BString		fState;

	static BObjectList<Printer> sPrinters;
};
// -----------------------------------------------------------------------------

#endif
