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
//	File Name:		Button.h
//	Author:			Frans van Nispen (xlr8@tref.nl)
//	Description:	BButton displays and controls a button in a window.
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------
#include <stdio.h>

// System Includes -------------------------------------------------------------
#include "../headers/Button.h"
#include <Control.h>
#include <InterfaceDefs.h>
#include <Message.h>
#include <Window.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------


//------------------------------------------------------------------------------
BButton::BButton(BRect frame, const char* name, const char* label,
				 BMessage* message, uint32 resizeMask, uint32 flags)
	:	BControl(frame, name, label, message, resizeMask, flags),
		fDefault(false),
		fPressed(false)
{
}
//------------------------------------------------------------------------------
BButton::~BButton()
{
}
//------------------------------------------------------------------------------
BButton::BButton(BMessage* data)
	:	BControl(data)
{
	if (data->FindBool("_default",&fDefault) != B_OK)
	{
		fDefault = false;
	}

	fPressed = false;
}
//------------------------------------------------------------------------------
BArchivable *BButton::Instantiate(BMessage *data)
{
	if (!validate_instantiation(data,"BButton"))
	{
		return NULL;
	}

	return new BButton(data);
}
//------------------------------------------------------------------------------
status_t BButton::Archive(BMessage* data, bool deep) const
{
	BControl::Archive(data, deep);
	data->AddBool("_default", fDefault);

	return B_OK;
}
//------------------------------------------------------------------------------
void BButton::Draw(BRect updateRect)
{
	BRect rect = Bounds();
	// We should request the view's base color which normaly is (216,216,216)
	rgb_color color = ui_color(B_PANEL_BACKGROUND_COLOR);
	rgb_color col;

	SetLowColor(tint_color(color, Value() ?
				B_DARKEN_1_TINT : B_LIGHTEN_2_TINT));

	BeginLineArray(14);
	if (!IsDefault())
	{
		SetHighColor(color);
		StrokeRect(rect);
		rect.InsetBy(1,1);
	}
	col = tint_color(color, IsEnabled() ? B_DARKEN_4_TINT : B_DARKEN_2_TINT);
	SetHighColor(col);
	AddLine(BPoint(rect.left, rect.bottom-1.0f),
			BPoint(rect.left, rect.top+1.0f), col);
	AddLine(BPoint(rect.left+1.0f, rect.top),
			BPoint(rect.right-1.0f, rect.top), col);
	col = tint_color(color, (Value() && !IsDefault()) ?
					 B_LIGHTEN_MAX_TINT : IsEnabled() ?
					 B_DARKEN_4_TINT : B_DARKEN_2_TINT);
	AddLine(BPoint(rect.left+1.0f, rect.bottom),
			BPoint(rect.right-1.0f, rect.bottom), col);
	AddLine(BPoint(rect.right, rect.bottom-1.0f),
			BPoint(rect.right, rect.top+1.0f), col);

	if (IsDefault())
	{
		rect.InsetBy(1,1);
		StrokeRect(rect);
	}

	rect.InsetBy(1,1);
	col = tint_color(color, Value() ? B_DARKEN_3_TINT : B_LIGHTEN_1_TINT);
	if (IsDefault())
	{
		col = tint_color(col, B_DARKEN_1_TINT);
	}
	AddLine(BPoint(rect.left, rect.top),
			BPoint(rect.left, rect.bottom), col);
	AddLine(BPoint(rect.left+1.0f, rect.top),
			BPoint(rect.right, rect.top), col);
	col = tint_color(color, Value() ?
					 B_LIGHTEN_1_TINT : IsEnabled() ?
					 B_DARKEN_2_TINT : B_NO_TINT);
	if (IsDefault())
	{
		col = tint_color(col, B_DARKEN_1_TINT);
	}
	AddLine(BPoint(rect.left+1.0f, rect.bottom),
			BPoint(rect.right, rect.bottom), col);
	AddLine(BPoint(rect.right, rect.bottom-1.0f),
			BPoint(rect.right, rect.top+1.0f), col);
	rect.InsetBy(1,1);
	col = tint_color(color, Value() ? B_DARKEN_2_TINT : B_LIGHTEN_MAX_TINT);
	AddLine(BPoint(rect.left, rect.top),
			BPoint(rect.left, rect.bottom), col);
	AddLine(BPoint(rect.left+1.0f, rect.top),
			BPoint(rect.right, rect.top), col);
	col = tint_color(color, Value() ? B_NO_TINT : B_NO_TINT);
	AddLine(BPoint(rect.left+1.0f, rect.bottom),
			BPoint(rect.right, rect.bottom), col);
	AddLine(BPoint(rect.right, rect.bottom-1.0f),
			BPoint(rect.right, rect.top+1.0f), col);
	EndLineArray();
	
	rect.InsetBy(1,1);
	FillRect(rect, B_SOLID_LOW);

	SetHighColor(tint_color(color, Value() ?
				 B_DARKEN_2_TINT : B_LIGHTEN_MAX_TINT));
	StrokeLine(BPoint(rect.left, rect.top+1.0f),
			   BPoint(rect.left+1.0f, rect.top));
	StrokeLine(BPoint(rect.left, rect.top),
			   BPoint(rect.left, rect.top));
	SetHighColor(tint_color(color, B_NO_TINT));
	StrokeLine(BPoint(rect.right-1.0f, rect.bottom),
			   BPoint(rect.right, rect.bottom-1.0f));
	StrokeLine(BPoint(rect.right, rect.bottom),
			   BPoint(rect.right, rect.bottom));
	SetHighColor(tint_color(color, Value() ?
				 B_DARKEN_3_TINT : B_LIGHTEN_1_TINT));
	if (IsDefault())
	{
		SetHighColor(tint_color(col, B_DARKEN_1_TINT));
	}
	StrokeLine(BPoint(rect.left-1.0f, rect.top-1.0f),
			   BPoint(rect.left-1.0f, rect.top-1.0f));
	SetHighColor(tint_color(color, Value() ?
				 B_LIGHTEN_1_TINT : IsEnabled() ?
				 B_DARKEN_2_TINT : B_NO_TINT));
	if (IsDefault())
	{
		SetHighColor(tint_color(col, B_DARKEN_1_TINT));
	}
	StrokeLine(BPoint(rect.right+1.0f, rect.bottom+1.0f),
			   BPoint(rect.right+1.0f, rect.bottom+1.0f));
	
	SetHighColor(tint_color(color, Value() ?
				 B_LIGHTEN_MAX_TINT : IsEnabled() ?
				 B_DARKEN_MAX_TINT : B_DISABLED_LABEL_TINT) );
	BFont font;
	GetFont(&font);

	float x = Bounds().Width() / 2 - font.StringWidth(Label()) / 2.0f;
	float y = Bounds().Height() / 2.0f + ceil(font.Size() / 2.0f - 1.0f);
	DrawString(Label(), BPoint(x, y));

	if (IsFocus())
	{
		SetHighColor( ui_color(B_KEYBOARD_NAVIGATION_COLOR) );
//		rect.InsetBy(2,2);
		StrokeRect(rect, B_MIXED_COLORS);
	}
}
//------------------------------------------------------------------------------
void BButton::MouseDown(BPoint where)
{
	if (IsEnabled())
	{
		SetMouseEventMask(B_POINTER_EVENTS,	B_NO_POINTER_HISTORY |
						  B_SUSPEND_VIEW_FOCUS);
		MakeFocus();
		SetValue(B_CONTROL_ON);
		fPressed = true;
	}
	else
	{
		BView::MouseDown(where);
		return;
	}
}
//------------------------------------------------------------------------------
void BButton::MouseUp(BPoint pt)
{
	if (IsEnabled() && fPressed)
	{
		if (Bounds().Contains(pt))
		{
			if (Value() == B_CONTROL_ON)
			{
				BControl::Invoke();
			}
			SetValue(B_CONTROL_OFF);
		}
		fPressed = false;
	}
	else
	{
		BView::MouseUp(pt);
		return;
	}
}
//------------------------------------------------------------------------------
void BButton::MouseMoved(BPoint pt, uint32 code, const BMessage* msg)
{
	if (IsEnabled() && fPressed)
	{
		if (code==B_EXITED_VIEW)
		{
			SetValue(B_CONTROL_OFF);
		}
		else
		{
			if (code==B_ENTERED_VIEW)
			{
				SetValue(B_CONTROL_ON);
			}
		}
	}
	else
	{
		BView::MouseMoved(pt, code, msg);
		return;
	}
}
//------------------------------------------------------------------------------
void BButton::KeyDown(const char* bytes, int32 numBytes)
{
	if (numBytes == 1)
	{
		switch (bytes[0])
		{
			case B_ENTER:
			case B_SPACE:
				SetValue(B_CONTROL_ON);
				snooze(50000);
				SetValue(B_CONTROL_OFF);
				Invoke();
				break;

			default:
				BControl::KeyDown(bytes, numBytes);
		}
	}
	else
	{
		BControl::KeyDown(bytes, numBytes);
	}
}
//------------------------------------------------------------------------------
void BButton::AttachedToWindow()
{
	BControl::AttachedToWindow();
	// set the default button
	if (IsDefault())
	{
		Window()->SetDefaultButton((BButton*)this);
	}
	
	// resize to minimum height (BeOS uses 24)
	if (Bounds().Height() < 20.0f)
	{
		ResizeTo( Bounds().Width(), 22.0f);
	}
	
	Draw(Bounds());
}
//------------------------------------------------------------------------------
void BButton::MakeDefault(bool state)
{
	if (state == IsDefault())
	{
		return;
	}

	fDefault = state;
	
	if (Window())
	{
		BButton *button = (BButton*)(Window()->DefaultButton());
		if (fDefault)
		{
			if (button)
			{
				button->MakeDefault(false);
				button->Invalidate();
			}
			Window()->SetDefaultButton((BButton*)this);
		}
		else
		{
			if (button == this)
			{
				Window()->SetDefaultButton(NULL);
			}
		}
	}

	Invalidate();
}
//------------------------------------------------------------------------------
bool BButton::IsDefault() const
{
	return fDefault;
}
//------------------------------------------------------------------------------
void BButton::GetPreferredSize(float* width, float* height)
{
	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);

	*height = ceil(fh.ascent + fh.descent + fh.leading) + 12.0f;
	*width = 20.0f + ceil(font.StringWidth(Label()));
	if (*width < 75.0f)
	{
		*width = 75.0f;
	}
}
//------------------------------------------------------------------------------
void BButton::ResizeToPreferred()
{
	float w, h;
	GetPreferredSize(&w, &h);
	BView::ResizeTo(w,h);
}
//------------------------------------------------------------------------------
void BButton::SetLabel(const char* text)
{
	BControl::SetLabel(text);
}
//------------------------------------------------------------------------------
void BButton::MessageReceived(BMessage* msg)
{
	BControl::MessageReceived(msg);
}
//------------------------------------------------------------------------------
void BButton::SetValue(int32 value)
{
	if (BControl::Value()==value)
	{
		return;
	}
	if (value==B_CONTROL_OFF)
	{
		BControl::SetValue(B_CONTROL_OFF);
	}
	else
	{
		BControl::SetValue(B_CONTROL_ON);
	}
}
//------------------------------------------------------------------------------
status_t BButton::Invoke(BMessage* msg)
{
	return BControl::Invoke(msg);
}
//------------------------------------------------------------------------------
void BButton::MakeFocus(bool state)
{
	BControl::MakeFocus(state);
}
//------------------------------------------------------------------------------
BHandler* BButton::ResolveSpecifier(BMessage* msg, int32 index,
									BMessage* specifier, int32 form,
									const char* property)
{
	return NULL;
}
//------------------------------------------------------------------------------
status_t BButton::GetSupportedSuites(BMessage* data)
{
	return B_OK;
}
//------------------------------------------------------------------------------
void BButton::AllAttached()
{
	BView::AllAttached();
}
//------------------------------------------------------------------------------
void BButton::AllDetached()
{
	BView::AllDetached();
}
//------------------------------------------------------------------------------
void BButton::FrameMoved(BPoint new_position)
{
	BView::FrameMoved(new_position);
}
//------------------------------------------------------------------------------
void BButton::FrameResized(float new_width, float new_height)
{
	BView::FrameResized(new_width, new_height);
}
//------------------------------------------------------------------------------
void BButton::WindowActivated(bool state)
{
	BView::WindowActivated(state);
}
//------------------------------------------------------------------------------
void BButton::DetachedFromWindow()
{
	BView::DetachedFromWindow();
}
//------------------------------------------------------------------------------
status_t BButton::Perform(perform_code d, void* arg)
{
	return B_ERROR;
}
//------------------------------------------------------------------------------
void BButton::_ReservedButton1()
{
}
//------------------------------------------------------------------------------
void BButton::_ReservedButton2()
{
}
//------------------------------------------------------------------------------
void BButton::_ReservedButton3()
{
}
//------------------------------------------------------------------------------
BButton& BButton::operator=(const BButton&)
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

