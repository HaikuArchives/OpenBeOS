/*

MarginView.cpp

Copyright (c) 2001 OpenBeOS.

Author: Simon Gauvin

Version 12.13.2001

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

	1. New Text Control Class that:
		- block character entry in KeyDown
		- block negative numbers
		- single number should be decimalized in inches and cm units
		- click in field selects entire field
	1.1 Check the margin sizes are not outside bounds
	2: Make Strings in UI resources
	3. Remove hard pixel values in the GUI creation code
	4. Mouse margin changes.
	
*/

#ifndef HELLO_VIEW_H
#include "MarginView.h"
#endif

#include <AppKit.h>
#include <stdio.h>
#include <stdlib.h>

/*----------------- MarginView Constants --------------------*/

const static float _pointUnits = 1; // 1 point = 1 point 
const static float _inchUnits = 72; // 1" = 72 points
const static float _cmUnits = 28.346; // 72/2.54 1cm = 28.346 points

const static float _minFieldWidth = 100; // pixels
const static float _minUnitHeight = 30; // pixels
const static float _drawInset = 10; // pixels
	
const static float unitFormat[] = { _inchUnits, _cmUnits, _pointUnits }; 
const pattern dots = {{ 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 }};

const rgb_color black = { 0,0,0,0 };
const rgb_color blue = { 0,0,255,0 };
const rgb_color red = { 255,0,0,0 };
const rgb_color white = { 255,255,255,0 };
const rgb_color gray = { 220,220,220,0 };

/*----------------- MarginView Public Methods --------------------*/

/**
 * Constructor
 *
 * @param none
 * @return void
 */
