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
#include <StringView.h>
#include <Box.h>

StatusWindow::StatusWindow(const char *text, BRect rect) 
	:	BWindow(rect, "PDF Writer", 
			B_MODAL_WINDOW, 
			B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_NOT_CLOSABLE) 
{

	BRect r(0, 0, rect.Width(), rect.Height());

	// view for the background color
	BView *panel = new BBox(r, "top_panel", B_FOLLOW_ALL, 
					B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
					B_PLAIN_BORDER);
	AddChild(panel);

	BStringView *stringView = new BStringView(r, "status_text", 
		"Generating PDF file...");
	stringView->SetAlignment(B_ALIGN_CENTER);
	panel->AddChild(stringView);

	Show();
}
