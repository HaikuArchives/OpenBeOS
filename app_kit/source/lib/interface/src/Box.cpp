//------------------------------------------------------------------------------
//	Copyright (c) 2001-2002, OpenBeOS
//
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//
//	File Name:		Box.cpp
//	Author:			Frans van Nispen (xlr8@tref.nl)
//	Description:	BBox objects group views together and draw a border
//					around them.
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------
#include <stdio.h>

// System Includes -------------------------------------------------------------
#include <Box.h>
#include <Control.h>
#include <InterfaceDefs.h>
#include <Window.h>
#include <Message.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------


//------------------------------------------------------------------------------
BBox::BBox(BRect frame, const char *name, uint32 resizeMask, uint32 flags,
		   border_style border)
	:	BView(frame, name, resizeMask, flags),
		fLabel(NULL),
		fStyle(border),
		fLabelView(NULL)
{
	SetFont(be_bold_font);
}
//------------------------------------------------------------------------------
BBox::~BBox()
{
	if (fLabelView)
	{
		fLabelView->RemoveSelf();
	}

	if (fLabel)
	{
		delete[] fLabel;
	}
}
//------------------------------------------------------------------------------
BBox::BBox(BMessage *data)
	:	BView(data)
{
	const char *label;

	if (data->FindInt32("_style", (int32&)fStyle) != B_OK)
	{
		fStyle = B_FANCY_BORDER;
	}
	if (data->FindString("_label", &label) != B_OK)
	{
		label = NULL;
	}

	SetLabel(label);
}
//------------------------------------------------------------------------------
BArchivable* BBox::Instantiate(BMessage* data)
{
	if (!validate_instantiation(data,"BBox"))
	{
		return NULL;
	}

	return new BBox(data);
}
//------------------------------------------------------------------------------
status_t BBox::Archive(BMessage* data, bool deep) const
{
	BView::Archive(data, deep);

	if (fLabel)
	{
		data->AddString("_label",fLabel);
	}
	data->AddInt32("_style", fStyle);

	return B_OK;
}
//------------------------------------------------------------------------------
void BBox::SetBorder(border_style style)
{
	fStyle = style;
	Invalidate();
}
//------------------------------------------------------------------------------
border_style BBox::Border() const
{
	return fStyle;
}
//------------------------------------------------------------------------------
void BBox::SetLabel(const char* label)
{
	if (fLabel)
		delete[] fLabel;
	fLabel = new char[strlen(label)+1];
	strcpy(fLabel, label);

	Invalidate();
}
//------------------------------------------------------------------------------
status_t BBox::SetLabel(BView* view_label)
{
	if (view_label){
		fLabelView = view_label;
		AddChild(fLabelView);
		fLabel = NULL;
	}
	Invalidate();
	return B_OK;
}
//------------------------------------------------------------------------------
const char* BBox::Label() const
{
	return fLabel;
}
//------------------------------------------------------------------------------
BView* BBox::LabelView() const
{
	return fLabelView;
}
//------------------------------------------------------------------------------
void BBox::Draw(BRect bounds)
{
	BRect rect = Bounds();

	SetLowColor(ViewColor());

	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);

	rect.top = (fh.ascent + fh.descent)/2.0f;

	switch (fStyle)
	{
		case B_FANCY_BORDER:
			SetHighColor(tint_color(ViewColor(), B_LIGHTEN_MAX_TINT));
			rect.left++;	rect.top++;
			StrokeRect(rect);
			SetHighColor(tint_color(ViewColor(), B_DARKEN_3_TINT));
			rect.OffsetBy(-1,-1);
			StrokeRect(rect);
			break;

		case B_PLAIN_BORDER:
			rect.top--;
			SetHighColor(tint_color(ViewColor(), B_LIGHTEN_MAX_TINT));
			StrokeLine(BPoint(rect.left, rect.bottom), BPoint(rect.left, rect.top));
			StrokeLine(BPoint(rect.left+1.0f, rect.top), BPoint(rect.right, rect.top));
			SetHighColor(tint_color(ViewColor(), B_DARKEN_3_TINT));
			StrokeLine(BPoint(rect.left+1.0f, rect.bottom), BPoint(rect.right, rect.bottom));
			StrokeLine(BPoint(rect.right, rect.bottom), BPoint(rect.right, rect.top+1.0f));
			break;

		default:
			break;
	}
	
	SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_MAX_TINT));
	if (fLabel)
	{
		rect.Set(6.0f, 1.0f, 12.0f + font.StringWidth(fLabel),
				 fh.ascent + fh.descent);
		FillRect(rect, B_SOLID_LOW);
		DrawString(fLabel, BPoint(10.0f, ceil(fh.ascent - fh.descent) + 1.0f ));
	}
}
//------------------------------------------------------------------------------
void BBox::AttachedToWindow()
{
	if (Parent())
	{
		SetViewColor( Parent()->ViewColor() );
	}
	else
	{
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	}
}
//------------------------------------------------------------------------------
void BBox::ResizeToPreferred()
{
	BView::ResizeToPreferred();
}
//------------------------------------------------------------------------------
void BBox::GetPreferredSize(float* width, float* height)
{
	BRect r(0,0,99,99);

	if (Parent())
	{
		r = Parent()->Bounds();
		r.InsetBy(10,10);
	}
	
	*width = r.Width();
	*height = r.Height();
}
//------------------------------------------------------------------------------
void BBox::DetachedFromWindow()
{
	BView::DetachedFromWindow();
}
//------------------------------------------------------------------------------
void BBox::AllAttached()
{
	BView::AllAttached();
}
//------------------------------------------------------------------------------
void BBox::AllDetached()
{
	BView::AllDetached();
}
//------------------------------------------------------------------------------
void BBox::FrameResized(float new_width, float new_height)
{
	BView::FrameResized(new_width, new_height);
}
//------------------------------------------------------------------------------
void BBox::MessageReceived(BMessage *msg)
{
	BView::MessageReceived(msg);
}
//------------------------------------------------------------------------------
void BBox::MouseDown(BPoint pt)
{
	BView::MouseDown(pt);
}
//------------------------------------------------------------------------------
void BBox::MouseUp(BPoint pt)
{
	BView::MouseUp(pt);
}
//------------------------------------------------------------------------------
void BBox::WindowActivated(bool state)
{
	BView::WindowActivated(state);
}
//------------------------------------------------------------------------------
void BBox::MouseMoved(BPoint pt, uint32 code, const BMessage* msg)
{
	BView::MouseMoved(pt, code, msg);
}
//------------------------------------------------------------------------------
void BBox::FrameMoved(BPoint new_position)
{
	BView::FrameMoved(new_position);
}
//------------------------------------------------------------------------------
BHandler* BBox::ResolveSpecifier(BMessage* msg, int32 index,
								 BMessage* specifier, int32 form,
								 const char* property)
{
	return NULL;
}
//------------------------------------------------------------------------------
void BBox::MakeFocus(bool state)
{
	BView::MakeFocus(state);
}
//------------------------------------------------------------------------------
status_t BBox::GetSupportedSuites(BMessage* data)
{
	return B_OK;
}
//------------------------------------------------------------------------------
status_t BBox::Perform(perform_code d, void* arg)
{
	return B_ERROR;
}
//------------------------------------------------------------------------------
void BBox::_ReservedBox1()
{
}
//------------------------------------------------------------------------------
void BBox::_ReservedBox2()
{
}
//------------------------------------------------------------------------------
BBox& BBox::operator=(const BBox&)
{
	// Assignment not allowed
	return *this;
}
//------------------------------------------------------------------------------

/*
 * $Log $
 *
 * $Id  $
 *
 */

