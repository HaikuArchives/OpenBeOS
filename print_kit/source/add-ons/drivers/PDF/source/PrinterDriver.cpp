/*

PDF Writer printer driver.

Copyright (c) 2001 OpenBeOS. 

Authors: 
	Philippe Houdoin
	Michael Pfeiffer
	
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


#include <stdio.h>
#include <string.h>			// for memset()

#include <StorageKit.h>

#include "PrinterDriver.h"

#include "PrinterSetupWindow.h"
#include "PageSetupWindow.h"
#include "JobSetupWindow.h"
#include "StatusWindow.h"

// Private prototypes
// ------------------

#ifdef CODEWARRIOR
	#pragma mark [Constructor & destructor]
#endif

// Constructor & destructor
// ------------------------

PrinterDriver::PrinterDriver()
	:	fJobFile(NULL),
		fPrinterNode(NULL),
		fJobMsg(NULL),

		fTransport(NULL),
		fTransportAddOn(-1),
		fTransportInitProc(NULL),
		fTransportExitProc(NULL)		
{
}

PrinterDriver::~PrinterDriver()
{
}

#ifdef CODEWARRIOR
	#pragma mark [Public methods]
#endif

// Public methods
// --------------

status_t 
PrinterDriver::PrintJob
	(
	BFile 		*jobFile,		// spool file
	BNode 		*printerNode,	// printer node, used by OpenTransport() to find & load transport add-on
	BMessage 	*jobMsg			// job message
	)
{
	print_file_header	pfh;
	status_t			status;
	BMessage 			*msg;
	int32 				page;
	uint32				copy;
	uint32				copies;

	fJobFile		= jobFile;
	fPrinterNode	= printerNode;
	fJobMsg			= jobMsg;

	if (!fJobFile || !fPrinterNode) 
		return B_ERROR;

	// open transport
	if (OpenTransport() != B_OK) 
		return B_ERROR;

	// show status window
	fStatusWindow = new StatusWindow("", BRect(40, 40, 200, 54));

	// read print file header	
	fJobFile->Seek(0, SEEK_SET);
	fJobFile->Read(&pfh, sizeof(pfh));
	
	// read job message
	fJobMsg = msg = new BMessage();
	msg->Unflatten(fJobFile);
	
	if (msg->HasInt32("copies")) 
		copies = msg->FindInt32("copies");
	else
		copies = 1;
	
	status = B_OK;
	for (copy = 0; copy < copies; copy++) {
		for (page = 1; page <= pfh.page_count && status == B_OK; page++)
			status = PrintPage(page, pfh.page_count);

		if (status != B_OK)
			break;

		// re-read job message for next page
		fJobFile->Seek(sizeof(pfh), SEEK_SET);
		msg->Unflatten(fJobFile);
	}
		
	CloseTransport();

	delete fJobMsg;
	
	// close status window
	if (fStatusWindow->Lock()) {
		fStatusWindow->Quit();
	}

	return status;
}


status_t 
PrinterDriver::PrintPage(int32 pageNumber, int32 pageCount) 
{
	char text[128];

	sprintf(text, "Faking print of page %ld/%ld...", pageNumber, pageCount);
	BAlert *alert = new BAlert("PrinterDriver::PrintPage()", text, "Hmm?");
	alert->Go();
	return B_OK;
}


status_t 
PrinterDriver::PrinterSetup(char *printerName)
	// name of printer, to attach printer settings
{
	PrinterSetupWindow *psw;

	psw = new PrinterSetupWindow(printerName);
	return psw->Go();
}


status_t PrinterDriver::PageSetup(BMessage *setupMsg, const char *printerName)
{
	BRect paperRect;
	BRect printRect;

	// const float kScale			= 8.3333f;
	const float kScreen			= 72.0f;
	const float kLetterWidth	= 8.5;
	const float kLetterHeight	= 11;

	// set default value if property not set
	if (!setupMsg->HasInt64("xres")) 
		setupMsg->AddInt64("xres", 360);
		
	if (!setupMsg->HasInt64("yres"))
		setupMsg->AddInt64("yres", 360);
				
	if (!setupMsg->HasInt32("orientation"))
		setupMsg->AddInt32("orientation", PORTRAIT_ORIENTATION);
			
	paperRect = BRect(0, 0, kLetterWidth * kScreen, kLetterHeight * kScreen);
	printRect = BRect(0, 0, kLetterWidth * kScreen, kLetterHeight * kScreen);
 	
	if (!setupMsg->HasRect("paper_rect"))
		setupMsg->AddRect("paper_rect", paperRect);

	if (!setupMsg->HasRect("printable_rect"))
		setupMsg->AddRect("printable_rect", printRect);
	 		
	if (!setupMsg->HasFloat("scaling"))
		setupMsg->AddFloat("scaling", kScreen);

	PageSetupWindow *psw;
	
	psw = new PageSetupWindow(setupMsg, printerName);
	return psw->Go();
}



status_t 
PrinterDriver::JobSetup(BMessage *jobMsg, const char *printerName)
{
	// set default value if property not set
	if (!jobMsg->HasInt32("copies"))
		jobMsg->AddInt32("copies", 1);

	if (!jobMsg->HasInt32("first_page"))
		jobMsg->AddInt32("first_page", 1);
		
	if (!jobMsg->HasInt32("last_page"))
		jobMsg->AddInt32("last_page", MAX_INT32);

	JobSetupWindow * jsw;

	jsw = new JobSetupWindow(jobMsg, printerName);
	return jsw->Go();
}


status_t PrinterDriver::OpenTransport()
{
	char	buffer[512];
	BPath	*path;
	

	if (!fPrinterNode)
		return B_ERROR;

	// first, find & load transport add-on
	path = new BPath();
	
	// find name of this printer transport add-on 
	fPrinterNode->ReadAttr("transport", B_STRING_TYPE, 0, buffer, sizeof(buffer));
	
	// try first on user add-ons directory
	find_directory(B_USER_ADDONS_DIRECTORY, path);
	path->Append("Print/transport");
	path->Append(buffer);
	fTransportAddOn = load_add_on(path->Path());
	
	if (fTransportAddOn < 0) {
		// add-on not in user add-ons directory. try system one
		find_directory(B_BEOS_ADDONS_DIRECTORY, path);
		path->Append("Print/transport");
		path->Append(buffer);
		fTransportAddOn = load_add_on(path->Path());
	}
	
	if (fTransportAddOn < 0) {
		BAlert * alert = new BAlert("Uh oh!", "Couldn't find transport add-on.", "OK");
		alert->Go();
		return B_ERROR;
	}
	
	// get init & exit proc
	get_image_symbol(fTransportAddOn, "init_transport", B_SYMBOL_TYPE_TEXT, (void **) &fTransportInitProc);
	get_image_symbol(fTransportAddOn, "exit_transport", B_SYMBOL_TYPE_TEXT, (void **) &fTransportExitProc);
	
	if (!fTransportInitProc || !fTransportExitProc) {
		BAlert * alert = new BAlert("Uh oh!", "Couldn't resolve transport symbols.", "OK");
		alert->Go();
		return B_ERROR;
	}
	
	delete path;
	
	// now, init transport add-on
	node_ref 	ref;
	BDirectory 	dir;

	fPrinterNode->GetNodeRef(&ref);
	dir.SetTo(&ref);
	
	path = new BPath(&dir, NULL);
	strcpy(buffer, path->Path());
	
	// create BMessage for init_transport()
	BMessage *msg = new BMessage('TRIN');
	msg->AddString("printer_file", buffer);
	
	fTransport = (*fTransportInitProc)(msg);
	
	delete msg;
	delete path;
	
	if (fTransport == 0) {
		BAlert *alert = new BAlert("Uh oh!", "Couldn't open transport.", "OK");
		alert->Go();
		return B_ERROR;
	}
	
	return B_OK;
}


status_t 
PrinterDriver::CloseTransport()
{
	if (!fTransportAddOn)
		return B_ERROR;

	if (fTransportExitProc)
		(*fTransportExitProc)();

	unload_add_on(fTransportAddOn);
	fTransportAddOn = 0;
	fTransport = NULL;
	
	return B_OK;
}

#ifdef CODEWARRIOR
	#pragma mark [Privates routines]
#endif

// Private routines
// ----------------
