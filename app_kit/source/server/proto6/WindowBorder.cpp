#include <String.h>
#include <Locker.h>
#include <Region.h>
#include <Debug.h>
#include "View.h"	// for mouse button defines
#include "ServerWindow.h"
#include "WindowBorder.h"
#include "Decorator.h"
#include "Desktop.h"

//#define DEBUG_WINBORDER

#ifdef DEBUG_WINBORDER
#include <stdio.h>
#endif

bool is_moving_window=false;
bool is_resizing_window=false;
WindowBorder *activeborder=NULL;

WindowBorder::WindowBorder(ServerWindow *win, const char *bordertitle)
 : Layer((win==NULL)?BRect(0,0,0,0):win->frame,
   (win==NULL)?NULL:win->title->String())
{
#ifdef DEBUG_WINBORDER
printf("WindowBorder()\n");
#endif
	mbuttons=0;
	swin=win;

	title=new BString(bordertitle);
	hresizewin=false;
	vresizewin=false;
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
	mbuttons=buttons;
	click_type click=decor->Clicked(pt, mbuttons);

	switch(click)
	{
		case CLICK_MOVETOBACK:
		{
			MoveToBack();
			break;
		}
		case CLICK_MOVETOFRONT:
		{
			MoveToFront();
			break;
		}
		case CLICK_CLOSE:
		{
#ifdef DEBUG_WINBORDER
printf("WindowBorder(): SetCloseButton\n");
#endif
			decor->SetCloseButton(true);
			swin->SetFocus(true);
			decor->SetFocus(true);
			decor->Draw();
			activeborder=this;
			break;
		}
		case CLICK_ZOOM:
		{
#ifdef DEBUG_WINBORDER
printf("WindowBorder(): SetZoomButton\n");
#endif
			decor->SetZoomButton(true);
			swin->SetFocus(true);
			decor->SetFocus(true);
			decor->Draw();
			activeborder=this;
			break;
		}
		case CLICK_MINIMIZE:
		{
#ifdef DEBUG_WINBORDER
printf("WindowBorder(): SetMinimizeButton\n");
#endif
			decor->SetMinimizeButton(true);
			swin->SetFocus(true);
			decor->SetFocus(true);
			decor->Draw();
			activeborder=this;
			break;
		}
		case CLICK_DRAG:
		{
#ifdef DEBUG_WINBORDER
printf("WindowBorder(): Click Tab\n");
#endif
			if(buttons==B_PRIMARY_MOUSE_BUTTON)
			{
				is_moving_window=true;
				activeborder=this;
			}
			if(buttons==B_SECONDARY_MOUSE_BUTTON)
				MoveToBack();
			break;
		}
		case CLICK_RESIZE:
		{
#ifdef DEBUG_WINBORDER
printf("WindowBorder(): Click Resize\n");
#endif
			if(buttons==B_PRIMARY_MOUSE_BUTTON)
			{
				is_resizing_window=true;
				activeborder=this;
			}
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
	click_type click=decor->Clicked(pt, mbuttons);
	if(click!=CLICK_CLOSE && decor->GetCloseButton())
	{
		decor->SetCloseButton(false);
		decor->Draw();
	}	

	if(click!=CLICK_ZOOM && decor->GetZoomButton())
	{
		decor->SetZoomButton(false);
		decor->Draw();
	}	

	if(click!=CLICK_MINIMIZE && decor->GetMinimizeButton())
	{
		decor->SetMinimizeButton(false);
		decor->Draw();
	}	

	if(is_moving_window==true)
	{
		float dx=pt.x-mousepos.x,
			dy=pt.y-mousepos.y;
		if(dx!=0 || dy!=0)
		{
			clientframe.OffsetBy(pt);
	
			swin->Lock();
			swin->frame.OffsetBy(dx,dy);
			swin->Unlock();
	
			layerlock->Lock();
			MoveBy(dx,dy);
			parent->RequestDraw();
			layerlock->Unlock();
			decor->MoveBy(BPoint(dx, dy));
			decor->Draw();
		}
	}
	if(is_resizing_window==true)
	{
		float dx=pt.x-mousepos.x,
			dy=pt.y-mousepos.y;
		if(dx!=0 || dy!=0)
		{
			clientframe.right+=dx;
			clientframe.bottom+=dy;
	
			swin->Lock();
			swin->frame.right+=dx;
			swin->frame.bottom+=dy;
			swin->Unlock();
	
			layerlock->Lock();
			ResizeBy(dx,dy);
			parent->RequestDraw();
			layerlock->Unlock();
			decor->Resize(swin->frame);
			decor->Draw();
		}
	}
	mousepos=pt;
}

void WindowBorder::MouseUp(BPoint pt, uint32 buttons)
{
//	mbuttons&= ~buttons;
	mbuttons=buttons;
	activeborder=NULL;
	is_moving_window=false;
	is_resizing_window=false;

	click_type click=decor->Clicked(pt, mbuttons);

	switch(click)
	{
		case CLICK_CLOSE:
		{
			decor->SetCloseButton(false);
			decor->Draw();
			
			// call close window stuff here
			
			break;
		}
		case CLICK_ZOOM:
		{
			decor->SetZoomButton(false);
			decor->Draw();
			
			// call zoom stuff here
			
			break;
		}
		case CLICK_MINIMIZE:
		{
			decor->SetMinimizeButton(false);
			decor->Draw();
			
			// call minimize stuff here
			
		}
		default:
		{
			break;
		}
	}
}

void WindowBorder::Draw(BRect update_rect)
{
#ifdef DEBUG_WINBORDER
printf("WindowBorder()::Draw():"); update_rect.PrintToStream();
#endif

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
#ifdef DEBUG_WINBORDER
printf("WindowBorder::SetDecorator(%p)\n",newdecor);
#endif
	decor=newdecor;
}

ServerWindow *WindowBorder::Window(void) const
{
	return swin;
}

void WindowBorder::RequestDraw(void)
{
/*	if(invalid)
	{
		for(int32 i=0; i<invalid->CountRects();i++)
			decor->Draw(ConvertToTop(invalid->RectAt(i)));
		delete invalid;
		invalid=NULL;
		is_dirty=false;
	}
*/
}

void WindowBorder::MoveToBack(void)
{
#ifdef DEBUG_WINBORDER
printf("WindowBorder(): MoveToBack\n");
#endif
	swin->SetFocus(false);

	// Move the window to the back (the top of the tree)
	Layer *top=GetRootLayer();
	ASSERT(top!=NULL);
	layerlock->Lock();
	top->RemoveChild(this);
	top->AddChild(this);
	swin->SetFocus(false);
	decor->SetFocus(false);
	decor->Draw();
	layerlock->Unlock();
}

void WindowBorder::MoveToFront(void)
{
#ifdef DEBUG_WINBORDER
printf("WindowBorder(): MoveToFront\n");
#endif
	swin->SetFocus(true);

	// Move the window to the front by making it the bottom
	// child of the root layer
	Layer *top=GetRootLayer();
	layerlock->Lock();
	top->RemoveChild(this);
	uppersibling=top->bottomchild;
	lowersibling=NULL;
	top->bottomchild=this;
	if(top->topchild==NULL)
		top->topchild=this;
	parent=top;
	swin->SetFocus(true);
	decor->SetFocus(true);
	decor->Draw();
	layerlock->Unlock();
	is_moving_window=true;
	activeborder=this;
}
