#ifndef	_TEXT_CONTROL_H
#define	_TEXT_CONTROL_H

#include "../headers/InterfaceDefs.h"
#include <BeBuild.h>
#include "../headers/Control.h"
#include <be/interface/TextView.h>			/* For convenience */

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

class _BTextInput_;

// The actual TextControl
class BTextControl : public BControl {
  public:
							BTextControl(	BRect frame, const char *name,
											const char *label,
											const char *initial_text, BMessage *message,
											uint32 rmask = B_FOLLOW_LEFT | B_FOLLOW_TOP,
											uint32 flags = B_WILL_DRAW | B_NAVIGABLE);
	virtual					~BTextControl();
							BTextControl(BMessage *data);
	static	BArchivable		*Instantiate(BMessage *data);
	virtual	status_t		Archive(BMessage *data, bool deep = true) const;

	virtual	void			SetText(const char *text);
	const char				*Text() const;

	virtual	void			SetValue(int32 value);
	virtual	status_t		Invoke(BMessage *msg = NULL);

	BTextView				*TextView() const;

	virtual	void			SetModificationMessage(BMessage *message);
	BMessage				*ModificationMessage() const;

	virtual	void			SetAlignment(alignment label, alignment text);
	void					GetAlignment(alignment *label, alignment *text) const;
	virtual	void			SetDivider(float dividing_line);
	float					Divider() const;

	virtual	void			Draw(BRect updateRect);
	virtual	void			MouseDown(BPoint where);
	virtual	void			AttachedToWindow();
	virtual	void			MakeFocus(bool focusState = true);
	virtual	void			SetEnabled(bool state);
	virtual	void			FrameMoved(BPoint new_position);
	virtual	void			FrameResized(float new_width, float new_height);
	virtual	void			WindowActivated(bool active);
	
	virtual	void			GetPreferredSize(float *width, float *height);
	virtual	void			ResizeToPreferred();

	virtual void			MessageReceived(BMessage *msg);
	virtual BHandler		*ResolveSpecifier(	BMessage *msg, int32 index,
												BMessage *specifier, int32 form,
												const char *property);

	virtual	void			MouseUp(BPoint pt);
	virtual	void			MouseMoved(BPoint pt, uint32 code, const BMessage *msg);
	virtual	void			DetachedFromWindow();

	virtual void			AllAttached();
	virtual void			AllDetached();
	virtual status_t		GetSupportedSuites(BMessage *data);
	virtual void			SetFlags(uint32 flags);

/*----- Private or reserved -----------------------------------------*/
	virtual status_t		Perform(perform_code d, void *arg);

  private:
	friend	class 			_BTextInput_;
	virtual	void			_ReservedTextControl1();
	virtual	void			_ReservedTextControl2();
	virtual	void			_ReservedTextControl3();
	virtual	void			_ReservedTextControl4();

	BTextControl			&operator=(const BTextControl &);

// this one is not defined, so I guess I am on my own here
	_BTextInput_			*fText;

	char					*fUnusedCharP;	//fLabel;	// this is in BControl
	BMessage				*fModificationMessage;
	alignment				fLabelAlign;
	float					fDivider;
//	uint16					fPrevWidth;
//	uint16					fPrevHeight;
	uint32					_reserved[4];
#if !_PR3_COMPATIBLE_
	uint32					_more_reserved[4];
#endif

//	bool					fClean;
	bool					fSkipSetFlags;		// will use this for SHIFT-TAB
	bool					fUnusedBool1;
	bool					fUnusedBool2;
};

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
using namespace OpenBeOS;
#endif

#endif
