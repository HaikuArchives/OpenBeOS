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

#include <InterfaceKit.h>
#include <SupportKit.h>
#include "pdflib.h"				// for pageFormat constants 
#include "PrinterDriver.h"
#include "PageSetupWindow.h"

// static global variables
static struct {
	char  *label;
	float width;
	float height;
} pageFormat[] = {
	{"Letter", letter_width, letter_height },
	{"Legal",  legal_width,  legal_height  },
	{"Ledger", ledger_width, ledger_height  },
	{"p11x17", p11x17_width, p11x17_height  },
	{"A0",     a0_width,     a0_height     },
	{"A1",     a1_width,     a1_height     },
	{"A2",     a2_width,     a2_height     },
	{"A3",     a3_width,     a3_height     },
	{"A4",     a4_width,     a4_height     },
	{"A5",     a5_width,     a5_height     },
	{"A6",     a6_width,     a6_height     },
	{"B5",     b5_width,     b5_height     },
	{NULL,     0.0,          0.0           }
};


static struct {
	char  *label;
	int32 orientation;
} orientation[] = {
	{"Portrait",  PrinterDriver::PORTRAIT_ORIENTATION},
	{"Landscape", PrinterDriver::LANDSCAPE_ORIENTATION},
	{NULL, 0}
};


PageSetupWindow::PageSetupWindow(BMessage *msg, const char *printerName)
	:	BWindow(BRect(0,0,200,200), "Page Setup", B_TITLED_WINDOW_LOOK,
 			B_MODAL_APP_WINDOW_FEEL, B_NOT_RESIZABLE | B_NOT_MINIMIZABLE |
 			B_NOT_ZOOMABLE)
{
	fSetupMsg	= msg;
	fExitSem 	= create_sem(0, "PageSetup");
	fResult		= B_ERROR;

	if (printerName) {
		BString	title;
		
		title << printerName << " Job Setup";
		SetTitle(title.String());
	}
	
#define MARGIN 6

	// ---- Ok, build a default job setup user interface
	BRect		r;
	BBox		*panel;
	BButton		*button;
	float		x, y, w, h;
	int         i;
	BMenuItem	*item;
	float       width, height;
	int32       orient;
	
	if (B_OK == msg->FindRect("paper_rect", &r)) {
		width = r.Width(); height = r.Height();
	} else {
		width = a4_width; height = a4_height;
	}		
	
	// if (B_OK != msg->FindInt32("orientation", &orient)) orient = 0;
	orient = PrinterDriver::PORTRAIT_ORIENTATION;
	
	r = Bounds();

	// add a *dialog* background
	panel = new BBox(r, "top_panel", B_FOLLOW_ALL, 
					B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
					B_PLAIN_BORDER);

	// add page format menu
	x = r.left + MARGIN; y = r.top + MARGIN;
	
	BMenu* m = new BMenu("popupmenu");
	m->SetRadioMode(true);
	BMenuField * mf = new BMenuField(BRect(x, y, x + 200, y + 20), "page_size", 
		"Page Size", m);
	fPageSizeMenu = mf;
	mf->ResizeToPreferred();
	mf->GetPreferredSize(&w, &h);
	panel->AddChild(mf);

	item = NULL;
	for (i = 0; pageFormat[i].label != NULL; i++) {
		BMessage* msg = new BMessage('pgsz');
		msg->AddFloat("width", pageFormat[i].width);
		msg->AddFloat("height", pageFormat[i].height);
		BMenuItem* mi = new BMenuItem(pageFormat[i].label, msg);
		m->AddItem(mi);
		if (width == pageFormat[i].width && height == pageFormat[i].height) {
			item = mi; orient = PrinterDriver::PORTRAIT_ORIENTATION;
		}
		if (height == pageFormat[i].width && width == pageFormat[i].height) {
			item = mi; orient = PrinterDriver::LANDSCAPE_ORIENTATION;
		}
	}
	mf->Menu()->SetLabelFromMarked(true);
	if (!item) item = m->ItemAt(0);
	item->SetMarked(true);
	mf->MenuItem()->SetLabel(item->Label());

	// add orientation menu
	y += h + MARGIN;
	 
	m = new BMenu("orientation");
	m->SetRadioMode(true);
	mf = new BMenuField(BRect(x, y, x + 200, y + 20), "orientation", "Orientation", m);
	fOrientationMenu = mf;
	mf->ResizeToPreferred();
	panel->AddChild(mf);
	r.top += h;
	item = NULL;
	for (int i = 0; orientation[i].label != NULL; i++) {
		BMessage* msg = new BMessage('ornt');
		msg->AddInt32("orientation", orientation[i].orientation);
		BMenuItem* mi = new BMenuItem(orientation[i].label, msg);
		m->AddItem(mi);
		if (orient == orientation[i].orientation) item = mi;
	}
	mf->Menu()->SetLabelFromMarked(true);
	if (!item) m->ItemAt(0);
	item->SetMarked(true);
	mf->MenuItem()->SetLabel(item->Label());
	
	// add a "OK" button, and make it default
	button 	= new BButton(r, NULL, "OK", new BMessage(OK_MSG), 
		B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	button->ResizeToPreferred();
	button->GetPreferredSize(&w, &h);
	x = r.right - w - 8;
	y = r.bottom - h - 8;
	button->MoveTo(x, y);
	panel->AddChild(button);
	button->MakeDefault(true);

	// add a "Cancel button	
	button 	= new BButton(r, NULL, "Cancel", new BMessage(CANCEL_MSG), 
		B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	button->GetPreferredSize(&w, &h);
	button->ResizeToPreferred();
	button->MoveTo(x - w - 8, y);
	panel->AddChild(button);

	// add a separator line...
	BBox * line = new BBox(BRect(r.left, y - 9, r.right, y - 8), NULL,
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM );
	panel->AddChild(line);

	// Finally, add our panel to window
	AddChild(panel);
}


void 
PageSetupWindow::UpdateSetupMessage() 
{
	BMenuItem *item;
	int32 orientation = 0;
	item = fOrientationMenu->Menu()->FindMarked();
	if (item) {
		BMessage *msg = item->Message();
		msg->FindInt32("orientation", &orientation);
		fSetupMsg->ReplaceInt32("orientation", 0);
	}

	item = fPageSizeMenu->Menu()->FindMarked();
	if (item) {
		float w, h;
		BMessage *msg = item->Message();
		msg->FindFloat("width", &w);
		msg->FindFloat("height", &h);
		BRect r;
		if (orientation == 0) 
			r.Set(0, 0, w, h);
		else
			r.Set(0, 0, h, w);
		fSetupMsg->ReplaceRect("paper_rect", r);
		r.InsetBy(10,10);
		fSetupMsg->ReplaceRect("printable_rect", r);
	}
}


PageSetupWindow::~PageSetupWindow()
{
	delete_sem(fExitSem);
}


bool 
PageSetupWindow::QuitRequested()
{
	release_sem(fExitSem);
	return true;
}


void 
PageSetupWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what){
		case OK_MSG:
			UpdateSetupMessage();
			fResult = B_OK;
			release_sem(fExitSem);
			break;
		
		case CANCEL_MSG:
			fResult = B_ERROR;
			release_sem(fExitSem);
			break;
					
		default:
			inherited::MessageReceived(msg);
			break;
	}
}
			

status_t 
PageSetupWindow::Go()
{
	MoveTo(300,300);
	Show();
	acquire_sem(fExitSem);
	Lock();
	Quit();

	return fResult;
}
