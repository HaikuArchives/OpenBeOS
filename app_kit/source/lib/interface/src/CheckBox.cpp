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
//	File Name:		CheckBox.cpp
//	Author:			Frans van Nispen (xlr8@tref.nl)
//	Description:	BCheckBox displays an on/off control.
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------
#include <stdio.h>

// System Includes -------------------------------------------------------------
#include <InterfaceDefs.h>
#include "../headers/CheckBox.h"
#include <Control.h>
#include <Window.h>
#include <Message.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

//------------------------------------------------------------------------------
BCheckBox::BCheckBox(BRect frame, const char* name, const char* label,
					 BMessage* message, uint32 resizeMask, uint32 flags)
	:	BControl(frame, name, label, message, resizeMask, flags),
		fOutlined(false),
		fPressed(false)
{
}
//------------------------------------------------------------------------------
BCheckBox::~BCheckBox()
{
}
//------------------------------------------------------------------------------
BCheckBox::BCheckBox(BMessage* data)
	:	BControl(data),
		fOutlined(false),
		fPressed(false)
{
}
//------------------------------------------------------------------------------
BArchivable* BCheckBox::Instantiate(BMessage* data)
{
	if (!validate_instantiation(data,"BCheckBox"))
	{
		return NULL;
	}

	return new BCheckBox(data);
}
//------------------------------------------------------------------------------
status_t BCheckBox::Archive(BMessage* data, bool deep) const
{
	BControl::Archive(data, deep);
	return B_OK;
}
//------------------------------------------------------------------------------
void BCheckBox::Draw(BRect updateRect)
{
	SetPenSize(1);

	// We should request the view's base color which normaly is (216,216,216)
	rgb_color color = ui_color(B_PANEL_BACKGROUND_COLOR);
	rgb_color col;

	// Draw background
	SetLowColor(ViewColor());

	// draw box
	BRect rect;
	rect.Set(1.0, 3.0, 13.0, 15.0);
	if (!IsEnabled())
	{
		rect.InsetBy(1,1);
	}

	SetHighColor(tint_color(color, IsEnabled() ?
				 B_LIGHTEN_MAX_TINT : B_LIGHTEN_1_TINT));
	FillRect(rect);

	float add1 = 0.0f, add2 = 0.0f;
	IsEnabled()	? add1 = 1.0f : add2 = 1.0f;

	BeginLineArray(6);

	col = tint_color(color, IsEnabled() ? B_DARKEN_1_TINT : B_DARKEN_2_TINT);
	AddLine(BPoint(rect.left, rect.bottom-add2),
			BPoint(rect.left, rect.top), col);
	AddLine(BPoint(rect.left, rect.top),
			BPoint(rect.right-add2, rect.top), col);
	col = tint_color(color, IsEnabled() ? B_DARKEN_4_TINT : B_LIGHTEN_2_TINT);
	rect.InsetBy(1,1);
	AddLine(BPoint(rect.left, rect.bottom),
			BPoint(rect.left, rect.top), col);
	AddLine(BPoint(rect.left, rect.top),
			BPoint(rect.right, rect.top), col);
	col = tint_color(color, IsEnabled() ? B_NO_TINT : B_DARKEN_1_TINT);
	AddLine(BPoint(rect.left+add1, rect.bottom),
			BPoint(rect.right, rect.bottom), col);
	AddLine(BPoint(rect.right, rect.bottom),
			BPoint(rect.right, rect.top+add1), col);

	EndLineArray();
	
	rect.InsetBy(2,2);
	if (Value() == B_CONTROL_ON)
	{	
		SetHighColor(tint_color(ui_color(B_KEYBOARD_NAVIGATION_COLOR),
					 IsEnabled() ? B_NO_TINT : B_LIGHTEN_1_TINT));
		SetPenSize(2);
		StrokeLine(BPoint(rect.left, rect.top),
				   BPoint(rect.right, rect.bottom));
		StrokeLine(BPoint(rect.left, rect.bottom),
				   BPoint(rect.right, rect.top));
		SetPenSize(1);
	}
	
	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);

	float h = ceil(fh.ascent + fh.descent + fh.leading) + 3.0f;

	SetHighColor(IsFocus() ? ui_color(B_KEYBOARD_NAVIGATION_COLOR) : ViewColor());
	BRect rect2(BPoint(18.0f, 3.0f),
				BPoint(22.0f + font.StringWidth(BControl::Label()), h));
	StrokeRect(rect2, B_MIXED_COLORS);

	SetHighColor(tint_color(color, IsEnabled() ?
				 B_DARKEN_MAX_TINT : B_DISABLED_LABEL_TINT));
	DrawString(BControl::Label(), BPoint( 21.0f, 3.0f + font.Size()));

	if (fOutlined)
	{
		rect.Set(1.0, 3.0, 13.0, 15.0);
		if (!IsEnabled())
		{
			rect.InsetBy(1,1);
		}
		SetHighColor(tint_color(color, B_DARKEN_3_TINT));
		StrokeRect(rect);
	}
}
//------------------------------------------------------------------------------
void BCheckBox::MouseDown(BPoint where)
{
	if (!IsEnabled())
	{
		BView::MouseDown(where);
		return;
	}

	SetMouseEventMask(B_POINTER_EVENTS,	B_NO_POINTER_HISTORY |
					  B_SUSPEND_VIEW_FOCUS);

	MakeFocus();
	fOutlined = true;
	fPressed = true;
	Draw(Bounds());
}
//------------------------------------------------------------------------------
void BCheckBox::MouseUp(BPoint pt)
{
	if (IsEnabled() && fPressed)
	{
		fOutlined = false;

		if (Bounds().Contains(pt))
		{
			if (Value() == B_CONTROL_OFF)
			{
				SetValue(B_CONTROL_ON);
			}
			else
			{
				SetValue(B_CONTROL_OFF);
			}
	
			BControl::Invoke();
		}
		fPressed = false;
	}
	else
	{
		BView::MouseUp( pt );
		return;
	}
}
//------------------------------------------------------------------------------
void BCheckBox::MouseMoved(BPoint pt, uint32 code, const BMessage* msg)
{
	if (IsEnabled() && fPressed)
	{
		if (code == B_EXITED_VIEW)
		{
			fOutlined = false;
		}
		else
		{
			if (code == B_ENTERED_VIEW)
			{
				fOutlined = true;
			}
		}
		Draw(Bounds());
	}
	else
	{
		BView::MouseMoved(pt, code, msg);
		return;
	}
}
//------------------------------------------------------------------------------
void BCheckBox::KeyDown(const char* bytes, int32 numBytes)
{
	BControl::KeyDown(bytes, numBytes);
}
//------------------------------------------------------------------------------
void BCheckBox::AttachedToWindow()
{
	BControl::AttachedToWindow();

	// resize to minimum height (BeOS uses 24)
	if (Bounds().Height() < 18.0f)
	{
		ResizeTo( Bounds().Width(), 18.0f);
	}

	Draw(Bounds());
}
//------------------------------------------------------------------------------
void BCheckBox::GetPreferredSize(float* width, float* height)
{
	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);

	*height = ceil(fh.ascent + fh.descent + fh.leading) + 6.0f;
	*width = 22.0f + ceil(font.StringWidth(BControl::Label()));
}
//------------------------------------------------------------------------------
void BCheckBox::ResizeToPreferred()
{
	float w, h;
	GetPreferredSize(&w, &h);
	BView::ResizeTo(w,h);
}
//------------------------------------------------------------------------------
void BCheckBox::MessageReceived(BMessage* msg)
{
	BControl::MessageReceived(msg);
}
//------------------------------------------------------------------------------
void BCheckBox::SetValue(int32 value)
{
	if (BControl::Value() == value)
	{
		return;
	}
	if (value == B_CONTROL_OFF)
	{
		BControl::SetValue(B_CONTROL_OFF);
	}
	else
	{
		BControl::SetValue(B_CONTROL_ON);
	}
}
//------------------------------------------------------------------------------
status_t BCheckBox::Invoke(BMessage* msg)
{
	return BControl::Invoke(msg);
}
//------------------------------------------------------------------------------
void BCheckBox::MakeFocus(bool state)
{
	BControl::MakeFocus();
}
//------------------------------------------------------------------------------
void BCheckBox::AllAttached()
{
	BControl::AllAttached();
}
//------------------------------------------------------------------------------
void BCheckBox::AllDetached()
{
	BControl::AllDetached();
}
//------------------------------------------------------------------------------
void BCheckBox::FrameMoved(BPoint new_position)
{
	BControl::FrameMoved(new_position);
}
//------------------------------------------------------------------------------
void BCheckBox::FrameResized(float new_width, float new_height)
{
	BControl::FrameResized(new_width, new_height);
}
//------------------------------------------------------------------------------
void BCheckBox::WindowActivated(bool state)
{
	BControl::WindowActivated(state);
}
//------------------------------------------------------------------------------
void BCheckBox::DetachedFromWindow()
{
	BControl::DetachedFromWindow();
}
//------------------------------------------------------------------------------
BHandler* BCheckBox::ResolveSpecifier(BMessage* msg, int32 index,
									  BMessage* specifier, int32 form,
									  const char* property)
{
	return NULL;
}
//------------------------------------------------------------------------------
status_t BCheckBox::GetSupportedSuites(BMessage* data)
{
	return B_OK;
}
//------------------------------------------------------------------------------
status_t BCheckBox::Perform(perform_code d, void* arg)
{
	return B_ERROR;
}
//------------------------------------------------------------------------------
void BCheckBox::_ReservedCheckBox1()
{
}
//------------------------------------------------------------------------------
void BCheckBox::_ReservedCheckBox2()
{
}
//------------------------------------------------------------------------------
void BCheckBox::_ReservedCheckBox3()
{
}
//------------------------------------------------------------------------------
BCheckBox& BCheckBox::operator=(const BCheckBox&)
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

