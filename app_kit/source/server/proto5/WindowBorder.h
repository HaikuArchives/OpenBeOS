#ifndef _WINBORDER_H_
#define _WINBORDER_H_

#include <Rect.h>
#include <String.h>
#include "Layer.h"

class ServerWindow;
class Decorator;

class WindowBorder : public Layer
{
public:
	WindowBorder(ServerWindow *win, const char *bordertitle);
	~WindowBorder(void);
	void MouseDown(BPoint pt, uint32 buttons);
	void MouseMoved(BPoint pt);
	void MouseUp(BPoint pt, uint32 buttons);
	void Draw(BRect update);
	void SystemColorsUpdated(void);
	void SetDecorator(Decorator *newdecor);

	ServerWindow *swin;
	BString *title;
	Decorator *decor;
	int32 flags;
	BRect frame, clientframe;
	uint32 mbuttons;
	bool update;
};

#endif