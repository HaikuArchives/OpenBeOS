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

#ifndef FONTSWINDOW_H
#define FONTSWINDOW_H

#include <InterfaceKit.h>
#include <Message.h>
#include <Messenger.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>
#include <String.h>
#include "Utils.h"
#include "Fonts.h"

class FontsWindow : public HWindow 
{
public:
	// Constructors, destructors, operators...

							FontsWindow(Fonts* fonts);
							~FontsWindow();

	typedef HWindow 		inherited;

	// public constantes
	enum {
		OK_MSG				= 'ok__',
		CANCEL_MSG			= 'cncl',
		EMBED_MSG           = 'mbed',
		SUBST_MSG           = 'subs'
	};
			
	// Virtual function overrides
public:	
	virtual void 			MessageReceived(BMessage *msg);
	virtual bool 			QuitRequested();
	void                    Quit();

private:
	Fonts*                  fFonts;	
	BListView*              fList;

	void                    FillFontList();
	void                    EmptyFontList();
	void                    SetItemText(BStringItem* i, FontFile* f);
	friend class EmbedFont;
};

#endif
