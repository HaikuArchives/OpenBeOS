/*

PDF Writer printer driver.

Copyright (c) 2001 OpenBeOS. 

Authors: 
	Philippe Houdoin
	Simon Gauvin	
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

#include "StatusWindow.h"
#include <Message.h>
#include <Box.h>


StatusWindow::StatusWindow(int32 pages, PrinterDriver *pd) 
	:	BWindow(BRect(100, 100, 400, 185), "PDF Writer", 
			B_TITLED_WINDOW, 
			B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_NOT_CLOSABLE) 
{
	printerDriver = pd;
	pageCount = 0;
//	copyCount = 0;
	BRect r(0, 0, Frame().Width(), Frame().Height());

	// view for the background color
	BView *panel = new BBox(r, "top_panel", B_FOLLOW_ALL, 
					B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
					B_PLAIN_BORDER);
	AddChild(panel);
/*
	if (copies > 1) {
		pages *= copies;
	}
	
	r.Set(10, 0, Frame().Width(), 50);
	copyLabel = new BStringView(r, "copy_text", "Copy");
	panel->AddChild(copyLabel);
	
	r.Set(10, 55, Frame().Width()-20, 65);
	copyStatus = new BStatusBar(r, "copyStatus");
	copyStatus->SetMaxValue(copies);
	copyStatus->SetBarHeight(12);
	panel->AddChild(copyStatus);
*/	
	r.Set(10, 12, Frame().Width()-5, 22);
	pageLabel = new BStringView(r, "page_text", "Page");
	panel->AddChild(pageLabel);

	r.Set(10, 15, Frame().Width()-10, 10);
	pageStatus = new BStatusBar(r, "pageStatus");
	pageStatus->SetMaxValue(pages);
	pageStatus->SetBarHeight(12);
	panel->AddChild(pageStatus);

	// Cancel button
	// add a separator line...
//	BBox *line = new BBox(BRect(r.left, 50, r.right, 51), NULL,
//						 B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP );
//	panel->AddChild(line);

	// add a "Cancel button
	int32 x = 110;
	int32 y = 55;
	cancel 	= new BButton(BRect(x, y, x + 100, y + 20), NULL, "Cancel", 
				new BMessage('cncl'), B_FOLLOW_RIGHT | B_FOLLOW_TOP);
	cancel->ResizeToPreferred();
	panel->AddChild(cancel);

	Show();
}

void StatusWindow::MessageReceived(BMessage *msg) 
{
	switch (msg->what) {
/*
		case 'copy':
			copy = "";
			copy << "Copy: " << ++copyCount;
			copyLabel->SetText(copy.String());
			copyStatus->Update(1);
			break;
*/
		case 'cncl':
			printerDriver->StopPrinting();
			break;
		case 'page':
			page = "";
			page << "Writing Page: " << ++pageCount;
			pageLabel->SetText(page.String());
			pageStatus->Update(1);
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}
