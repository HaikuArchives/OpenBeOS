/*

PDF Writer printer driver.

Version: 12.19.2000

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

#include <InterfaceKit.h>
#include <SupportKit.h>
#include "pdflib.h"				// for pageFormat constants 
#include "PrinterDriver.h"
#include "PageSetupWindow.h"

// Simon: added
#include "MarginView.h"

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

static	const char * pdf_compatibility[] = {"1.2", "1.3", "1.4", NULL};

// Simon changed: window size 200,200 -> 400,250
PageSetupWindow::PageSetupWindow(BMessage *msg, const char *printerName)
	:	BWindow(BRect(0,0,400,220), "Page Setup", B_TITLED_WINDOW_LOOK,
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
	
// Simon changed 6 -> 10
#define MARGIN 10

// Simon added
#define OFFSET 200 

	// ---- Ok, build a default page setup user interface
	BRect		r;
	BBox		*panel;
	BButton		*button;
	float		x, y, w, h;
	int         i;
	BMenuItem	*item;
	float       width, height;
	int32       orient;
	BRect		margin, page;
	
	if (B_OK == fSetupMsg->FindRect("paper_rect", &r)) {
		width = r.Width(); height = r.Height();
	} else {
		width = a4_width; height = a4_height;
	}		
	page = r;
	
	// if (B_OK != fSetupMsg->FindInt32("orientation", &orient)) orient = 0;
	orient = PrinterDriver::PORTRAIT_ORIENTATION;
	
	r = Bounds();

	// add a *dialog* background
	panel = new BBox(r, "top_panel", B_FOLLOW_ALL, 
					B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
					B_PLAIN_BORDER);

	// Create the margin view

	if (B_OK == fSetupMsg->FindRect("printable_rect", &margin)) 
	{
		// re-calculate the margin from the printable rect
		//margin.top -= page.top;
		margin.left -= page.left;
		margin.right = page.right - margin.right;
		margin.bottom = page.bottom - margin.bottom;

		marginView = new MarginView(BRect(20,20,200,160), width, height,
			margin, UNIT_POINT);
	} else {
		marginView = new MarginView(BRect(20,20,200,160), width, height);
	}
	panel->AddChild(marginView);
	
	// add page format menu
	//x = r.left + MARGIN; y = r.top + MARGIN;
	// Simon Changed to OFFSET popups
	x = r.left + MARGIN * 2 + OFFSET; y = r.top + MARGIN * 2;
		
	BMenu* m = new BMenu("popupmenu");
	m->SetRadioMode(true);

	// Simon changed width 200->140
	BMenuField * mf = new BMenuField(BRect(x, y, x + 140, y + 20), "page_size", 
		"Page Size:", m);
	fPageSizeMenu = mf;
	mf->ResizeToPreferred();
	mf->GetPreferredSize(&w, &h);

	// Simon added: SetDivider
	mf->SetDivider(be_plain_font->StringWidth("Page Size#"));

	panel->AddChild(mf);

	item = NULL;
	for (i = 0; pageFormat[i].label != NULL; i++) 
	{
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
	if (!item)
		item = m->ItemAt(0);
	item->SetMarked(true);
	mf->MenuItem()->SetLabel(item->Label());

	// add orientation menu
	y += h + MARGIN;
	 
	m = new BMenu("orientation");
	m->SetRadioMode(true);
	
	// Simon changed 200->140
	mf = new BMenuField(BRect(x, y, x + 140, y + 20), "orientation", "Orientation:", m);
	
	// Simon added: SetDivider
	mf->SetDivider(be_plain_font->StringWidth("Orientation#"));
		
	fOrientationMenu = mf;
	mf->ResizeToPreferred();
	panel->AddChild(mf);
	r.top += h;
	item = NULL;
	for (int i = 0; orientation[i].label != NULL; i++) 
	{
	 	BMessage* msg = new BMessage('ornt');		
		msg->AddInt32("orientation", orientation[i].orientation);
		BMenuItem* mi = new BMenuItem(orientation[i].label, msg);
		m->AddItem(mi);
		
		if (orient == orientation[i].orientation) item = mi;
	}
	mf->Menu()->SetLabelFromMarked(true);
	if (!item)
		item = m->ItemAt(0);
	item->SetMarked(true);
	mf->MenuItem()->SetLabel(item->Label());

	// add PDF comptibility  menu
	y += h + MARGIN;
	 
	m = new BMenu("pdf_compatibility");
	m->SetRadioMode(true);
	mf = new BMenuField(BRect(x, y, x + 200, y + 20), "pdf_compatibility", "PDF Compatibility:", m);
	fPDFCompatibilityMenu = mf;
	mf->ResizeToPreferred();
	panel->AddChild(mf);
	r.top += h;

	item = NULL;
	for (int i = 0; pdf_compatibility[i] != NULL; i++) {
		const char * setting_value;
		
		BMenuItem* mi = new BMenuItem(pdf_compatibility[i], NULL);
		m->AddItem(mi);
		
		if ( (fSetupMsg->FindString("pdf_compatibility", &setting_value) == B_OK) &&
			 strcmp(setting_value, pdf_compatibility[i]) == 0)
			item = mi;
	}
	mf->Menu()->SetLabelFromMarked(true);
	if (!item)
		item = m->ItemAt(1);	// "1.3" by default
	item->SetMarked(true);
	mf->MenuItem()->SetLabel(item->Label());

	// add PDF compression slider
	y += h + MARGIN;

	BSlider * slider = new BSlider(BRect(x, y, panel->Bounds().right - MARGIN, y + 20), "pdf_compression",
								"Compression:",
								NULL, 0, 9);
	fPDFCompressionSlider = slider;

	panel->AddChild(slider);
					
	slider->SetLimitLabels("None", "Best");
	slider->SetHashMarks(B_HASH_MARKS_BOTTOM);
	slider->ResizeToPreferred();
	slider->GetPreferredSize(&w, &h);
	
	int32 compression;
	if (fSetupMsg->FindInt32("pdf_compression", &compression) != B_OK)
		compression = 6;
	slider->SetValue(compression);
	
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
PageSetupWindow::UpdateSetupMessage() 
{
	BMenuItem *item;
	int32 orientation = 0;
	item = fOrientationMenu->Menu()->FindMarked();
	if (item) {
		BMessage *msg = item->Message();
		msg->FindInt32("orientation", &orientation);
		fSetupMsg->ReplaceInt32("orientation", orientation);
	}

	item = fPDFCompatibilityMenu->Menu()->FindMarked();
	if (item) {
		if (fSetupMsg->HasString("pdf_compatibility"))
			fSetupMsg->ReplaceString("pdf_compatibility", item->Label());
		else
			fSetupMsg->AddString("pdf_compatibility", item->Label());
	}
			
	if (fSetupMsg->HasInt32("pdf_compression"))
		fSetupMsg->ReplaceInt32("pdf_compression", fPDFCompressionSlider->Value());
	else
		fSetupMsg->AddInt32("pdf_compression", fPDFCompressionSlider->Value());

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
		
	// Simon Todo: Add Margin values to setup message
	/*
		Philippe: not sure what to do here, what it the 
			printable_rect supposed to be? I also need a way
			to save the units in the message, and assume that
			all your calculations are performed by PDFLib
			in points?...
	*/
		// calculate the printable_rect like this?
		
		BRect margin = marginView->GetMargin();
		r.top += margin.top;
		r.right -= margin.right;
		r.left += margin.left;
		r.bottom -= margin.bottom;
		fSetupMsg->ReplaceRect("printable_rect", r);
	}
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

		// Simon added
		case 'pgsz':
			{
				float w, h;
				msg->FindFloat("width", &w);
				msg->FindFloat("height", &h);
				BMenuItem *item = fOrientationMenu->Menu()->FindMarked();
				if (item) {
					int32 orientation = 0;
					BMessage *m = item->Message();
					m->FindInt32("orientation", &orientation);
					if (orientation == PrinterDriver::PORTRAIT_ORIENTATION) {
						marginView->SetPageSize(w, h);
					} else {
						marginView->SetPageSize(h, w);
					}
					marginView->UpdateView();
				}
			}
			break;

		// Simon added
		case 'ornt':
			{	
				BPoint p = marginView->GetPageSize();
				int32 orientation;
				msg->FindInt32("orientation", &orientation);
				if (orientation == PrinterDriver::LANDSCAPE_ORIENTATION
					&& p.x > p.y) { 
					marginView->SetPageSize(p.y, p.x);
					marginView->UpdateView();
				}
				if (orientation == PrinterDriver::PORTRAIT_ORIENTATION
					&& p.y > p.x) {
					marginView->SetPageSize(p.y, p.x);
					marginView->UpdateView();
				}
			}
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
