/*

PDF Writer printer driver.

Copyright (c) 2002 OpenBeOS. 

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
#include "DocInfoWindow.h"

/* TODO:
  - 
  - allow adding of user defined document information fields
  - add checks: 
    - key must be in DocEncoding, 
    - key must not be Title or Creator (already set in PDFWriter::InitWriter)
    - size of value must be less than 255
*/

/**
 * Constuctor
 *
 * @param 
 * @return
 */
DocInfoWindow::DocInfoWindow(BMessage *doc_info)
	:	HWindow(BRect(0,0,400,220), "Fonts", B_TITLED_WINDOW_LOOK,
 			B_MODAL_APP_WINDOW_FEEL, B_NOT_RESIZABLE | B_NOT_MINIMIZABLE |
 			B_NOT_ZOOMABLE)
{
	// ---- Ok, build a default page setup user interface
	BRect		r;
	BBox		*panel;
	BButton		*button;
	float		x, y, w, h;
	BString 	setting_value;

	fDocInfo = doc_info;
	
	// add a *dialog* background
	r = Bounds();
	panel = new BBox(r, "top_panel", B_FOLLOW_ALL, 
					B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
					B_PLAIN_BORDER);

	// add table for text controls (document info key and value)
	fTable = new BView(BRect(r.left+5, r.top+5, r.right-5-B_V_SCROLL_BAR_WIDTH, r.bottom-5-B_H_SCROLL_BAR_HEIGHT-30), "table", B_FOLLOW_ALL, B_WILL_DRAW);
	fTable->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// add table to ScrollView
	panel->AddChild(new BScrollView("scroll_table", fTable, B_FOLLOW_LEFT | B_FOLLOW_TOP, 0, false, true));

	// fill table
	BuildTable();

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
	
	MoveTo(320, 320);
}


// --------------------------------------------------
bool 
DocInfoWindow::QuitRequested()
{
	return true;
}


// --------------------------------------------------
void 
DocInfoWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what){
		case OK_MSG: ReadFieldsFromTable(); Quit();
			break;
		
		case CANCEL_MSG: Quit();
			break;

		default:
			inherited::MessageReceived(msg);
			break;
	}
}

// --------------------------------------------------
void 
DocInfoWindow::BuildTable() 
{
	BRect r;
	float y;
	float w;
	char *name;
	uint32 type;
	int32 count;
	
	r = fTable->Bounds();
	y = 5;
	w = r.Width() - 10;
	for (int32 i = 0; fDocInfo->GetInfo(B_STRING_TYPE, i, &name, &type, &count) != B_BAD_INDEX; i++) {
		if (type == B_STRING_TYPE) {
			BString value;
			if (fDocInfo->FindString(name, &value) == B_OK) {
				BString s;
				BTextControl* v = new BTextControl(BRect(0, 0, w, 20), "", name, value.String(), new BMessage());
				float w, h;
				fTable->AddChild(v);
				v->GetPreferredSize(&w, &h);
				v->MoveTo(5, y);
				y += h + 2;
			}
		}
	}
}


// --------------------------------------------------
void 
DocInfoWindow::ReadFieldsFromTable() 
{
	BView* child;
	BMessage m;
	for (int32 i = 0; (child = fTable->ChildAt(i)) != NULL; i++) {
		BTextControl* t = dynamic_cast<BTextControl*>(child);
		if (t) {
			m.AddString(t->Label(), t->Text());
		}
	}
	*fDocInfo = m;
}


