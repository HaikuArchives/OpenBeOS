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
#include <ctype.h>

/* TODO:
  - scroll to TextControl that receives focus
*/


#if 0
class TextControl : public BView
{
	BStringView *fLabel;
	BTextView   *fText;
public:
	TextControl(BRect frame,
				const char *name,
				const char *label, 
				const char *initial_text, 
				BMessage *message,
				uint32 rmask = B_FOLLOW_LEFT | B_FOLLOW_TOP,
				uint32 flags = B_WILL_DRAW | B_NAVIGABLE); 
	void MakeFocus(bool focus = true);	
	const char *Label() { return fLabel->Text(); }
	const char *Text()  { return fText->Text(); }
};


TextControl::TextControl(BRect frame,
				const char *name,
				const char *label, 
				const char *initial_text, 
				BMessage *message,
				uint32 rmask = B_FOLLOW_LEFT | B_FOLLOW_TOP,
				uint32 flags = B_WILL_DRAW | B_NAVIGABLE)
	: BView(frame, name, rmask, flags)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect r(0, 0, frame.Width() / 2, frame.Height());
	fLabel = new BStringView(r, "", label);
	BRect f(r);
	f.OffsetTo(frame.Width() / 2 + 1, 0);
	r.InsetBy(2,2);
	fText  = new BTextView(f, "", r, rmask, flags | B_NAVIGABLE);
	fText->SetWordWrap(false);
	fText->DisallowChar('\n'); fText->DisallowChar('\t');
	fText->Insert(initial_text);
	AddChild(fLabel); AddChild(fText);
}

void 
TextControl::MakeFocus(bool focus)
{
	if (focus && Parent()) {
		BView *p = Parent();
		p->ScrollTo(0, Frame().top);
//		(new BAlert("test", "test", "test", NULL, NULL))->Go();
	}
	fText->MakeFocus(focus);
}

#else
#define TextControl BTextControl
/*
class TextControl : public BTextControl
{
public:
	TextControl(BRect frame,
				const char *name,
				const char *label, 
				const char *initial_text, 
				BMessage *message,
				uint32 rmask = B_FOLLOW_LEFT | B_FOLLOW_TOP,
				uint32 flags = B_WILL_DRAW | B_NAVIGABLE); 
	void MakeFocus(bool focus = true);	
};


TextControl::TextControl(BRect frame,
				const char *name,
				const char *label, 
				const char *initial_text, 
				BMessage *message,
				uint32 rmask = B_FOLLOW_LEFT | B_FOLLOW_TOP,
				uint32 flags = B_WILL_DRAW | B_NAVIGABLE)
	: BTextControl(frame, name, label, initial_text, message, rmask, flags)
{
}

void 
TextControl::MakeFocus(bool focus)
{
	if (focus && Parent()) {
		BView *p = Parent();
		p->ScrollTo(0, Frame().top);
		(new BAlert("test", "test", "test", NULL, NULL))->Go();
	}
	BTextControl::MakeFocus(focus);
}
*/
#endif

/**
 * Constuctor
 *
 * @param 
 * @return
 */
DocInfoWindow::DocInfoWindow(BMessage *doc_info)
	:	HWindow(BRect(0,0,400,220), "Document Information", B_TITLED_WINDOW_LOOK,
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

	// add list of keys
	fKeyList = new BMenu("Delete Key");
	BMenuField *menu = new BMenuField(BRect(0, 0, 180, 10), "delete", "", fKeyList);
	menu->SetDivider(0);
	panel->AddChild(menu);
//	menu->PreferredSize(&w, &h);
//	menu->ResizeTo(100, 20);

	// add table for text controls (document info key and value)
	fTable = new BView(BRect(r.left+5, r.top+5, r.right-5-B_V_SCROLL_BAR_WIDTH, r.bottom-5-B_H_SCROLL_BAR_HEIGHT-30-20), "table", B_FOLLOW_ALL, B_WILL_DRAW);
	fTable->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// add table to ScrollView
	fTableScrollView = new BScrollView("scroll_table", fTable, B_FOLLOW_LEFT | B_FOLLOW_TOP, 0, false, true);
	panel->AddChild(fTableScrollView);

	// position list of keys
	menu->MoveTo(5, fTableScrollView->Frame().bottom+2);

	// add add key text control
	BTextControl *add = new BTextControl(BRect(0, 0, 180, 20), "add", "Add Key:", "", new BMessage(ADD_KEY_MSG));
	add->SetDivider(60);
	panel->AddChild(add);
	add->MoveTo(menu->Frame().right + 5, menu->Frame().top);

	// fill table
	BuildTable(fDocInfo);

	// add a "OK" button, and make it default
	button 	= new BButton(r, NULL, "OK", new BMessage(OK_MSG), 
		B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	button->ResizeToPreferred();
	button->GetPreferredSize(&w, &h);
	x = r.right - w - 8;
	y = r.bottom - h - 8;
	button->MoveTo(x, y);
	panel->AddChild(button);
//	button->MakeDefault(true);

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
	
	if (fTable->ChildAt(0)) fTable->ChildAt(0)->MakeFocus();
}


// --------------------------------------------------
bool 
DocInfoWindow::QuitRequested()
{
	return true;
}


// --------------------------------------------------
void 
DocInfoWindow::Quit()
{
	EmptyKeyList();
	inherited::Quit();
}


// --------------------------------------------------
void 
DocInfoWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what){
		case OK_MSG: ReadFieldsFromTable(fDocInfo); Quit();
			break;
		
		case CANCEL_MSG: Quit();
			break;
			
		case ADD_KEY_MSG: AddKey(msg);
			break;
		
		case REMOVE_KEY_MSG: RemoveKey(msg);
			break;

		default:
			inherited::MessageReceived(msg);
			break;
	}
}