MarginView::MarginView(BRect rect, // frame
		int32 pageWidth,
		int32 pageHeight,
		BRect margins,
		uint32 units)
		
	   	:BBox(rect, NULL, B_FOLLOW_ALL)
{
	this->units = units;
	unitValue = unitFormat[units];
	
	SetLabel("Margins");
	
	maxPageHeight = rect.Height() - _minUnitHeight - 20;
	maxPageWidth = rect.Width() - _minFieldWidth - 10;

	this->margins = margins;
		
	this->pageWidth = pageWidth;
	this->pageHeight = pageHeight; 

	// Create the BLooper to handle unit menu messages
	marginMgr = new MarginManager(this);

// Create text fields
	char str[100];
	BMessage *msg;
	msg = new BMessage(MARGIN_CHANGED);
	BRect r(rect.Width() - be_plain_font->StringWidth("Top#") - 50, 
			20, 
			rect.Width() - 10, 
			50);
	// top	
	msg = new BMessage(MARGIN_CHANGED);
	sprintf(str, "%2.2f", margins.top);
	top = new BTextControl( r, "top", "Top", (const char *)str, msg,
				B_FOLLOW_RIGHT);
	top->SetDivider(be_plain_font->StringWidth("Top#"));
	top->SetTarget(marginMgr);
	AddChild(top);
	//left
	r.OffsetBy(0,20);
	r.left = rect.Width() - be_plain_font->StringWidth("Left#") - 50;
	sprintf(str, "%2.2f", margins.left);
	msg = new BMessage(MARGIN_CHANGED);
	left = new BTextControl( r, "left", "Left", (const char *)str, msg,
				B_FOLLOW_RIGHT);	
	left->SetDivider(be_plain_font->StringWidth("Left#"));
	left->SetTarget(marginMgr);
	AddChild(left);
	//bottom
	r.OffsetBy(0,20);
	r.left = rect.Width() - be_plain_font->StringWidth("Bottom#") - 50;
	sprintf(str, "%2.2f", margins.bottom);
	msg = new BMessage(MARGIN_CHANGED);
	bottom = new BTextControl( r, "bottom", "Bottom", (const char *)str, msg,
				B_FOLLOW_RIGHT);
	bottom->SetDivider(be_plain_font->StringWidth("Bottom#"));
	bottom->SetTarget(marginMgr);
	AddChild(bottom);
	//right
	r.OffsetBy(0,20);
	r.left = rect.Width() - be_plain_font->StringWidth("Right#") - 50;
	sprintf(str, "%2.2f", margins.right);
	msg = new BMessage(MARGIN_CHANGED);
	right = new BTextControl( r, "right", "Right", (const char *)str, msg,
				B_FOLLOW_RIGHT);
	right->SetDivider(be_plain_font->StringWidth("Right#"));
	right->SetTarget(marginMgr);
	AddChild(right);

// Create Units popup
	r.OffsetBy(-10,30);
	r.right += 20;

	menu = new BPopUpMenu("units");
	mf = new BMenuField(r, "units", "Units", menu,
			B_FOLLOW_BOTTOM|B_FOLLOW_RIGHT|B_WILL_DRAW);
	mf->ResizeToPreferred();
	mf->SetDivider(be_plain_font->StringWidth("Units#"));
	
	// Inches
	msg = new BMessage(UNIT_INCH);
	menu->AddItem(item = new BMenuItem("Inches", msg));
	item->SetMarked(true);
	item->SetTarget(marginMgr);
	if (units == UNIT_INCH) {
		item->SetMarked(true);
		mf->MenuItem()->SetLabel(item->Label());	
	}

	// cm
	msg = new BMessage(UNIT_CM);
	menu->AddItem(item = new BMenuItem("cm", msg));
	item->SetTarget(marginMgr);
	if (units == UNIT_CM) {
		item->SetMarked(true);
		mf->MenuItem()->SetLabel(item->Label());	
	}
	
	// points
	msg = new BMessage(UNIT_POINT);
	menu->AddItem(item = new BMenuItem("Points", msg));
	item->SetTarget(marginMgr);
	if (units == UNIT_POINT) {
		item->SetMarked(true);
		mf->MenuItem()->SetLabel(item->Label());	
	}
	
	AddChild(mf);

	// calculate the sizes for drawing page view
	CalculateViewSize();
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

	float y_offset = 20.0;
	float x_offset = 10.0;
	BRect r;

	// Calculate offsets depending on orientation
	if (pageWidth < pageHeight) { // Portrait
		x_offset = (maxPageWidth/2 + 10) - viewWidth/2;
	} else { // landscape
		y_offset = (maxPageHeight/2 + 20) - viewHeight/2;
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
	char str[50];
	sprintf(str, "%2.1f x %2.1f", pageWidth/unitValue, pageHeight/unitValue); 
	SetFontSize(10);
	DrawString((const char *)str, BPoint(x_offset, maxPageHeight + 40));
}


/**
 * BeOS Hook Function, change the size of the margin display
 *
 * @param none
 * @return void
 */
void MarginView::FrameResized(float width, float height)
{
	maxPageHeight = height - _minUnitHeight - 20;
	maxPageWidth = width - _minFieldWidth - 10;

	CalculateViewSize();
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
 * @param none
 * @return void
 */
void MarginView::UpdateView(void)
{
	Window()->Lock();
	CalculateViewSize();
	Invalidate();
	Window()->Unlock();
}

/**
 * SetPageSize
 *
 * @param none
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
 * @return rect, return margin values allways in points
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
	
	margin.Set(fleft, ftop, fbottom, fright );
	
	return margin;
}

/*----------------- MarginView Private Methods --------------------*/

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
	
	// set the units
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
	char str[50];
	sprintf(str, "%2.2f", ftop); 
	top->SetText(str);
	sprintf(str, "%2.2f", fleft); 
	left->SetText(str);
	sprintf(str, "%2.2f", fright); 
	right->SetText(str);
	sprintf(str, "%2.2f", fbottom); 
	bottom->SetText(str);

	// update UI
	CalculateViewSize();
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
 * @param none
 * @return void
 */
void MarginView::CalculateViewSize(void)
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
	
	// TODO: Check that the margins don't overlap each other...
	
	// find the length of 1 pixel in points
	// 	ex: 80px/800pt = 0.1px/pt 
	float pixelLength = viewHeight/pageHeight;
	
	// convert the margins to points
	// The text field will have a number that us in the current unit
	//  ex 0.2" * 72pt = 14.4pts
	margins.top = atof(top->Text()) * unitValue;
	margins.right = atof(right->Text()) * unitValue;
	margins.bottom = atof(bottom->Text()) * unitValue;
	margins.left = atof(left->Text()) * unitValue;

	// convert the unit value to pixels
	//  ex: 14.4pt * 0.1px/pt = 1.44px
	margins.top *= pixelLength;
	margins.right *= pixelLength;
	margins.bottom *= pixelLength;
	margins.left *= pixelLength;
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
			mv->UpdateView();
			break;

		case FLIP_PAGE:
			{	
				BPoint p;
				p = mv->GetPageSize();
				mv->SetPageSize(p.y, p.x);
				mv->UpdateView();
			}
			break;
			
		case MARGIN_CHANGED:
			mv->UpdateView();
			break;
		
		case UNIT_INCH:
		case UNIT_CM:
		case UNIT_POINT:
			mv->SetUnits(msg->what);
			break;
			
		default:
			BLooper::MessageReceived(msg);
			break;
	}
}
