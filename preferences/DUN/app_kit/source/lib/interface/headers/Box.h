#ifndef _BOX_H
#define _BOX_H

#include <BeBuild.h>
#include <View.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

class BBox : public BView{
  public:
						BBox(	BRect bounds, const char *name = NULL, uint32 resizeFlags = B_FOLLOW_LEFT | B_FOLLOW_TOP,
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

  private:
	virtual	void		_ReservedBox1();
	virtual	void		_ReservedBox2();

	char				*fLabel;
	BRect				fBounds;
	border_style		fStyle;
	BView				*fLabelView;
	uint32				_reserved[1];
};

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
using namespace OpenBeOS;
#endif

#endif /* _BOX_H */