// --------------------------------------------------
void 
DocInfoWindow::BuildTable(BMessage *docInfo) 
{
	BRect r;
	float y;
	float w;
	float rowHeight;
	char *name;
	uint32 type;
	int32 count;

	EmptyKeyList();
	while (fTable->ChildAt(0)) {
		BView *child = fTable->ChildAt(0);
		fTable->RemoveChild(child);
		delete child;
	}

	fTable->ScrollTo(0, 0);
	
	r = fTable->Bounds();
	y = 5;
	w = r.Width() - 10;

	for (int32 i = 0; docInfo->GetInfo(B_STRING_TYPE, i, &name, &type, &count) != B_BAD_INDEX; i++) {
		if (type == B_STRING_TYPE) {
			BString value;
			if (docInfo->FindString(name, &value) == B_OK) {
				BString s;
				TextControl* v = new TextControl(BRect(0, 0, w, 20), name, name, value.String(), new BMessage());
				float w;
				fTable->AddChild(v);
				v->GetPreferredSize(&w, &rowHeight);
				v->MoveTo(5, y);
				y += rowHeight + 2;

				fKeyList->AddItem(new BMenuItem(name, new BMessage(REMOVE_KEY_MSG)));
			}
		}
	}
	
	BScrollBar *sb = fTableScrollView->ScrollBar(B_VERTICAL);
	if (sb) {
		float th = fTable->Bounds().Height()+1;
		float h = y - th;
		if (h > 0) {
			sb->SetProportion(th / (float)y);
			sb->SetRange(0, h);
			sb->SetSteps(rowHeight + 2, th);
		} else {
			sb->SetRange(0, 0);
		}
	}
}


// --------------------------------------------------
void 
DocInfoWindow::ReadFieldsFromTable(BMessage *toDocInfo) 
{
	BView* child;
	BMessage m;
	for (int32 i = 0; (child = fTable->ChildAt(i)) != NULL; i++) {
		TextControl* t = dynamic_cast<TextControl*>(child);
		if (t) {
			m.AddString(t->Label(), t->Text());
		}
	}
	*toDocInfo = m;
}


// --------------------------------------------------
bool 
DocInfoWindow::IsValidKey(const char* key) 
{
	if (*key == 0) return false;
	while (*key) {
		if (isspace(*key) || iscntrl(*key)) break;
		key ++;
	}
	return *key == 0;
}


// --------------------------------------------------
void 
DocInfoWindow::AddKey(BMessage *msg) 
{
	void *p;
	BTextControl *text;
	BString key;
	BMessage docInfo;
	if (msg->FindPointer("source", &p) != B_OK) return;
	text = reinterpret_cast<BTextControl*>(p);
	if (!text) return;
	key = text->Text();
	
	// key is valid and is not list already
	if (IsValidKey(key.String())) {
		BMessage docInfo;
		ReadFieldsFromTable(&docInfo);
		if (!docInfo.HasString(key.String())) {
			docInfo.AddString(key.String(), "");
			BuildTable(&docInfo);
		}
	}
}


// --------------------------------------------------
void 
DocInfoWindow::RemoveKey(BMessage *msg) 
{
	void *p;
	BMenuItem *item;
	BString key;
	BMessage docInfo;
	if (msg->FindPointer("source", &p) != B_OK) return;
	item = reinterpret_cast<BMenuItem*>(p);
	if (!item) return;
	key = item->Label();
	
	ReadFieldsFromTable(&docInfo);
	if (docInfo.HasString(key.String())) {
		docInfo.RemoveName(key.String());
		BuildTable(&docInfo);
	}
}

// --------------------------------------------------
void 
DocInfoWindow::EmptyKeyList() 
{
	while (fKeyList->ItemAt(0)) {
		BMenuItem *i = fKeyList->ItemAt(0);
		fKeyList->RemoveItem(i);
		delete i;
	}
}
