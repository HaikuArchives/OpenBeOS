// BCheckBox
// by Frans van Nispen (xlr8@tref.nl)
// status: done.
#include <stdio.h>
#include "../headers/InterfaceDefs.h"
#include "../headers/CheckBox.h"
#include "../headers/Control.h"
#include <interface/Window.h>
#include <app/Message.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

BCheckBox::BCheckBox(	BRect frame, const char *name, const char *label, BMessage *message,
					uint32 resizeMask, uint32 flags) : BControl( frame, name, label, message,
					resizeMask, flags)
{
	fOutlined = false;
	fPressed = false;
}

BCheckBox::~BCheckBox()
{
}


BCheckBox::BCheckBox(BMessage *data) : BControl(data)
{
	fOutlined = false;
	fPressed = false;
}

BArchivable *BCheckBox::Instantiate(BMessage *data)
{
   if(!validate_instantiation(data,"BCheckBox"))	return NULL;
   return new BCheckBox(data);
}

status_t BCheckBox::Archive(BMessage *data, bool deep = true) const
{
	BControl::Archive(data, deep);
	return B_OK;
}


void BCheckBox::Draw(BRect updateRect)
{
	SetPenSize(1);

// here we should request the Views base color whcih normaly is (216,216,216);
	rgb_color color = ui_color(B_PANEL_BACKGROUND_COLOR);
	rgb_color col;
// Draw background
	SetLowColor(ViewColor());
//	if (Parent())
//		FillRect(Bounds(), B_SOLID_LOW);				// 

// draw box
	BRect rect;
		rect.Set(1.0, 3.0, 13.0, 15.0);
		if (!IsEnabled())
			rect.InsetBy(1,1);

	SetHighColor(tint_color(color, IsEnabled() ? B_LIGHTEN_MAX_TINT : B_LIGHTEN_1_TINT));
	FillRect(rect);


		float add1 = 0.0f, add2 = 0.0f;
		IsEnabled()	? add1 = 1.0f : add2 = 1.0f;
	
		BeginLineArray(6);

		col = tint_color(color, IsEnabled() ? B_DARKEN_1_TINT : B_DARKEN_2_TINT);
		AddLine(BPoint(rect.left, rect.bottom-add2), BPoint(rect.left, rect.top), col);
		AddLine(BPoint(rect.left, rect.top), BPoint(rect.right-add2, rect.top), col);
		col = tint_color(color, IsEnabled() ? B_DARKEN_4_TINT : B_LIGHTEN_2_TINT);
		rect.InsetBy(1,1);
		AddLine(BPoint(rect.left, rect.bottom), BPoint(rect.left, rect.top), col);
		AddLine(BPoint(rect.left, rect.top), BPoint(rect.right, rect.top), col);
		col = tint_color(color, IsEnabled() ? B_NO_TINT : B_DARKEN_1_TINT);
		AddLine(BPoint(rect.left+add1, rect.bottom), BPoint(rect.right, rect.bottom), col);
		AddLine(BPoint(rect.right, rect.bottom), BPoint(rect.right, rect.top+add1), col);

		EndLineArray();
	
	rect.InsetBy(2,2);
	if (Value() == B_CONTROL_ON){
		SetHighColor(tint_color(ui_color(B_KEYBOARD_NAVIGATION_COLOR), IsEnabled() ? B_NO_TINT : B_LIGHTEN_1_TINT));
		SetPenSize(2);
		StrokeLine(BPoint(rect.left, rect.top), BPoint(rect.right, rect.bottom));
		StrokeLine(BPoint(rect.left, rect.bottom), BPoint(rect.right, rect.top));
		SetPenSize(1);
	}
	
	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);

	float h = ceil(fh.ascent + fh.descent + fh.leading) + 3.0f;

	SetHighColor( IsFocus() ? ui_color(B_KEYBOARD_NAVIGATION_COLOR) : ViewColor() );
	BRect rect2( BPoint(18.0f, 3.0f), BPoint(22.0f + font.StringWidth(BControl::Label()), h));
	StrokeRect(rect2, B_MIXED_COLORS);

	SetHighColor( tint_color(color, IsEnabled() ? B_DARKEN_MAX_TINT : B_DISABLED_LABEL_TINT) );
	DrawString( BControl::Label(), BPoint( 21.0f, 3.0f + font.Size()) );

	if (fOutlined){
			rect.Set(1.0, 3.0, 13.0, 15.0);
			if (!IsEnabled())
				rect.InsetBy(1,1);
		SetHighColor(tint_color(color, B_DARKEN_3_TINT));
		StrokeRect(rect);
	}
}

