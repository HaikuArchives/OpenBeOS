// BBox
// by Frans van Nispen (xlr8@tref.nl)
// status: done.
#include <stdio.h>
#include "../headers/Box.h"
#include "../headers/Control.h"
#include "../headers/InterfaceDefs.h"
#include <interface/Window.h>
#include <app/Message.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

BBox::BBox(BRect frame, const char *name, uint32 resizeMask, uint32 flags, border_style border)
			: BView(frame, name, resizeMask, flags)
{
	fStyle = border;
	fLabel = NULL;
	fLabelView = NULL;

#if 0
	if (beos_gui_type())
#endif
		SetFont(be_bold_font);
}

BBox::~BBox()
{
	if (fLabelView)
		fLabelView->RemoveSelf();
	
	if (fLabel)
		delete[] fLabel;
}


BBox::BBox(BMessage *data) : BView(data)
{
	const char *label;
	
	if(data->FindInt32("_style",(int32&)fStyle) != B_OK)	fStyle = B_FANCY_BORDER;
	if(data->FindString("_label",&label) != B_OK)			label = NULL;
	SetLabel(label);
}

BArchivable *BBox::Instantiate(BMessage *data)
{
   if(!validate_instantiation(data,"BBox"))     return NULL;
   return new BBox(data);
}

status_t BBox::Archive(BMessage *data, bool deep = true) const
{
	BView::Archive(data, deep);
	
	if (fLabel)
		data->AddString("_label",fLabel);
	data->AddInt32("_style", fStyle);

	return B_OK;
}

void BBox::SetBorder(border_style style)
{
	fStyle = style;
	Invalidate();
}

border_style BBox::Border() const
{
	return fStyle;
}

void BBox::SetLabel(const char *label)
{
	if (fLabel)
		delete[] fLabel;
	fLabel = new char[strlen(label)+1];
	strcpy(fLabel, label);

	Invalidate();
}

status_t BBox::SetLabel(BView *view_label)
{
	if (view_label){
		fLabelView = view_label;
		AddChild(fLabelView);
		fLabel = NULL;
	}
	Invalidate();
	return B_OK;
}

const char *BBox::Label() const
{
	return fLabel;
}

BView *BBox::LabelView() const
{
	return fLabelView;
}

void BBox::Draw(BRect bounds)
{
	BRect rect = Bounds();

	SetLowColor(ViewColor());

	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);

	rect.top = (fh.ascent + fh.descent)/2.0f;

	switch (fStyle){
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
	
#if 0
	SetHighColor(tint_color(ui_color(B_VIEW_COLOR), beos_gui_type() ? B_DARKEN_MAX_TINT : B_DARKEN_4_TINT));
#endif
	SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_MAX_TINT));
	if (fLabel){
		rect.Set(6.0f, 1.0f, 12.0f + font.StringWidth( fLabel ), fh.ascent + fh.descent);
		FillRect(rect, B_SOLID_LOW);
		DrawString( fLabel, BPoint(10.0f, ceil(fh.ascent - fh.descent) +1.0f ));
	}
}

void BBox::AttachedToWindow()
{
	if (Parent())
		SetViewColor( Parent()->ViewColor() );
	else
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

}

void BBox::ResizeToPreferred()
{
	BView::ResizeToPreferred();
}
void BBox::GetPreferredSize(float *width, float *height)
{
	BRect r(0,0,99,99);

	if (Parent()){
		r = Parent()->Bounds();
		r.InsetBy(10,10);
	}
	
	*width = r.Width();
	*height = r.Height();
}

void BBox::DetachedFromWindow()	{}
void BBox::AllAttached()			{}
void BBox::AllDetached()			{}
void BBox::FrameResized(float new_width, float new_height)	{}
void BBox::MessageReceived(BMessage *msg)
{
	BView::MessageReceived(msg);
}

void BBox::MouseDown(BPoint pt)	{}
void BBox::MouseUp(BPoint pt)	{}
void BBox::WindowActivated(bool state)	{}
void BBox::MouseMoved(BPoint pt, uint32 code, const BMessage *msg)	{}
void BBox::FrameMoved(BPoint new_position)	{}

BHandler *BBox::ResolveSpecifier(BMessage *msg, int32 index, BMessage *specifier,int32 form, const char *property)
{
	return NULL;
}

void BBox::MakeFocus(bool state = true)
{
	BView::MakeFocus(state);
}

status_t BBox::GetSupportedSuites(BMessage *data)
{
	return B_OK;
}

void BBox::_ReservedBox1()	{}
void BBox::_ReservedBox2()	{}

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
#endif
