#ifndef	_TEXT_CONTROLI_H
#define	_TEXT_CONTROLI_H

#include "interface/TextControl.h"

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

// this needs to be a patched version of the TextView
class _BTextInput_ : public BTextView {
  public:
			_BTextInput_(	BTextControl *parent, BRect frame, const char *name, BRect rect,
							uint32 mask = B_FOLLOW_LEFT | B_FOLLOW_TOP, uint32 flags = B_WILL_DRAW | B_NAVIGABLE);
			~_BTextInput_();
			
	virtual	void			AttachedToWindow();
	virtual	void			MakeFocus(bool state = true);
	virtual	void			SetViewColor(rgb_color);
	rgb_color				ViewColor();
	virtual bool			CanEndLine(int32);
	virtual void			KeyDown(const char *bytes, int32 numBytes);
	virtual void			Draw(BRect);
	virtual	void			SetEnabled(bool state);
	virtual	void			MouseDown(BPoint where);

  private:
  	rgb_color				fViewColor;
  	bool					fEnabled;
  	bool					fChanged;
	BTextControl			*fParent;
};

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
#endif

#endif

