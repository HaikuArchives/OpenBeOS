/*

MarginView.cpp

Copyright (c) 2001 OpenBeOS.

Author: Simon Gauvin

Version 2001.12.27

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

	Todo:

	2 Make Strings constants or UI resources
	4 Finish documentation
	
*/

#ifndef MARGIN_VIEW_H
#include "MarginView.h"
#endif

#include <AppKit.h>
#include <SupportKit.h>

#include <stdio.h>
#include <stdlib.h>

/*----------------- MarginView Private Constants --------------------*/

const int Y_OFFSET = 20;
const int X_OFFSET = 10;
const int STRING_SIZE = 50;
const int _WIDTH = 50;
const int NUM_COUNT = 10;

const static float _pointUnits = 1; // 1 point = 1 point 
const static float _inchUnits = 72; // 1" = 72 points
const static float _cmUnits = 28.346; // 72/2.54 1cm = 28.346 points

const static float _minFieldWidth = 100; // pixels
const static float _minUnitHeight = 30; // pixels
const static float _drawInset = 10; // pixels
	
const static float unitFormat[] = { _inchUnits, _cmUnits, _pointUnits }; 
const static char *unitNames[] = { "Inch", "cm", "Points", NULL };
const static uint32 unitMsg[] = { MarginView::UNIT_INCH, 
								  MarginView::UNIT_CM, 
								  MarginView::UNIT_POINT };

const pattern dots = {{ 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 }};

const rgb_color black 	= { 0,0,0,0 };
const rgb_color red 	= { 255,0,0,0 };
const rgb_color white 	= { 255,255,255,0 };
const rgb_color gray 	= { 220,220,220,0 };

/*----------------- MarginView Public Methods --------------------*/

/**
 * Constructor
 *
 * @param frame, BRect that is the size of the view passed to the superclase
 * @param pageWidth, float that is the points value of the page width
 * @param pageHeight, float that is the points value of the page height
 * @param margins, BRect values of margins
 * @param units, unit32 enum for units used in view
 * @return void
 */
