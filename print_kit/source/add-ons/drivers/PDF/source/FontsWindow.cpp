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
#include "FontsWindow.h"
#include "Fonts.h"

/**
 * Constuctor
 *
 * @param 
 * @return
 */
FontsWindow::FontsWindow(Fonts *fonts)
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

	fFonts = fonts;
	
	// add a *dialog* background
	r = Bounds();
	panel = new BBox(r, "top_panel", B_FOLLOW_ALL, 
					B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
					B_PLAIN_BORDER);

	// add font list
	fList = new BListView(BRect(r.left+5, r.top+5, r.right-5-B_V_SCROLL_BAR_WIDTH, r.bottom-5-B_H_SCROLL_BAR_HEIGHT-30),
		"fonts_list", B_MULTIPLE_SELECTION_LIST);
	panel->AddChild(new BScrollView("scroll_list", fList, 
		B_FOLLOW_LEFT | B_FOLLOW_TOP, 0, false, true));

	FillFontList();

	// add a "OK" button, and make it default
	button 	= new BButton(r, NULL, "Embed", new BMessage(EMBED_MSG), 
		B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	button->ResizeToPreferred();
	button->GetPreferredSize(&w, &h);
	x = r.right - w - 8;
	y = r.bottom - h - 8;
	button->MoveTo(x, y);
	panel->AddChild(button);
	button->MakeDefault(true);

	// add a "Cancel button	
	button 	= new BButton(r, NULL, "Substitute", new BMessage(SUBST_MSG), 
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
FontsWindow::~FontsWindow()
{
}


// --------------------------------------------------
bool 
FontsWindow::QuitRequested()
{
	return true;
}


// --------------------------------------------------
void 
FontsWindow::Quit()
{
	inherited::Quit();
}


class EmbedFont 
{
	FontsWindow* fWindow;
	BListView*   fList;
	Fonts*       fFonts;
	bool         fEmbed;
	
	bool DoIt(BListItem* item);
public:
	EmbedFont(FontsWindow* window, BListView* list, Fonts* fonts, bool embed);
	static bool DoIt(BListItem* item, void* data);
};

EmbedFont::EmbedFont(FontsWindow* window, BListView* list, Fonts* fonts, bool embed)
	: fWindow(window)
	, fList(list)
	, fFonts(fonts)
	, fEmbed(embed)
{
}

bool
EmbedFont::DoIt(BListItem* item, void* data)
{
	if (item->IsSelected()) {
		EmbedFont* e = (EmbedFont*)data;
		return e->DoIt(item);
	}
	return false;
}

bool
EmbedFont::DoIt(BListItem* item)
{
	int32 i = fList->IndexOf(item);
	if (0 <= i && i < fFonts->Length()) {
		fFonts->At(i)->SetEmbed(fEmbed);
		fWindow->SetItemText((BStringItem*)item, fFonts->At(i));
	}
	return false;
}

// --------------------------------------------------
void 
FontsWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what){
		case OK_MSG: Quit();
			break;
		
		case CANCEL_MSG: Quit();
			break;

		case EMBED_MSG:
			{
			EmbedFont e(this, fList, fFonts, true);
			fList->DoForEach(EmbedFont::DoIt, (void*)&e);
			}
			fList->Invalidate();
			break;
			
		case SUBST_MSG:
			{
			EmbedFont e(this, fList, fFonts, false);
			fList->DoForEach(EmbedFont::DoIt, (void*)&e);
			}
			fList->Invalidate();
			break;
	
		default:
			inherited::MessageReceived(msg);
			break;
	}
}

void
FontsWindow::SetItemText(BStringItem* i, FontFile* f)
{
	const int64 KB = 1024;
	const int64 MB = KB * KB;
	char buffer[40];
	BString s;
	s << f->Name() << ": ";
	if (f->Type() == true_type_type) s << "TrueType";
	else s << "Type 1";
	s << " (";
	if (f->Size() >= MB) {
		sprintf(buffer, "%d MB", (int)(f->Size()/MB));
	} else if (f->Size() >= KB) {
		sprintf(buffer, "%d KB", (int)(f->Size()/KB));
	} else {
		sprintf(buffer, "%d B", (int)(f->Size()));
	} 
	s << buffer;
	s << ") ";
	s << (f->Embed() ? "embed" : "substitute");
	i->SetText(s.String());
}
			
void
FontsWindow::FillFontList()
{
	const int n = fFonts->Length();
	for (int i = 0; i < n; i++) {
		BStringItem* s;
		FontFile* f = fFonts->At(i);
		fList->AddItem(s = new BStringItem(""));
		SetItemText(s, f);
	}
}

void
FontsWindow::EmptyFontList()
{
	const int n = fList->CountItems();
	for (int i = 0; i < n; i++) {
		BListItem* item = fList->ItemAt(i);
		delete item;
	}
	fList->MakeEmpty();
}