#include "Layer.h"
#include "DisplayDriver.h"
#include <View.h>
#include "BeDecorator.h"

#define DEBUG_DECOR

#ifdef DEBUG_DECOR
#include <stdio.h>
#endif

BeDecorator::BeDecorator(Layer *lay, uint32 dflags, window_look wlook)
 : Decorator(lay, dflags, wlook)
{
#ifdef DEBUG_DECOR
printf("BeDecorator()\n");
#endif
	zoomstate=false;
	closestate=false;
	taboffset=0;

	blue.red=100;
	blue.green=100;
	blue.blue=255;

	blue2.red=150;
	blue2.green=150;
	blue2.blue=255;

	black.red=0;
	black.green=0;
	black.blue=0;

	gray.red=224;
	gray.green=224;
	gray.blue=224;
	
	white.red=224;
	white.green=224;
	white.blue=224;

	yellow.red=255;
	yellow.green=203;
	yellow.blue=0;

	Resize(lay->frame);
}

BeDecorator::~BeDecorator(void)
{
#ifdef DEBUG_DECOR
printf("~BeDecorator()\n");
#endif
}

click_type BeDecorator::Clicked(BPoint pt, uint32 buttons)
{
	// the case order is important - we go from smallest to largest
	if(closerect.Contains(pt))
	{

#ifdef DEBUG_DECOR
printf("BeDecorator():Clicked() - Close\n");
#endif

		return CLICK_CLOSE;
	}

	if(zoomrect.Contains(pt))
	{

#ifdef DEBUG_DECOR
printf("BeDecorator():Clicked() - Zoom\n");
#endif

		return CLICK_ZOOM;
	}
	
	if(resizerect.Contains(pt))
	{

#ifdef DEBUG_DECOR
printf("BeDecorator():Clicked() - Resize thumb\n");
#endif

		return CLICK_RESIZE_RB;
	}

	// Clicking in the tab?
	if(tabrect.Contains(pt))
	{
		// Here's part of our window management stuff
		if(buttons==B_PRIMARY_MOUSE_BUTTON)
			return CLICK_MOVETOFRONT;
		if(buttons==B_SECONDARY_MOUSE_BUTTON)
			return CLICK_MOVETOBACK;
		return CLICK_TAB;
	}

	// We got this far, so user is clicking on the border?
	BRect borderrect(frame);
	borderrect.top+=19;
	BRect clientrect(borderrect.InsetByCopy(2,2));
	if(borderrect.Contains(pt) && !clientrect.Contains(pt))
	{
#ifdef DEBUG_DECOR
printf("BeDecorator():Clicked() - Drag\n");
#endif		
		return CLICK_DRAG;
	}

	// Guess user didn't click anything
#ifdef DEBUG_DECOR
printf("BeDecorator():Clicked()\n");
#endif
	return CLICK_NONE;
}

void BeDecorator::Resize(BRect rect)
{
#ifdef DEBUG_DECOR
printf("BeDecorator()::Resize()"); rect.PrintToStream();
#endif
	frame=rect;
	closerect=frame;
	zoomrect=frame;
	tabrect=frame;
	resizerect=frame;
	borderrect=frame;

	closerect.left+=2;
	closerect.top+=2;
	closerect.right=closerect.left+10;
	closerect.bottom=closerect.top+10;

	zoomrect.top+=2;
	zoomrect.right-=2;
	zoomrect.bottom-=2;
	zoomrect.left=zoomrect.right-10;
	zoomrect.bottom=zoomrect.top+10;

	borderrect.top+=19;
	
	resizerect.top=resizerect.bottom-18;
	resizerect.left=resizerect.right-18;
	
	tabrect.bottom=tabrect.top+18;
}

BRect BeDecorator::GetBorderSize(void)
{
	return bsize;
}

BPoint BeDecorator::GetMinimumSize(void)
{
	return minsize;
}

void BeDecorator::SetFlags(uint32 dflags)
{
	flags=dflags;
}

void BeDecorator::UpdateFont(void)
{
}

void BeDecorator::UpdateTitle(const char *string)
{
}

void BeDecorator::SetFocus(bool bfocused)
{
	focused=bfocused;
}

void BeDecorator::SetCloseButton(bool down)
{
	closestate=down;
}

void BeDecorator::SetZoomButton(bool down)
{
	zoomstate=down;
}

void BeDecorator::Draw(BRect update)
{
#ifdef DEBUG_DECOR
printf("BeDecorator()::Draw():"); update.PrintToStream();
#endif
	// We need to draw a few things: the tab, the resize thumb, the borders,
	// and the buttons

	DrawTab();

	// Draw the top view's client area - just a hack :)
	driver->FillRect(borderrect,blue);
	
	DrawFrame();

}

void BeDecorator::DrawZoom(BRect r)
{
	if(zoomstate)
	{
		driver->FillRect(r,black);
	}	
	else
	{
		driver->FillRect(r,gray);
		driver->StrokeRect(r,black);
	}
}

void BeDecorator::DrawClose(BRect r)
{
	if(closestate)
	{
		driver->FillRect(r,gray);
		driver->StrokeRect(r,black);
		driver->FillRect(r.InsetByCopy(2,2),black);
	}
	else
	{
		driver->FillRect(r,gray);
		driver->StrokeRect(r,black);
		driver->StrokeRect(r.InsetByCopy(2,2),black);
	}
}

void BeDecorator::Draw(void)
{
	// Easy way to draw everything - no worries about drawing only certain
	// things

	DrawTab();

	// Draw the top view's client area - just a hack :)
	driver->FillRect(borderrect,blue);
	
	DrawFrame();
}

void BeDecorator::DrawTab(void)
{
	if(focused)
		driver->FillRect(tabrect,yellow);
	else
		driver->FillRect(tabrect,gray);
	driver->StrokeRect(tabrect,black);
}

void BeDecorator::DrawFrame(void)
{
	driver->StrokeRect(borderrect,black);

	// Draw the resize thumb if we're supposed to
	if(!(flags & B_NOT_RESIZABLE))
	{
		driver->FillRect(resizerect,gray);
		driver->StrokeRect(resizerect,black);
	}

	// Draw the buttons if we're supposed to	
	if(!(flags & B_NOT_CLOSABLE))
		DrawClose(closerect);
	if(!(flags & B_NOT_ZOOMABLE))
		DrawZoom(zoomrect);
}

void BeDecorator::SetLook(window_look wlook)
{
	look=wlook;
}

void BeDecorator::CalculateBorders(void)
{
	switch(look)
	{
		case B_NO_BORDER_WINDOW_LOOK:
		{
			bsize.Set(0,0,0,0);
			break;
		}
		case B_TITLED_WINDOW_LOOK:
		case B_DOCUMENT_WINDOW_LOOK:
		case B_BORDERED_WINDOW_LOOK:
		{
			bsize.top=18;
			break;
		}
		case B_MODAL_WINDOW_LOOK:
		case B_FLOATING_WINDOW_LOOK:
		{
			bsize.top=15;
			break;
		}
		default:
		{
			break;
		}
	}
}
