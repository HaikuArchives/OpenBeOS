// BStringView
// by Frans van Nispen (xlr8@tref.nl)
// status: done.
#include <stdio.h>
#include "interface/StringView.h"
#include <interface/View.h>
#include <interface/Window.h>
#include <app/Message.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

BStringView::BStringView(	BRect frame, const char *name, const char *text, uint32 resizeMask, uint32 flags)
						: BView( frame, name, resizeMask, flags)
{
	fText = new char[strlen(text)+1];
	strcpy(fText, text);
	fAlign = B_ALIGN_LEFT;
}

BStringView::BStringView(BMessage *data) : BView(data)
{
	const char *text;

	if(data->FindInt32("_aligne",(int32&)fAlign) != B_OK)	fAlign = B_ALIGN_LEFT;
	if(data->FindString("_text",&text) != B_OK)				text = NULL;
	SetText(text);
}

BArchivable	*BStringView::Instantiate(BMessage *data)
{
   if(!validate_instantiation(data,"BStringView"))     return NULL;
   return new BStringView(data);
}

status_t BStringView::Archive(BMessage *data, bool deep = true) const
{
	BView::Archive(data, deep);
	
	if (fText)
		data->AddString("_text",fText);
	data->AddInt32("_align", fAlign);

	return B_OK;
}

BStringView::~BStringView()
{
	if (fText)
		delete[] fText;
}

void BStringView::SetText(const char *text)
{
	if (fText)
		delete[] fText;
	fText = new char[strlen(text)+1];
	strcpy(fText, text);
	Invalidate();
}

const char *BStringView::Text() const
{
	return fText;
}

void BStringView::SetAlignment(alignment flag)
{
	fAlign = flag;
	Invalidate();
}

alignment BStringView::Alignment() const
{
	return fAlign;
}

void BStringView::AttachedToWindow()
{
	if (Parent())
		SetViewColor( Parent()->ViewColor() );
}

void BStringView::Draw(BRect bounds)
{
	SetLowColor(ViewColor());
	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);
	
	float y = Bounds().bottom - ceil(fh.descent);
	float x;
	switch (fAlign){
	case B_ALIGN_RIGHT:
		x = Bounds().Width() - font.StringWidth(fText) - 2.0f;
		break;

	case B_ALIGN_CENTER:
		x = (Bounds().Width() - font.StringWidth(fText))/2.0f;
		break;

	default:
		x = 2.0f;
		break;
	}
	
	DrawString( fText, BPoint(x,y) );
}

void BStringView::ResizeToPreferred()
{
	float w, h;
	GetPreferredSize(&w, &h);
	BView::ResizeTo(w,h);
}

void BStringView::GetPreferredSize(float *width, float *height)
{
	BFont font;
	GetFont(&font);
	font_height fh;
	font.GetHeight(&fh);

	*height = ceil(fh.ascent + fh.descent + fh.leading) + 2.0f;
	*width = 4.0f + ceil(font.StringWidth(fText));
}

void BStringView::MessageReceived(BMessage *msg)
{
	BView::MessageReceived(msg);
}

void BStringView::MouseDown(BPoint pt)	{}
void BStringView::MouseUp(BPoint pt)		{}
void BStringView::MouseMoved(BPoint pt, uint32 code, const BMessage *msg)	{}
void BStringView::DetachedFromWindow()	{}
void BStringView::FrameMoved(BPoint new_position)	{}
void BStringView::FrameResized(float new_width, float new_height)	{}

BHandler *BStringView::ResolveSpecifier(	BMessage *msg, int32 index, BMessage *specifier, int32 form, const char *property)
{
	return NULL;
}

void BStringView::MakeFocus(bool state = true)
{
	BView::MakeFocus(state);
}

void BStringView::AllAttached()	{}
void BStringView::AllDetached()	{}
status_t BStringView::GetSupportedSuites(BMessage *data)
{
	return B_OK;
}

void BStringView::_ReservedStringView1()	{}
void BStringView::_ReservedStringView2()	{}
void BStringView::_ReservedStringView3()	{}

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
#endif
