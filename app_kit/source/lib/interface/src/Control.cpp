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
//	File Name:		Control.cpp
//	Author:			Frans van Nispen (xlr8@tref.nl)
//	Description:	BMessageFilter class creates objects that filter
//					in-coming BMessages.  
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

// System Includes -------------------------------------------------------------
#include <BeBuild.h>
#include <Control.h>
#include <Invoker.h>
#include <Message.h>
#include <View.h>
#include <Window.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------


//------------------------------------------------------------------------------
BControl::BControl(BRect frame, const char* name, const char* label,
				   BMessage* message, uint32 resizeMask, uint32 flags)
	:	BView(frame, name, resizeMask, flags),
		BInvoker(message, NULL)
{
	fLabel = new char[strlen(label) + 1];
	strcpy(fLabel, label);
	fEnabled = true;
	fValue = B_CONTROL_OFF;
}
//------------------------------------------------------------------------------
BControl::~BControl()
{
	if (fLabel)
	{
		delete[] fLabel;
	}
}
//------------------------------------------------------------------------------
BControl::BControl(BMessage* data)
	:	BView(data)
{
	const char *label;

	if (data->FindInt32("_val", &fValue) != B_OK)
	{
		fValue = 0;
	}
	if (data->FindString("_label", &label) != B_OK)
	{
		label = NULL;
	}
	SetLabel(label);

	if (data->FindBool("_disable", &fEnabled) != B_OK)
	{
		fEnabled = false;
	}
	fEnabled = !fEnabled;

	BMessage msg;
	if (data->FindMessage("_msg", &msg) == B_OK)
	{
		BInvoker::SetMessage(&msg);
	}
}
//------------------------------------------------------------------------------
BArchivable* BControl::Instantiate(BMessage* data)
{
	if (!validate_instantiation(data,"BControl"))
	{
		return NULL;
	}

	return new BControl(data);
}
//------------------------------------------------------------------------------
status_t BControl::Archive(BMessage* data, bool deep) const
{
	BView::Archive(data, deep);

	if (fValue)		data->AddInt32("_val", fValue);
	if (!fEnabled)	data->AddBool("_disable", true);
	if (fLabel)		data->AddString("_label", fLabel);
	if (Message())	data->AddMessage("_msg", Message());

	return B_OK;
}
//------------------------------------------------------------------------------
void BControl::WindowActivated(bool state)
{
	BView::WindowActivated(state);
}
//------------------------------------------------------------------------------
void BControl::AttachedToWindow()
{
	if (Parent())
	{
		SetViewColor(Parent()->ViewColor());
	}
	BView::AttachedToWindow();
	BInvoker::SetTarget(Window());
}
//------------------------------------------------------------------------------
void BControl::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case B_CONTROL_INVOKED:
			Invoke();
			break;

		default:
			BView::MessageReceived(msg);
	}
}
//------------------------------------------------------------------------------
void BControl::MakeFocus(bool state)
{
	BView::MakeFocus(state);
	fFocusChanging = true;
	Invalidate();
	Sync();
	fFocusChanging = false;
}
//------------------------------------------------------------------------------
void BControl::KeyDown(const char* bytes, int32 numBytes)
{
	BMessage* msg = Window()->CurrentMessage();
	if (numBytes == 1)
	{
		switch (bytes[0])
		{
			case B_UP_ARROW:
				msg->ReplaceInt64("when", (int64)system_time());
				msg->ReplaceInt32("key", 38);
				msg->ReplaceInt32("raw_char", B_TAB);
				msg->ReplaceInt32("modifiers", B_SCROLL_LOCK | B_SHIFT_KEY);
				msg->ReplaceInt8("byte", B_TAB);
				msg->ReplaceString("bytes", "");
				Looper()->PostMessage(msg);
				break;

			case B_DOWN_ARROW:
				msg->ReplaceInt64("when", (int64)system_time());
				msg->ReplaceInt32("key", 38);
				msg->ReplaceInt32("raw_char", B_TAB);
				msg->ReplaceInt8("byte", B_TAB);
				msg->ReplaceString("bytes", "");
				Looper()->PostMessage(msg);
				break;

			case B_ENTER:
			case B_SPACE:
				if (Value())
				{
					SetValue(B_CONTROL_OFF);
				}
				else
				{
					SetValue(B_CONTROL_ON);
				}
				BInvoker::Invoke();
				break;

			default:
				BView::KeyDown(bytes, numBytes);
		}
	}
	else
	{
		BView::KeyDown(bytes, numBytes);
	}
}
//------------------------------------------------------------------------------
void BControl::MouseDown(BPoint pt)
{
	BView::MouseDown(pt);
}
//------------------------------------------------------------------------------
void BControl::MouseUp(BPoint pt)
{
	BView::MouseUp(pt);
}
//------------------------------------------------------------------------------
void BControl::MouseMoved(BPoint pt, uint32 code, const BMessage *msg)
{
	BView::MouseMoved(pt, code, msg);
}
//------------------------------------------------------------------------------
void BControl::DetachedFromWindow()
{
	BView::DetachedFromWindow();
}
//------------------------------------------------------------------------------
void BControl::SetLabel(const char* label)
{
	if (fLabel)
	{
		delete[] fLabel;
	}
	fLabel = new char[strlen(label)+1];
	strcpy(fLabel, label);

	Invalidate();
}
//------------------------------------------------------------------------------
const char* BControl::Label() const
{
	return fLabel;
}
//------------------------------------------------------------------------------
void BControl::SetValue(int32 value)
{
	if (value == Value())
	{
		return;
	}
	fValue = value;
	Invalidate();
}
//------------------------------------------------------------------------------
int32 BControl::Value() const
{
	return fValue;
}
//------------------------------------------------------------------------------
void BControl::SetEnabled(bool on)
{
	fEnabled = on;
	if (fEnabled)
	{
		BView::SetFlags(Flags() | B_NAVIGABLE);
	}
	else
	{
		BView::SetFlags(Flags() & (0xffffffff - B_NAVIGABLE));
	}
	Invalidate();
}
//------------------------------------------------------------------------------
bool BControl::IsEnabled() const
{
	return fEnabled;
}
//------------------------------------------------------------------------------
void BControl::GetPreferredSize(float* width, float* height)
{
	*width = 100.0f;
	*height = 100.0f;
}
//------------------------------------------------------------------------------
void BControl::ResizeToPreferred()
{
	BView::ResizeToPreferred();
}
//------------------------------------------------------------------------------
status_t BControl::Invoke(BMessage *message)
{
	BInvoker::Invoke(message);
	return B_OK;
}
//------------------------------------------------------------------------------
BHandler* BControl::ResolveSpecifier(BMessage* msg, int32 index,
									 BMessage* specifier, int32 form,
									 const char* property)
{
	// we need to revolve specifiers for label and value here, that is as soon
	// as I know how
	return NULL;
}
//------------------------------------------------------------------------------
status_t BControl::GetSupportedSuites(BMessage *data)
{
	//Adds the name "suite/vnd.Be-control" to the message.
	//See "Scripting Support" in the class overview and the "Scripting" section
	//in The Application Kit chapter for more information.

	return B_OK;
}
//------------------------------------------------------------------------------
void BControl::AllAttached()
{
	BView::AllAttached();
}
void BControl::AllDetached()
{
	BView::AllDetached();
}
status_t BControl::Perform(perform_code d, void* arg)
{
	return B_ERROR;
}
//------------------------------------------------------------------------------
bool BControl::IsFocusChanging() const
{
	return fFocusChanging;
}
//------------------------------------------------------------------------------
bool BControl::IsTracking() const
{
	return fTracking;
}
//------------------------------------------------------------------------------
void BControl::SetTracking(bool state)
{
	fTracking = state;
}
//------------------------------------------------------------------------------
void BControl::SetValueNoUpdate(int32 value)
{
	if (value == Value())
		return;
	fValue = value;
}
//------------------------------------------------------------------------------
void BControl::_ReservedControl1()
{
}
//------------------------------------------------------------------------------
void BControl::_ReservedControl2()
{
}
//------------------------------------------------------------------------------
void BControl::_ReservedControl3()
{
}
//------------------------------------------------------------------------------
void BControl::_ReservedControl4()
{
}
//------------------------------------------------------------------------------
BControl& BControl::operator=(const BControl&)
{
	return *this;
}
//------------------------------------------------------------------------------

/*
 * $Log $
 *
 * $Id  $
 *
 */