void BCheckBox::MouseDown(BPoint where)
{
	if (!IsEnabled()){
		BView::MouseDown( where );
		return;
	}

	SetMouseEventMask(B_POINTER_EVENTS,	B_NO_POINTER_HISTORY | B_SUSPEND_VIEW_FOCUS);

	MakeFocus();
	fOutlined = true;
	fPressed = true;
	Draw(Bounds());
}

void BCheckBox::MouseUp(BPoint pt)
{
	if (IsEnabled() && fPressed){

		fOutlined = false;

		if (Bounds().Contains(pt)){
			if (Value() == B_CONTROL_OFF)
				SetValue(B_CONTROL_ON);
			else
				SetValue(B_CONTROL_OFF);
	
			BControl::Invoke();
		}
		fPressed = false;
	}else{
		BView::MouseUp( pt );
		return;
	}
}

void BCheckBox::MouseMoved(BPoint pt, uint32 code, const BMessage *msg)
{
	if (IsEnabled() && fPressed){
		if (code==B_EXITED_VIEW)
			fOutlined = false;
		else
		{
			if (code==B_ENTERED_VIEW)
				fOutlined = true;
		}
		Draw(Bounds());
	}else{
		BView::MouseMoved( pt, code, msg );
		return;
	}
}

void BCheckBox::KeyDown(const char *bytes, int32 numBytes)
{
	BControl::KeyDown(bytes, numBytes);
}

void BCheckBox::AttachedToWindow()
{
	BControl::AttachedToWindow();

	if (Bounds().Height()<18.0f)					// resize to minimum height (BeOS uses 24)
		ResizeTo( Bounds().Width(), 18.0f);
	
	Draw(Bounds());
}

void BCheckBox::GetPreferredSize(float *width, float *height)
{
	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);

	*height = ceil(fh.ascent + fh.descent + fh.leading) + 6.0f;
	*width = 22.0f + ceil(font.StringWidth(BControl::Label()));
}

void BCheckBox::ResizeToPreferred()
{
	float w, h;
	GetPreferredSize(&w, &h);
	BView::ResizeTo(w,h);
}

void BCheckBox::MessageReceived(BMessage *msg)
{
	BControl::MessageReceived(msg);
}

void BCheckBox::SetValue(int32 value)
{
	if (BControl::Value()==value)
		return;
	if (value==B_CONTROL_OFF)
		BControl::SetValue(B_CONTROL_OFF);
	else
		BControl::SetValue(B_CONTROL_ON);
}

status_t BCheckBox::Invoke(BMessage *msg = NULL)
{
	return BControl::Invoke(msg);
}

void BCheckBox::MakeFocus(bool state = true)
{
	BControl::MakeFocus();
}

void BCheckBox::AllAttached() {}
void BCheckBox::AllDetached() {}
void BCheckBox::FrameMoved(BPoint new_position)	{}
void BCheckBox::FrameResized(float new_width, float new_height)	{}
void BCheckBox::WindowActivated(bool state)	{}
void BCheckBox::DetachedFromWindow()			{}
BHandler * BCheckBox::ResolveSpecifier(	BMessage *msg, int32 index, BMessage *specifier, int32 form, const char *property)
{
	return NULL;
}

status_t BCheckBox::GetSupportedSuites(BMessage *data)
{
	return B_OK;
}

void BCheckBox::_ReservedCheckBox1()	{}
void BCheckBox::_ReservedCheckBox2()	{}
void BCheckBox::_ReservedCheckBox3()	{}

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
#endif
