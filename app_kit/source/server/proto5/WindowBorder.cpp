#include <String.h>
#include "ServerWindow.h"
#include "WindowBorder.h"
#include "Decorator.h"

#define DEBUG_WINBORDER

#ifdef DEBUG_WINBORDER
#include <stdio.h>
#endif

WindowBorder::WindowBorder(ServerWindow *win, const char *bordertitle)
 : Layer(win->frame,win->title->String())
{
#ifdef DEBUG_WINBORDER
printf("WindowBorder()\n");
#endif
	mbuttons=0;
	swin=win;
	title=new BString(bordertitle);
}

WindowBorder::~WindowBorder(void)
{
#ifdef DEBUG_WINBORDER
printf("~WindowBorder()\n");
#endif
	delete title;
}

void WindowBorder::MouseDown(BPoint pt, uint32 buttons)
{
#ifdef DEBUG_WINBORDER
printf("WindowBorder()::MouseDown\n");
#endif
	mbuttons|=buttons;
}

void WindowBorder::MouseMoved(BPoint pt)
{
}

void WindowBorder::MouseUp(BPoint pt, uint32 buttons)
{
#ifdef DEBUG_WINBORDER
printf("WindowBorder()::MouseUp\n");
#endif
	mbuttons&= ~buttons;
}

void WindowBorder::Draw(BRect update_rect)
{
	if(decor->Look()==B_NO_BORDER_WINDOW_LOOK)
		return;
printf("WindowBorder()::Draw():"); update_rect.PrintToStream();

	if(update && visible!=NULL)
		is_updating=true;

	decor->Draw(update_rect);

	if(update && visible!=NULL)
		is_updating=false;
}

void WindowBorder::SetDecorator(Decorator *newdecor)
{
	// AtheOS kills the decorator here. However, the decor
	// under OBOS doesn't belong to the border - it belongs to the
	// ServerWindow, so the ServerWindow will handle all (de)allocation
	// tasks. We just need to update the pointer.
	decor=newdecor;
}
