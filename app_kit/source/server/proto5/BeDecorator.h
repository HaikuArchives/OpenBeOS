#ifndef _BEOS_DECORATOR_H_
#define _BEOS_DECORATOR_H_

#include "Decorator.h"

class BeDecorator: public Decorator
{
public:
	BeDecorator(Layer *lay, uint32 dflags, window_look wlook);
	~BeDecorator(void);
	
	click_type Clicked(BPoint pt);
	void Resize(BRect rect);
	BRect GetBorderSize(void);
	BPoint GetMinimumSize(void);
	void SetTitle(const char *newtitle);
	void SetFlags(uint32 flags);
	void SetLook(window_look wlook);
	void UpdateFont(void);
	void UpdateTitle(const char *string);
	void SetFocus(bool focused);
	void SetCloseButton(bool down);
	void SetZoomButton(bool down);
	void Draw(BRect update);
	void DrawZoom(BRect r);
	void DrawClose(BRect r);
	void CalculateBorders(void);

	bool zoomstate;
	bool closestate;
	uint32 taboffset;
	rgb_color blue,blue2,black,gray,white,yellow;
	BRect zoomrect,closerect,tabrect,frame,
		resizerect,borderrect;
};

#endif