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
//	File Name:		CheckBox.h
//	Author:			Frans van Nispen (xlr8@tref.nl)
//	Description:	BCheckBox displays an on/off control.
//------------------------------------------------------------------------------

#ifndef	_CHECK_BOX_H
#define	_CHECK_BOX_H

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#include <BeBuild.h>
#include <Control.h>

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------


// BCheckBox class -------------------------------------------------------------
class BCheckBox : public BControl {
public:
						BCheckBox(	BRect frame,
									const char *name,
									const char *label,
									BMessage *message,
									uint32 resizeMask = B_FOLLOW_LEFT | B_FOLLOW_TOP, 
									uint32 flags = B_WILL_DRAW | B_NAVIGABLE); 
	virtual				~BCheckBox();

						BCheckBox(BMessage *data);
	static	BArchivable	*Instantiate(BMessage *data);
	virtual	status_t	Archive(BMessage *data, bool deep = true) const;

	virtual	void		Draw(BRect updateRect);
	virtual	void		AttachedToWindow();
	virtual	void		MouseDown(BPoint where);

	virtual void		MessageReceived(BMessage *msg);
	virtual void		WindowActivated(bool state);
	virtual	void		KeyDown(const char *bytes, int32 numBytes);
	virtual	void		MouseUp(BPoint pt);
	virtual	void		MouseMoved(BPoint pt, uint32 code, const BMessage *msg);
	virtual	void		DetachedFromWindow();
	virtual	void		SetValue(int32 value);
	virtual void		GetPreferredSize(float *width, float *height);
	virtual void		ResizeToPreferred();
	virtual	status_t	Invoke(BMessage *msg = NULL);
	virtual	void		FrameMoved(BPoint new_position);
	virtual	void		FrameResized(float new_width, float new_height);

	virtual BHandler	*ResolveSpecifier(	BMessage *msg,
											int32 index,
											BMessage *specifier,
											int32 form,
											const char *property);
	virtual status_t	GetSupportedSuites(BMessage *data);

	virtual void		MakeFocus(bool state = true);
	virtual void		AllAttached();
	virtual void		AllDetached();

// Private or reserved ---------------------------------------------------------
	virtual status_t	Perform(perform_code d, void *arg);

private:
	virtual	void		_ReservedCheckBox1();
	virtual	void		_ReservedCheckBox2();
	virtual	void		_ReservedCheckBox3();

			BCheckBox	&operator=(const BCheckBox &);

			bool		fOutlined;
			bool		fPressed;
			uint32		_reserved[1];
};
//------------------------------------------------------------------------------

#endif	// _CHECK_BOX_H

/*
 * $Log $
 *
 * $Id  $
 *
 */

