#include "Layer.h"
#include "DisplayDriver.h"
#include <View.h>
#include "BeDecorator.h"
#include "ColorUtils.h"

//#define DEBUG_DECOR

#ifdef DEBUG_DECOR
#include <stdio.h>
#endif

BeDecorator::BeDecorator(Layer *lay, uint32 dflags, uint32 wlook)
 : Decorator(lay, dflags, wlook)
{
#ifdef DEBUG_DECOR
printf("BeDecorator()\n");
#endif
	zoomstate=false;
	closestate=false;
	taboffset=0;

	// These hard-coded assignments will go bye-bye when the system colors 
	// API is implemented
	SetRGBColor(&tab_highcol,255,236,33);
	SetRGBColor(&tab_lowcol,234,181,0);

	SetRGBColor(&button_highcol,255,255,0);
	SetRGBColor(&button_lowcol,255,203,0);

	SetRGBColor(&frame_highcol,216,216,216);
	SetRGBColor(&frame_midcol,184,184,184);
	SetRGBColor(&frame_lowcol,110,110,110);

	Resize(lay->frame);
	
	// We need to modify the visible rectangle because we have tabbed windows
	if(lay->visible)
	{
		delete lay->visible;
		lay->visible=GetBorderSize();
	}
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

		return CLICK_RESIZE;
	}

	// Clicking in the tab?
	if(tabrect.Contains(pt))
	{
		// Here's part of our window management stuff
		if(buttons==B_PRIMARY_MOUSE_BUTTON && !focused)
			return CLICK_MOVETOFRONT;
		if(buttons==B_SECONDARY_MOUSE_BUTTON)
			return CLICK_MOVETOBACK;
		return CLICK_DRAG;
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
	tabrect=frame;
	resizerect=frame;
	borderrect=frame;

	closerect.left+=2;
	closerect.top+=2;
	closerect.right=closerect.left+10;
	closerect.bottom=closerect.top+10;

	borderrect.top+=19;
	
	resizerect.top=resizerect.bottom-18;
	resizerect.left=resizerect.right-18;
	
	tabrect.bottom=tabrect.top+18;
	if(layer->name)
	{
		float titlewidth=closerect.right
			+driver->StringWidth(layer->name->String(),
			layer->name->Length())+35;
		tabrect.right=(titlewidth<frame.right-1)?titlewidth:frame.right-1;
	}
	else
		tabrect.right=tabrect.left+tabrect.Width()/2;
	
	zoomrect=tabrect;
	zoomrect.top+=2;
	zoomrect.right-=2;
	zoomrect.bottom-=2;
	zoomrect.left=zoomrect.right-10;
	zoomrect.bottom=zoomrect.top+10;
}

void BeDecorator::MoveBy(BPoint pt)
{
	frame.OffsetBy(pt);
	closerect.OffsetBy(pt);
	tabrect.OffsetBy(pt);
	resizerect.OffsetBy(pt);
	borderrect.OffsetBy(pt);
	zoomrect.OffsetBy(pt);
}

BRegion *BeDecorator::GetBorderSize(void)
{
	BRegion *r=new BRegion(borderrect);
	r->Include(tabrect);
	return r;
}

BPoint BeDecorator::GetMinimumSize(void)
{
	return minsize;
}

void BeDecorator::SetFlags(uint32 wflags)
{
	dflags=wflags;
}

void BeDecorator::UpdateFont(void)
{
}

void BeDecorator::UpdateTitle(const char *string)
{
	if(string)
	{
		driver->SetDrawingMode(B_OP_OVER);
		driver->DrawString((char *)string,strlen(string),
			BPoint(closerect.right+7,closerect.bottom));
		driver->SetDrawingMode(B_OP_COPY);
	}

}

void BeDecorator::SetFocus(bool bfocused)
{
	focused=bfocused;

	if(focused)
	{
		SetRGBColor(&tab_highcol,255,236,33);
		SetRGBColor(&tab_lowcol,234,181,0);
	}
	else
	{
		SetRGBColor(&tab_highcol,235,235,235);
		SetRGBColor(&tab_lowcol,160,160,160);
	}

	button_highcol=tab_highcol;
	button_lowcol=tab_lowcol;
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
	// We need to draw a few things: the tab, the resize thumb, the borders,
	// and the buttons

	if(tabrect.Intersects(update))
		DrawTab();

	// Draw the top view's client area - just a hack :)
	rgb_color blue={100,100,255,255};

	if(borderrect.Intersects(update))
		driver->FillRect(borderrect,blue);
	
	DrawFrame();

}

void BeDecorator::Draw(void)
{
	// Easy way to draw everything - no worries about drawing only certain
	// things

	DrawTab();

	// Draw the top view's client area - just a hack :)
	rgb_color blue={100,100,255,255};
	driver->FillRect(borderrect,blue);
	
	DrawFrame();
}

void BeDecorator::DrawZoom(BRect r)
{
	BRect zr=r;
	zr.left+=zr.Width()/3;
	zr.top+=zr.Height()/3;

	DrawBlendedRect(zr,zoomstate);
	DrawBlendedRect(zr.OffsetToCopy(r.LeftTop()),
		zoomstate);
}

void BeDecorator::DrawClose(BRect r)
{
	DrawBlendedRect(r,closestate);
}