MarginView::MarginView(BRect frame,
		int32 pageWidth,
		int32 pageHeight,
		BRect margins,
		uint32 units)
		
	   	:BBox(frame, NULL, B_FOLLOW_ALL)
{
	//char str[STRING_SIZE];
	BMessage *msg;
	BString str;

	this->units = units;
	unitValue = unitFormat[units];
	
	SetLabel("Margins");
	
	maxPageHeight = frame.Height() - _minUnitHeight - Y_OFFSET;
	maxPageWidth = frame.Width() - _minFieldWidth - X_OFFSET;

	this->margins = margins;
		
	this->pageWidth = pageWidth;
	this->pageHeight = pageHeight; 

	// Create the BLooper to handle unit menu messages
	marginMgr = new MarginManager(this);

// Create text fields
	msg = new BMessage(MARGIN_CHANGED);
	BRect r(frame.Width() - be_plain_font->StringWidth("Top#") - _WIDTH, 
			Y_OFFSET, frame.Width() - X_OFFSET, _WIDTH);
	// top	
	msg = new BMessage(TOP_MARGIN_CHANGED);
	//sprintf(str, "%2.2f", margins.top/unitValue);
	str << margins.top/unitValue;
	top = new BTextControl( r, "top", "Top", str.String(), msg,
				B_FOLLOW_RIGHT);
	top->SetDivider(be_plain_font->StringWidth("Top#"));
	top->SetTarget(marginMgr);
	AllowOnlyNumbers(top, NUM_COUNT);
	AddChild(top);
	//left
	r.OffsetBy(0, Y_OFFSET);
	r.left = frame.Width() - be_plain_font->StringWidth("Left#") - _WIDTH;
	//sprintf(str, "%2.2f", margins.left/unitValue);
    str = "";	
	str << margins.left/unitValue;
	msg = new BMessage(LEFT_MARGIN_CHANGED);
	left = new BTextControl( r, "left", "Left", str.String(), msg,
				B_FOLLOW_RIGHT);	
	left->SetDivider(be_plain_font->StringWidth("Left#"));
	left->SetTarget(marginMgr);
	AllowOnlyNumbers(left, NUM_COUNT);
	AddChild(left);
	//bottom
	r.OffsetBy(0, Y_OFFSET);
	r.left = frame.Width() - be_plain_font->StringWidth("Bottom#") - _WIDTH;
	//sprintf(str, "%2.2f", margins.bottom/unitValue);
    str = "";	
	str << margins.bottom/unitValue;
	msg = new BMessage(BOTTOM_MARGIN_CHANGED);
	bottom = new BTextControl( r, "bottom", "Bottom", str.String(), msg,
				B_FOLLOW_RIGHT);
	bottom->SetDivider(be_plain_font->StringWidth("Bottom#"));
	bottom->SetTarget(marginMgr);
	AllowOnlyNumbers(bottom, NUM_COUNT);
	AddChild(bottom);
	//right
	r.OffsetBy(0, Y_OFFSET);
	r.left = frame.Width() - be_plain_font->StringWidth("Right#") - _WIDTH;
	//sprintf(str, "%2.2f", margins.right/unitValue);
    str = "";	
	str << margins.right/unitValue;
	msg = new BMessage(RIGHT_MARGIN_CHANGED);
	right = new BTextControl( r, "right", "Right", str.String(), msg,
				B_FOLLOW_RIGHT);
	right->SetDivider(be_plain_font->StringWidth("Right#"));
	right->SetTarget(marginMgr);
	AllowOnlyNumbers(right, NUM_COUNT);
	AddChild(right);

// Create Units popup
	r.OffsetBy(-X_OFFSET,Y_OFFSET);
	r.right += Y_OFFSET;

	menu = new BPopUpMenu("units");
	mf = new BMenuField(r, "units", "Units", menu,
			B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT|B_WILL_DRAW);
	mf->ResizeToPreferred();
	mf->SetDivider(be_plain_font->StringWidth("Units#"));
	
	// Construct menu items 
	for (int i=0; unitNames[i] != NULL; i++ ) 
	{
		msg = new BMessage(unitMsg[i]);
		menu->AddItem(item = new BMenuItem(unitNames[i], msg));
		item->SetTarget(marginMgr);
		if (units == unitMsg[i]) {
			item->SetMarked(true);
		}
	}
	AddChild(mf);

	// calculate the sizes for drawing page view
	CalculateViewSize(MARGIN_CHANGED);
}

/**
 * Destructor
 *
 * @param none
 * @return void
 */
MarginView::~MarginView() {
	marginMgr->Lock();
	marginMgr->Quit();
}

/*----------------- MarginView Public BeOS Hook Methods --------------------*/

/**
 * Draw
 *
 * @param BRect, the draw bounds
 * @return void
 */
void MarginView::Draw(BRect rect)
{
	BBox::Draw(rect);

	float y_offset = (float)Y_OFFSET;
	float x_offset = (float)X_OFFSET;
	BRect r;

	// Calculate offsets depending on orientation
	if (pageWidth < pageHeight) { // Portrait
		x_offset = (maxPageWidth/2 + X_OFFSET) - viewWidth/2;
	} else { // landscape
		y_offset = (maxPageHeight/2 + Y_OFFSET) - viewHeight/2;
	}
	
	// draw the page
	SetHighColor(white);
	r = BRect(0, 0, viewWidth, viewHeight);
	r.OffsetBy(x_offset, y_offset);
	FillRect(r);
	SetHighColor(black);
	StrokeRect(r);

	// draw margin
	SetHighColor(red);
	SetLowColor(white);
	r.top += margins.top;
	r.right -= margins.right;
	r.bottom -= margins.bottom;
	r.left += margins.left;
	StrokeRect(r, dots);

	// draw the page size label
	SetHighColor(black);
	SetLowColor(gray);
	char str[STRING_SIZE];
	sprintf(str, "%2.1f x %2.1f", pageWidth/unitValue, pageHeight/unitValue); 
	SetFontSize(10);
	DrawString((const char *)str, BPoint(x_offset, maxPageHeight + 40));
}


