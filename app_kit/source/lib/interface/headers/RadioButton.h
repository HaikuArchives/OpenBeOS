#ifndef	_RADIO_BUTTON_H
#define	_RADIO_BUTTON_H

#include <BeBuild.h>
#include "interface/Control.h"

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

class BRadioButton : public BControl {
  public:
							BRadioButton(	BRect frame, const char *name,
											const char *label, BMessage *message,
											uint32 resizeMask = B_FOLLOW_LEFT |
											B_FOLLOW_TOP,
											uint32 flags = B_WILL_DRAW | B_NAVIGABLE); 

							BRadioButton(BMessage *data);
	virtual					~BRadioButton();

	static	BArchivable		*Instantiate(BMessage *data);
	virtual	status_t		Archive(BMessage *data, bool deep = true) const;

	virtual	void			Draw(BRect updateRect);
	virtual	void			MouseDown(BPoint where);
	virtual	void			AttachedToWindow();
	virtual	void			KeyDown(const char *bytes, int32 numBytes);
	virtual	void			SetValue(int32 value);
	virtual	void			GetPreferredSize(float *width, float *height);
	virtual void			ResizeToPreferred();
	virtual	status_t		Invoke(BMessage *msg = NULL);

	virtual void			MessageReceived(BMessage *msg);
	virtual void			WindowActivated(bool state);
	virtual	void			MouseUp(BPoint pt);
	virtual	void			MouseMoved(BPoint pt, uint32 code, const BMessage *msg);
	virtual	void			DetachedFromWindow();
	virtual	void			FrameMoved(BPoint new_position);
	virtual	void			FrameResized(float new_width, float new_height);

	virtual BHandler		*ResolveSpecifier(	BMessage *msg, int32 index,
												BMessage *specifier, int32 form,
												const char *property);

	virtual void			MakeFocus(bool state = true);
	virtual void			AllAttached();
	virtual void			AllDetached();
	virtual status_t		GetSupportedSuites(BMessage *data);

/*----- Private or reserved -----------------------------------------*/
	virtual status_t		Perform(perform_code d, void *arg);

  private:
	virtual	void			_ReservedRadioButton1();
	virtual	void			_ReservedRadioButton2();

			BRadioButton	&operator=(const BRadioButton&);
	static	BBitmap			*sBitmaps[2][3];
	bool					fOutlined;
	bool					fPressed;
	uint32					_reserved[1];
};

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
using namespace OpenBeOS;
#endif

#endif
