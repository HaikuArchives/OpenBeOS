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
//	File Name:		RadioButton.cpp
//	Author:			Frans van Nispen (xlr8@tref.nl)
//	Description:	BRadioButton represents a single on/off button.  All
//					sibling BRadioButton objects comprise a single
//					"multiple choice" control.
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------
#include <stdio.h>

// System Includes -------------------------------------------------------------
#include <Control.h>
#include <InterfaceDefs.h>
#include <Message.h>
#include <RadioButton.h>
#include <Window.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------


//------------------------------------------------------------------------------
BRadioButton::BRadioButton(BRect frame, const char* name, const char* label,
						   BMessage* message, uint32 mask, uint32 flags)
	:	BControl(frame, name, label, message, mask, flags),
		fOutlined(false),
		fPressed(false)
{
}
//------------------------------------------------------------------------------
BRadioButton::BRadioButton(BMessage* data)
	:	BControl(data),
		fOutlined(false),
		fPressed(false)
{
}
//------------------------------------------------------------------------------
BRadioButton::~BRadioButton()
{
}
//------------------------------------------------------------------------------
BArchivable* BRadioButton::Instantiate(BMessage* data)
{
	if (!validate_instantiation(data,"BRadioButton"))
	{
		return NULL;
	}

	return new BRadioButton(data);
}
//------------------------------------------------------------------------------
status_t BRadioButton::Archive(BMessage* data, bool deep = true) const
{
	BControl::Archive(data, deep);
	return B_OK;
}
//------------------------------------------------------------------------------
void BRadioButton::Draw(BRect updateRect)
{
	// We use the view's base color which is normally (216,216,216)
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

	FillRect(rect, B_SOLID_LOW);

	float add1 = 0.0f;
	float add2 = 0.0f;
	IsEnabled()	? add1 = 1.0f : add2 = 1.0f;

	BeginLineArray(6);

	col = tint_color(color, IsEnabled() ? B_DARKEN_1_TINT :
					 B_DARKEN_2_TINT);
	AddLine(BPoint(rect.left, rect.bottom - add2),
			BPoint(rect.left, rect.top), col);
	AddLine(BPoint(rect.left, rect.top),
			BPoint(rect.right - add2, rect.top), col);
	col = tint_color(color, IsEnabled() ? B_DARKEN_4_TINT :
					 B_LIGHTEN_2_TINT);
	rect.InsetBy(1, 1);
	AddLine(BPoint(rect.left, rect.bottom),
			BPoint(rect.left, rect.top), col);
	AddLine(BPoint(rect.left, rect.top),
			BPoint(rect.right, rect.top), col);
	col = tint_color(color, IsEnabled() ? B_NO_TINT : B_DARKEN_1_TINT);
	AddLine(BPoint(rect.left + add1, rect.bottom),
			BPoint(rect.right, rect.bottom), col);
	AddLine(BPoint(rect.right, rect.bottom),
			BPoint(rect.right, rect.top+add1), col);

	EndLineArray();
	
	rect.InsetBy(3, 3);
	if (Value() == B_CONTROL_ON)
	{
		SetHighColor(tint_color(ui_color(B_KEYBOARD_NAVIGATION_COLOR),
					 IsEnabled() ? B_NO_TINT : B_LIGHTEN_1_TINT));
		FillEllipse(rect);
		SetHighColor(tint_color(ui_color(B_KEYBOARD_NAVIGATION_COLOR),
								B_LIGHTEN_2_TINT));
		rect.InsetBy(-1, -1);
		StrokeEllipse(rect);
	}
	
	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);

	float h = ceil(fh.ascent + fh.descent + fh.leading) + 3.0f;

	SetHighColor(IsFocus() ? ui_color(B_KEYBOARD_NAVIGATION_COLOR) :
				 ViewColor() );
	BRect rect2(BPoint(18.0f, 3.0f),
				BPoint(22.0f + font.StringWidth(BControl::Label()), h));
	StrokeRect(rect2, B_MIXED_COLORS);

	SetHighColor(tint_color(color, IsEnabled() ?
				 B_DARKEN_MAX_TINT : B_DISABLED_LABEL_TINT) );
	DrawString(BControl::Label(), BPoint( 21.0f, 3.0f + font.Size()));

	if (fOutlined)
	{
		SetHighColor(tint_color(color, B_DARKEN_1_TINT));
		FillEllipse(rect);
	}
}
//------------------------------------------------------------------------------
void BRadioButton::MouseDown(BPoint where)
{
	if (!IsEnabled())
	{
		BView::MouseDown(where);
		return;
	}

	SetMouseEventMask(B_POINTER_EVENTS,	B_NO_POINTER_HISTORY |
					  B_SUSPEND_VIEW_FOCUS);

	MakeFocus();
	fPressed = true;
	fOutlined = true;
	Invalidate();
}
//------------------------------------------------------------------------------
void BRadioButton::AttachedToWindow()
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
void BRadioButton::KeyDown(const char* bytes, int32 numBytes)
{
	BControl::KeyDown(bytes, numBytes);
}
//------------------------------------------------------------------------------
void BRadioButton::SetValue(int32 value)
{
	if (BControl::Value() == value)
	{
		return;
	}

	if (!fPressed)
	{
		BView* parent = Parent();
		if (parent)
		{
			BView* sibling;
			int32 i = 0;
			while ((sibling = parent->ChildAt(i++)) != NULL)
			{
				if (sibling == this)
				{
					continue;
				}
		
				BRadioButton* radio = dynamic_cast<BRadioButton*>(sibling);
				if (radio != NULL)
				{
					radio->BControl::SetValue(B_CONTROL_OFF);
				}
			}
		}
	}

	BControl::SetValue(B_CONTROL_ON);
}
//------------------------------------------------------------------------------
void BRadioButton::GetPreferredSize(float* width, float* height)
{
	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);

	*height = ceil(fh.ascent + fh.descent + fh.leading) + 6.0f;
	*width = 22.0f + ceil(font.StringWidth(BControl::Label()));
}
//------------------------------------------------------------------------------
void BRadioButton::ResizeToPreferred()
{
	float w;
	float h;
	GetPreferredSize(&w, &h);
	BView::ResizeTo(w, h);
}
//------------------------------------------------------------------------------
status_t BRadioButton::Invoke(BMessage *msg = NULL)
{
	return BControl::Invoke(msg);
}
//------------------------------------------------------------------------------
void BRadioButton::MessageReceived(BMessage *msg)
{
	BControl::MessageReceived(msg);
}
//------------------------------------------------------------------------------
void BRadioButton::WindowActivated(bool state)
{
	BControl::WindowActivated(state);
}
//------------------------------------------------------------------------------
void BRadioButton::MouseUp(BPoint pt)
{
	if (IsEnabled() && fPressed)
	{
		fOutlined = false;
		fPressed = false;

		if (Bounds().Contains(pt))
		{
			SetValue(B_CONTROL_ON);
			Invoke();
		}
	}
	else
	{
		BView::MouseUp(pt);
		return;
	}
	Invalidate();
}
//------------------------------------------------------------------------------
void BRadioButton::MouseMoved(BPoint pt, uint32 code, const BMessage* msg)
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
void BRadioButton::DetachedFromWindow()
{
	BControl::DetachedFromWindow();
}
//------------------------------------------------------------------------------
void BRadioButton::FrameMoved(BPoint newPosition)
{
	BControl::FrameMoved(newPosition);
}
//------------------------------------------------------------------------------
void BRadioButton::FrameResized(float newWidth, float newHeight)
{
	BControl::FrameResized(newWidth, newHeight);
}
//------------------------------------------------------------------------------
BHandler *BRadioButton::ResolveSpecifier(BMessage *msg, int32 index, BMessage *specifier, int32 form, const char *property)
{
	return NULL;
}
//------------------------------------------------------------------------------
void BRadioButton::MakeFocus(bool state = true)
{
	BControl::MakeFocus();
}
//------------------------------------------------------------------------------
void BRadioButton::AllAttached()
{
	BControl::AllAttached();
}
//------------------------------------------------------------------------------
void BRadioButton::AllDetached()
{
	BControl::AllDetached();
}
//------------------------------------------------------------------------------
status_t BRadioButton::GetSupportedSuites(BMessage* data)
{
	return B_OK;
}
//------------------------------------------------------------------------------
status_t BRadioButton::Perform(perform_code d, void* arg)
{
	return B_ERROR;
}
//------------------------------------------------------------------------------
void BRadioButton::_ReservedRadioButton1()
{
}
//------------------------------------------------------------------------------
void BRadioButton::_ReservedRadioButton2()
{
}
//------------------------------------------------------------------------------
BRadioButton& BRadioButton::operator=(const BRadioButton&)
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