/**
 * BeOS Hook Function, change the size of the margin display
 *
 * @param width of the page
 * @param  height the page
 * @return void
 */
void MarginView::FrameResized(float width, float height)
{
	maxPageHeight = height - _minUnitHeight - X_OFFSET;
	maxPageWidth = width - _minFieldWidth - Y_OFFSET;

	CalculateViewSize(MARGIN_CHANGED);
	Invalidate();
}

/**
 * AttachToWindow
 *
 * @param none
 * @return void
 */
void MarginView::AttachedToWindow()
{
	if (Parent()) {
		SetViewColor(Parent()->ViewColor());
	}
}

/*----------------- MarginView Public Methods --------------------*/

/**
 * GetMessageHandler, returns the BLooper of the MarginManager so that
 * 	you can send messages to this component.
 *
 * @param none
 * @return BLooper*, the margin manager
 */
BLooper* MarginView::GetMessageHandler() { 
	return static_cast<BLooper*>(marginMgr); 
}

/**
 * GetUnits
 *
 * @param none
 * @return uint32 enum, units in inches, cm, points
 */
uint32 MarginView::GetUnits(void) {
	return units;
}

/**
 * UpdateView, recalculate and redraw the view
 *
 * @param msg is a message to the calculate size to tell which field caused 
 *		the update to occur, or it is a general update.
 * @return void
 */
void MarginView::UpdateView(uint32 msg)
{
	Window()->Lock();
	CalculateViewSize(msg);
	Invalidate();
	Window()->Unlock();
}

/**
 * SetPageSize
 *
 * @param pageWidth, float that is the unit value of the page width
 * @param pageHeight, float that is the unit value of the page height
 * @return void
 */
void MarginView::SetPageSize(float pageWidth, float pageHeight)
{
	this->pageWidth = pageWidth;
	this->pageHeight = pageHeight; 
}

/**
 * GetPageSize
 *
 * @param none
 * @return BPoint, contains actual point values of page in x, y of point
 */
BPoint MarginView::GetPageSize(void) {
	return BPoint(pageWidth, pageHeight);
}

/**
 * GetMargin
 *
 * @param none
 * @return rect, return margin values always in points
 */
BRect MarginView::GetMargin(void) 
{
	BRect margin;

	// convert the field text to values
	float ftop 		= atof(top->Text());
	float fright 	= atof(right->Text());
	float fleft 	= atof(left->Text());
	float fbottom 	= atof(bottom->Text());

	// convert to units to points
	switch (units)  
	{	
		case UNIT_INCH:
			// convert to points
			ftop *= _inchUnits;
			fright *= _inchUnits;
			fleft *= _inchUnits;
			fbottom *= _inchUnits;
			break;
		case UNIT_CM:
			// convert to points
			ftop *= _cmUnits;
			fright *= _cmUnits;
			fleft *= _cmUnits;
			fbottom *= _cmUnits;
			break;
	}
	
	margin.Set(fleft, ftop, fright, fbottom);
	
	return margin;
}

/*----------------- MarginView Private Methods --------------------*/


/**
 * AllowOnlyNumbers()
 *
 * @param BTextControl, the control we want to only allow numbers
 * @param maxNum, the maximun number of characters allowed
 * @return void
 */
void MarginView::AllowOnlyNumbers(BTextControl *textControl, int maxNum)
{
	BTextView *tv = textControl->TextView();

	for (long i = 0; i < 256; i++) {
		tv->DisallowChar(i); 
	}
	for (long i = '0'; i <= '9'; i++) {
		tv->AllowChar(i); 
	}
	tv->AllowChar(B_BACKSPACE);
	tv->AllowChar('.');
	tv->SetMaxBytes(maxNum);
}

/**
 * SetMargin
 *
 * @param brect, margin values in rect
 * @return void
 */
void  MarginView::SetMargin(BRect margin) {
	this->margins = margins;
}

