#include <String.h>
#include <Debug.h>
#include "View.h"	// for mouse button defines
#include "ServerWindow.h"
#include "WindowBorder.h"
#include "Decorator.h"
#include "Desktop.h"

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
	movewin=false;
	resizewin=false;
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
	click_type click=decor->Clicked(pt, buttons);

	switch(click)
	{
		case CLICK_MOVETOBACK:
		{
#ifdef DEBUG_WINBORDER
printf("WindowBorder(): MoveToBack\n");
#endif
			swin->SetFocus(false);

			// Move the window to the back (the top of the tree)
			Layer *top=GetRootLayer();
			layerlock->Lock();
			RemoveSelf();
			top->AddChild(this);
			layerlock->Unlock();
			break;
		}
		case CLICK_MOVETOFRONT:
		{
#ifdef DEBUG_WINBORDER
printf("WindowBorder(): MoveToFront\n");
#endif
			swin->SetFocus(true);

			// Move the window to the front by making it the bottom
			// child of the root layer
			Layer *top=GetRootLayer();
			layerlock->Lock();
			RemoveSelf();
			uppersibling=top->bottomchild;
			lowersibling=NULL;
			top->bottomchild=this;
			layerlock->Unlock();
			break;
		}
		case CLICK_CLOSE:
		{
#ifdef DEBUG_WINBORDER
printf("WindowBorder(): SetCloseButton\n");
#endif
			decor->SetCloseButton(true);
			break;
		}
		case CLICK_ZOOM:
		{
#ifdef DEBUG_WINBORDER
printf("WindowBorder(): SetZoomButton\n");
#endif
			decor->SetZoomButton(true);
			break;
		}
		case CLICK_MINIMIZE:
		{
#ifdef DEBUG_WINBORDER
printf("WindowBorder(): SetMinimizeButton\n");
#endif
			decor->SetMinimizeButton(true);
			break;
		}
		case CLICK_NONE:
		{
#ifdef DEBUG_WINBORDER
printf("WindowBorder(): Click None\n");
#endif
			break;
		}
		default:
		{
#ifdef DEBUG_WINBORDER
printf("WindowBorder()::MouseDown: Undefined click type\n");
#endif
			break;
		}
	}
}

void WindowBorder::MouseMoved(BPoint pt, uint32 buttons)
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

ServerWindow *WindowBorder::Window(void) const
{
	return swin;
}
