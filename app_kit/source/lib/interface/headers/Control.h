// BControl
// by Frans van Nispen (xlr8@tref.nl)
// status: done.
#ifndef	_CONTROL_H
#define	_CONTROL_H

#include <BeBuild.h>
#include "app/Invoker.h"
#include "interface/Rect.h"
#include <Message.h>	/* For convenience */
#include <View.h>

class BWindow;

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

enum {
	B_CONTROL_OFF = 0,
	B_CONTROL_ON = 1
};

class BControl : public BView, public BInvoker {
  public:
						BControl(	BRect frame, const char *name, const char *label,
									BMessage *message, uint32 resizeMask, uint32 flags); 
	virtual				~BControl();
						BControl(BMessage *data);

//from BView
	static BArchivable	*Instantiate(BMessage *data);
	virtual	status_t	Archive(BMessage *data, bool deep = true) const;

	virtual	void		AttachedToWindow();
	virtual	void		DetachedFromWindow();
	virtual	void		MessageReceived(BMessage *msg);
	virtual	void		KeyDown(const char *bytes, int32 numBytes);
	virtual	void		MouseDown(BPoint pt);
	virtual	void		MouseUp(BPoint pt);
	virtual	void		MouseMoved(BPoint pt, uint32 code, const BMessage *msg);
	virtual void		AllAttached();
	virtual void		AllDetached();
	virtual void		MakeFocus(bool state = true);
	virtual	void		WindowActivated(bool state);
	virtual	void		GetPreferredSize(float *width, float *height);
	virtual void		ResizeToPreferred();
	virtual BHandler	*ResolveSpecifier(BMessage *msg, int32 index, BMessage *specifier,
									int32 form, const char *property);
	virtual status_t	GetSupportedSuites(BMessage *data);


// from BInvoker
	virtual	status_t	Invoke(BMessage *msg = NULL);


// just BControl
	virtual	void		SetLabel(const char *text);
	const char			*Label() const;
		
	virtual	void		SetValue(int32 value);
	int32				Value() const;

	virtual void		SetEnabled(bool on);
	bool				IsEnabled() const;

//This one needs to be set initialy by the BView (BOS)
	virtual void		SetTab(uint32 tab);
	uint32				Tab() const;

  protected:
	bool				IsFocusChanging() const;
	bool				IsTracking() const;
	void				SetTracking(bool state);

	void				SetValueNoUpdate(int32 value);

  private:
//these for future binary compatibility
	virtual	void		_ReservedControl1();
	virtual	void		_ReservedControl2();
	virtual	void		_ReservedControl3();
	virtual	void		_ReservedControl4();

	char				*fLabel;
	int32				fValue;
	bool				fEnabled;
	bool				fFocusChanging;
	bool				fTracking;
	bool				fWantsNav;
	uint32				fTab;
	uint32				_reserved[4];		// for future extension
};

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
using namespace OpenBeOS;
#endif

#endif