void BeDecorator::DrawTab(void)
{
	rgb_color tmpcol;
	float rstep,gstep,bstep;

	int steps=tabrect.IntegerHeight();
	rstep=(tab_highcol.red-tab_lowcol.red)/steps;
	gstep=(tab_highcol.green-tab_lowcol.green)/steps;
	bstep=(tab_highcol.blue-tab_lowcol.blue)/steps;

	for(float i=0;i<=steps; i++)
	{
		SetRGBColor(&tmpcol, uint8(tab_highcol.red-(i*rstep)),
			uint8(tab_highcol.green-(i*gstep)),
			uint8(tab_highcol.blue-(i*bstep)));
		driver->StrokeLine(BPoint(tabrect.left,tabrect.top+i),
			BPoint(tabrect.right,tabrect.top+i),tmpcol);
	}

	UpdateTitle(layer->name->String());

	// Draw the buttons if we're supposed to	
	if(!(dflags & NOT_CLOSABLE))
		DrawClose(closerect);
	if(!(dflags & NOT_ZOOMABLE))
		DrawZoom(zoomrect);
}

void BeDecorator::DrawBlendedRect(BRect r, bool down)
{
	// This bad boy is used to draw a rectangle with a gradient.
	// Note that it is not part of the Decorator API - it's specific
	// to just the BeDecorator. Called by DrawZoom and Draw Close

	// Actually just draws a blended square
	int32 w=r.IntegerWidth(),  h=r.IntegerHeight();

	rgb_color tmpcol,halfcol, startcol, endcol;
	float rstep,gstep,bstep,i;

	int steps=(w<h)?w:h;

	if(down)
	{
		startcol=button_lowcol;
		endcol=button_highcol;
	}
	else
	{
		startcol=button_highcol;
		endcol=button_lowcol;
	}

	SetRGBColor(&halfcol,(startcol.red+endcol.red)/2,
		(startcol.green+endcol.green)/2,
		(startcol.blue+endcol.blue)/2);

	rstep=(startcol.red-halfcol.red)/steps;
	gstep=(startcol.green-halfcol.green)/steps;
	bstep=(startcol.blue-halfcol.blue)/steps;

	for(i=0;i<=steps; i++)
	{
		SetRGBColor(&tmpcol, uint8(startcol.red-(i*rstep)),
			uint8(startcol.green-(i*gstep)),
			uint8(startcol.blue-(i*bstep)));
		driver->StrokeLine(BPoint(r.left,r.top+i),
			BPoint(r.left+i,r.top),tmpcol);

		SetRGBColor(&tmpcol, uint8(halfcol.red-(i*rstep)),
			uint8(halfcol.green-(i*gstep)),
			uint8(halfcol.blue-(i*bstep)));
		driver->StrokeLine(BPoint(r.left+steps,r.top+i),
			BPoint(r.left+i,r.top+steps),tmpcol);

	}
	
	SetRGBColor(&tmpcol, 128,128,0);
	driver->StrokeRect(r,tmpcol);
}

void BeDecorator::DrawFrame(void)
{
	BRect r=borderrect;
	bool down=false;
	
	r.InsetBy(1,1);
	driver->StrokeRect(r,frame_midcol);

	r.InsetBy(-1,-1);
	driver->StrokeLine(BPoint(r.left,r.top),BPoint(r.right,r.top),
		frame_highcol);
	driver->StrokeLine(BPoint(r.left,r.top),BPoint(r.left,r.bottom),
		frame_highcol);
	driver->StrokeLine(BPoint(r.right,r.bottom),BPoint(r.right,r.top-1),
		frame_lowcol);
	driver->StrokeLine(BPoint(r.right,r.bottom),BPoint(r.left-1,r.bottom),
		frame_lowcol);
	
	// Draw the resize thumb if we're supposed to
	if(!(dflags & NOT_RESIZABLE))
	{
		r=resizerect;

		int32 w=r.IntegerWidth(),  h=r.IntegerHeight();
		
		// This code is strictly for B_DOCUMENT_WINDOW looks
		if(dlook==WLOOK_DOCUMENT)
		{
			rgb_color tmpcol,halfcol, startcol, endcol;
			float rstep,gstep,bstep,i;
		
			int steps=(w<h)?w:h;
		
			if(down)
			{
				startcol=frame_lowcol;
				endcol=frame_highcol;
			}
			else
			{
				startcol=frame_highcol;
				endcol=frame_lowcol;
			}
		
			SetRGBColor(&halfcol,(startcol.red+endcol.red)/2,
				(startcol.green+endcol.green)/2,
				(startcol.blue+endcol.blue)/2);
		
			rstep=(startcol.red-halfcol.red)/steps;
			gstep=(startcol.green-halfcol.green)/steps;
			bstep=(startcol.blue-halfcol.blue)/steps;
		
			for(i=0;i<=steps; i++)
			{
				SetRGBColor(&tmpcol, uint8(startcol.red-(i*rstep)),
					uint8(startcol.green-(i*gstep)),
					uint8(startcol.blue-(i*bstep)));
				driver->StrokeLine(BPoint(r.left,r.top+i),
					BPoint(r.left+i,r.top),tmpcol);
		
				SetRGBColor(&tmpcol, uint8(halfcol.red-(i*rstep)),
					uint8(halfcol.green-(i*gstep)),
					uint8(halfcol.blue-(i*bstep)));
				driver->StrokeLine(BPoint(r.left+steps,r.top+i),
					BPoint(r.left+i,r.top+steps),tmpcol);
		
			}
			
			SetRGBColor(&tmpcol, 128,128,0);
			driver->StrokeRect(r,tmpcol);
		}
		else
		{
			
		}
	}
}

void BeDecorator::SetLook(uint32 wlook)
{
	dlook=wlook;
}

void BeDecorator::CalculateBorders(void)
{
	switch(dlook)
	{
		case WLOOK_NO_BORDER:
		{
			bsize.Set(0,0,0,0);
			break;
		}
		case WLOOK_TITLED:
		case WLOOK_DOCUMENT:
		case WLOOK_BORDERED:
		{
			bsize.top=18;
			break;
		}
		case WLOOK_MODAL:
		case WLOOK_FLOATING:
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