/**
 * SetUnits, called by the MarginMgr when the units popup is selected
 *
 * @param uint32, the enum that identifies the units requested to change to.
 * @return void
 */
void MarginView::SetUnits(uint32 unit)
{
	// do nothing if the current units are the same as requested
	if (unit == units) {
		return;
	}
	
	// set the units Format
	unitValue = unitFormat[unit];

	// convert the field text to values
	float ftop 		= atof(top->Text());
	float fright 	= atof(right->Text());
	float fleft 	= atof(left->Text());
	float fbottom 	= atof(bottom->Text());

	// convert to target units
	switch (units)  
	{	
		case UNIT_INCH:
			// convert to points
			ftop *= _inchUnits;
			fright *= _inchUnits;
			fleft *= _inchUnits;
			fbottom *= _inchUnits;
			// check for target unit is cm
			if (unit == UNIT_CM) {
				ftop /= _cmUnits;
				fright /= _cmUnits;
				fleft /= _cmUnits;
				fbottom /= _cmUnits;
			}
			break;
		case UNIT_CM:
			// convert to points
			ftop *= _cmUnits;
			fright *= _cmUnits;
			fleft *= _cmUnits;
			fbottom *= _cmUnits;
			// check for target unit is inches
			if (unit == UNIT_INCH) {
				ftop /= _inchUnits;
				fright /= _inchUnits;
				fleft /= _inchUnits;
				fbottom /= _inchUnits;
			}
			break;
		case UNIT_POINT:
			// check for target unit is cm
			if (unit == UNIT_CM) {
				ftop /= _cmUnits;
				fright /= _cmUnits;
				fleft /= _cmUnits;
				fbottom /= _cmUnits;
			}
			// check for target unit is inches
			if (unit == UNIT_INCH) {
				ftop /= _inchUnits;
				fright /= _inchUnits;
				fleft /= _inchUnits;
				fbottom /= _inchUnits;
			}
			break;
	}
	units = unit;
	
	// lock Window since these changes are from another thread
	Window()->Lock();
	
	// set the fields to new units
	BString str;
	str << ftop; 
	top->SetText(str.String());
	
	str = "";	
	str << fleft; 
	left->SetText(str.String());
	
	str = "";	
	str << fright; 
	right->SetText(str.String());
	
	str = "";	
	str << fbottom; 
	bottom->SetText(str.String());

	// update UI
	CalculateViewSize(MARGIN_CHANGED);
	Invalidate();
	
	Window()->Unlock();
}

/**
 * CalculateViewSize
 *
 * calculate the size of the view that is used
 *	to show the page inside the margin box. This is dependent 
 *	on the size of the box and the room we have to show it and
 *	the units that we are using and the orientation of the page.
 *	
 * @param msg, the message for which field changed to check value bounds 
 * @return void
 */
