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
//	File Name:		Box.h
//	Author:			Frans van Nispen (xlr8@tref.nl)
//	Description:	BBox objects group views together and draw a border
//					around them.
//------------------------------------------------------------------------------

#ifndef _BOX_H
#define _BOX_H

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#include <BeBuild.h>
#include <View.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------


// BBox class ------------------------------------------------------------------
class BBox : public BView{
public:
						BBox(	BRect bounds, const char *name = NULL,
								uint32 resizeFlags = B_FOLLOW_LEFT | B_FOLLOW_TOP,
								uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
								border_style border = B_FANCY_BORDER);
	virtual 			~BBox(void);

						BBox(BMessage *data);
	static	BArchivable	*Instantiate(BMessage *data);
	virtual	status_t	Archive(BMessage *data, bool deep = true) const;

	virtual	void		SetBorder(border_style style);
	border_style		Border() const;
	void				SetLabel(const char *label);
	status_t			SetLabel(BView *view_label);
	const char			*Label() const;
	BView				*LabelView() const;

	virtual	void		Draw(BRect bounds);
	virtual	void		AttachedToWindow();
	virtual	void		DetachedFromWindow();
	virtual	void		AllAttached();
	virtual	void		AllDetached();
	virtual void		FrameResized(float new_width, float new_height);
	virtual void		MessageReceived(BMessage *msg);
	virtual	void		MouseDown(BPoint pt);
	virtual	void		MouseUp(BPoint pt);
	virtual	void		WindowActivated(bool state);
	virtual	void		MouseMoved(BPoint pt, uint32 code, const BMessage *msg);
	virtual	void		FrameMoved(BPoint new_position);

	virtual BHandler	*ResolveSpecifier(	BMessage *msg, int32 index, BMessage *specifier,
											int32 form, const char *property);
	virtual void		ResizeToPreferred();
	virtual void		GetPreferredSize(float *width, float *height);
	virtual void		MakeFocus(bool state = true);
	virtual status_t	GetSupportedSuites(BMessage *data);

// Private or reserved ---------------------------------------------------------
	virtual	status_t	Perform(perform_code d, void* arg);

private:
	virtual	void		_ReservedBox1();
	virtual	void		_ReservedBox2();

			BBox		&operator=(const BBox &);

		char			*fLabel;
		BRect			fBounds;
		border_style	fStyle;
		BView			*fLabelView;
		uint32			_reserved[1];
};
//------------------------------------------------------------------------------

#endif	// _BOX_H

/*
 * $Log $
 *
 * $Id  $
 *
 */