void MarginView::CalculateViewSize(uint32 msg)
{
	// determine page orientation 	
	if (pageHeight < pageWidth) { // LANDSCAPE
		viewWidth = maxPageWidth;
		viewHeight = pageHeight * (viewWidth/pageWidth);
		float hdiff = viewHeight - maxPageHeight;
		if (hdiff > 0) {
			viewHeight -= hdiff;
			viewWidth -= hdiff;
		}
	} else { // PORTRAIT
		viewHeight = maxPageHeight;
		viewWidth = pageWidth * (viewHeight/pageHeight);
		float wdiff = viewWidth - maxPageWidth;
		if (wdiff > 0) {
			viewHeight -= wdiff;
			viewWidth -= wdiff;
		}
	}

	// calculate margins based on view size
	
	// find the length of 1 pixel in points
	// 	ex: 80px/800pt = 0.1px/pt 
	float pixelLength = viewHeight/pageHeight;
	
	// convert the margins to points
	// The text field will have a number that us in the current unit
	//  ex 0.2" * 72pt = 14.4pts
	float ftop = atof(top->Text()) * unitValue;
	float fright = atof(right->Text()) * unitValue;
	float fbottom = atof(bottom->Text()) * unitValue;
	float fleft = atof(left->Text()) * unitValue;

	// Check that the margins don't overlap each other...
	float ph = pageHeight;
	float pw = pageWidth;
 	BString str;

	//  Bounds calculation rules: 	
	if (msg == TOP_MARGIN_CHANGED) 
	{
		//	top must be <= bottom
		if (ftop > (ph - fbottom)) {
			ftop = ph - fbottom;
			str = "";	
			str << ftop / unitValue;
			Window()->Lock();
			top->SetText(str.String());
			Window()->Unlock();	
		}

	}

	if (msg == BOTTOM_MARGIN_CHANGED) 
	{
		//	bottom must be <= pageHeight 
		if (fbottom > (ph - ftop)) {
			fbottom = ph - ftop;
			str = "";	
			str << fbottom / unitValue;
			Window()->Lock();
			bottom->SetText(str.String());
			Window()->Unlock();	
		}
	}
	
	if (msg == LEFT_MARGIN_CHANGED) 
	{
		// 	left must be <= right  
		if (fleft > (pw - fright)) {
			fleft = pw - fright;
			str = "";	
			str << fleft / unitValue;
			Window()->Lock();
			left->SetText(str.String());
			Window()->Unlock();	
		}
	}
	
	if (msg == RIGHT_MARGIN_CHANGED) 
	{
		//	right must be <= pageWidth 
		if (fright > (pw - fleft)) {
			fright = pw - fleft;
			str = "";	
			str << fright / unitValue;
			Window()->Lock();
			right->SetText(str.String());
			Window()->Unlock();	
		}
	}
	
	// convert the unit value to pixels
	//  ex: 14.4pt * 0.1px/pt = 1.44px
	margins.top = ftop * pixelLength;
	margins.right = fright * pixelLength;
	margins.bottom = fbottom * pixelLength;
	margins.left = fleft * pixelLength;
}

/*----------------- MarginManager --------------------*/

/**
 * Constructor for MarginManager class
 *
 * @param MarginView*, the view we are using to proxy messages.
 * @return void
 */
MarginManager::MarginManager(MarginView *view) 
{
	mv = view;
	Run();
}

/**
 * Destructor for MarginManager class
 *
 * @param none
 * @return void
 */
MarginManager::~MarginManager() { 
}

/**
 * MesssageReceived()
 *
 * Proxy class to receive messages for the view that does not have a BLooper
 *
 * @param BMessage* , the message being received
 * @return void
 */
void MarginManager::MessageReceived(BMessage *msg)
{
	switch (msg->what) 
	{	
		case CHANGE_PAGE_SIZE:
			float w;
			float h;
			msg->FindFloat("width", &w);
			msg->FindFloat("height", &h);
			mv->SetPageSize(w, h);
			mv->UpdateView(MARGIN_CHANGED);
			break;

		case FLIP_PAGE:
			{	
				BPoint p;
				p = mv->GetPageSize();
				mv->SetPageSize(p.y, p.x);
				mv->UpdateView(MARGIN_CHANGED);
			}
			break;
			
		case MARGIN_CHANGED:
			mv->UpdateView(MARGIN_CHANGED);
			break;
		
		case TOP_MARGIN_CHANGED:
			mv->UpdateView(TOP_MARGIN_CHANGED);
			break;
		
		case LEFT_MARGIN_CHANGED:
			mv->UpdateView(LEFT_MARGIN_CHANGED);
			break;
		
		case RIGHT_MARGIN_CHANGED:
			mv->UpdateView(RIGHT_MARGIN_CHANGED);
			break;
		
		case BOTTOM_MARGIN_CHANGED:
			mv->UpdateView(BOTTOM_MARGIN_CHANGED);
			break;
		
		case MarginView::UNIT_INCH:
		case MarginView::UNIT_CM:
		case MarginView::UNIT_POINT:
			mv->SetUnits(msg->what);
			break;
			
		default:
			BLooper::MessageReceived(msg);
			break;
	}
}
