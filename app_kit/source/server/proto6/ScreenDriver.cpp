/*
	ScreenDriver.cpp
		Replacement class for the ViewDriver which utilizes a BWindowScreen for ease
		of testing without requiring a second video card for SecondDriver and also
		without the limitations (mostly speed) of ViewDriver.
		
		Note that unlike ViewDriver, this graphics module does NOT emulate
		the Input Server. The Router input server filter is required for use.
		
		The concept is pretty close to the retooled ViewDriver, where each module
		call locks a couple BLockers and draws to the buffer.
		
		Components:
			ScreenDriver: actual driver module
			FrameBuffer: BWindowScreen derivative which provides the module access
				to the video card
			Internal functions for doing graphics on the buffer
		
		Done:
			Basic module init and shutdown
			Clear() - 8-bit and 32-bit
			SetPixel32
			StrokeLine(pt,pt,rgb_color)
			StrokeRect - both forms
			FillRect - both forms, but not using fastest methods
		ToDo:
			16-Bit mode functions
			StrokeLine(pt,pattern) needs to utilize patterns
			FillRect(rect,pattern) needs to utilize patterns
			Implement a lot of functions
*/
#include "ScreenDriver.h"
#include "ServerCursor.h"
#include "ServerProtocol.h"
#include "ServerBitmap.h"
#include "ColorUtils.h"
#include "PortLink.h"
#include "DebugTools.h"
#include <View.h>
#include <stdio.h>
#include <string.h>
#include <String.h>

#define DEBUG_DRIVER

// Define this if you want to use the spacebar to launch the server prototype's
// test application
#define LAUNCH_TESTAPP

#ifdef LAUNCH_TESTAPP
#include <Roster.h>
#include <Entry.h>
#endif

// functions internal to the driver
void Clear_32Bit(void *buffer, int width, int height, rgb_color col);
void Clear_16Bit(void *buffer, int width, int height, uint16 col);
void Clear_8Bit(void *buffer, int width, int height, uint8 col);

void HLine_32Bit(graphics_card_info i, uint16 x, uint16 y, uint16 length, rgb_color col);
void HLine_16Bit(graphics_card_info i, uint16 x, uint16 y, uint16 length, uint16 col);
void HLine_16Bit(graphics_card_info i, uint16 x, uint16 y, uint16 length, uint8 col);

FrameBuffer::FrameBuffer(const char *title, uint32 space, status_t *st,bool debug)
	: BWindowScreen(title,space,st,debug)
{
	is_connected=false;
}

FrameBuffer::~FrameBuffer(void)
{
}

void FrameBuffer::ScreenConnected(bool connected)
{
	is_connected=connected;
	if(connected)
	{
		// Cache the state just in case
		graphics_card_info *info=CardInfo();
		gcinfo=*info;
	}
}

void FrameBuffer::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		case B_KEY_DOWN:
		{
			int32 key;
			msg->FindInt32("key",&key);
			if(key==0x47)	// Enter key
			{
				port_id serverport=find_port(SERVER_PORT_NAME);
				write_port(serverport,B_QUIT_REQUESTED,NULL,0);
				break;
			}
			#ifdef LAUNCH_TESTAPP
			if(key==0x5e)
			{
				printf("Launching test app\n");
				status_t value;
				BEntry entry("/boot/home/Desktop/openbeos/proto6/OBApplication/OBApplication");
				entry_ref ref;
				entry.GetRef(&ref);
				value=be_roster->Launch(&ref);

				BString str=TranslateStatusToString(value);
				printf("Launch return code: %s",str.String());
				printf("\n");
			}
			#endif
		}
		default:
			BWindowScreen::MessageReceived(msg);
	}
}

ScreenDriver::ScreenDriver(void)
{
	status_t st;
	fbuffer=new FrameBuffer("OBAppServer",B_8_BIT_640x480,&st,true);

	drawmode=DRAW_COPY;	
	// Create our cursor here
	cursor=new ServerBitmap(BRect(0,0,16,16),B_CMAP8);
}

ScreenDriver::~ScreenDriver(void)
{
	delete cursor;
}

void ScreenDriver::Initialize(void)
{
	fbuffer->Show();

	// draw on the module's default cursor here to make it look preety
	
	// wait 1 sec for the window to show...
	snooze(500000);
}

void ScreenDriver::Shutdown(void)
{
	fbuffer->Lock();
	fbuffer->Quit();
}

void ScreenDriver::Clear(uint8 red,uint8 green,uint8 blue)
{
	locker->Lock();
	rgb_color c;
	SetRGBColor(&c,red,green,blue);
	Clear(c);
	locker->Unlock();
}

void ScreenDriver::Clear(rgb_color col)
{
	locker->Lock();
#ifdef DEBUG_DRIVER
printf("ScreenDriver::Clear(%u,%u,%u)\n",col.red,col.green,col.blue);
#endif
	if(fbuffer->IsConnected())
	{
		switch(fbuffer->gcinfo.bits_per_pixel)
		{	
			case 32:
			case 24:
			{
printf("Clear: 32/24-bit\n");
				Clear_32Bit(fbuffer->gcinfo.frame_buffer,
					fbuffer->gcinfo.width,fbuffer->gcinfo.height,col);
				break;
			}
			case 16:
			case 15:
			{
printf("Clear: 16/15-bit unimplemented\n");
				// Look up color here
//				uint16 c;
//				Clear_16Bit(gcinfo.frame_buffer,gcinfo.width,gcinfo.height,c);
				break;
			}
			case 8:
			{
printf("Clear: 8-bit\n");
				// Look up color here
				uint8 c=FindClosestColor(fbuffer->ColorList(),col);
				
				Clear_8Bit(fbuffer->gcinfo.frame_buffer,
					fbuffer->gcinfo.width,fbuffer->gcinfo.height, c);
				break;
			}
			default:
			{
#ifdef DEBUG_DRIVER
printf("Clear: unknown bit depth %u\n",fbuffer->gcinfo.bits_per_pixel);
#endif
				break;
			}
		}
	}
	else
	{
#ifdef DEBUG_DRIVER
printf("Clear: driver is disconnected\n");
#endif
	}
	locker->Unlock();
}

void ScreenDriver::DrawBitmap(ServerBitmap *bitmap, BRect source, BRect dest)
{
	locker->Lock();
#ifdef DEBUG_DRIVER
printf("ScreenDriver::DrawBitmap(*, (%f,%f,%f,%f),(%f,%f,%f,%f) )\n",
	source.left,source.top,source.right,source.bottom,
	dest.left,dest.top,dest.right,dest.bottom);
#endif
	if(fbuffer->IsConnected())
	{
		// Scale bitmap here if source!=dest
	
		switch(fbuffer->gcinfo.bits_per_pixel)
		{	
			case 32:
			case 24:
			{
				printf("DrawBitmap: 32/24-bit\n");
				DrawServerBitmap32(bitmap,dest.LeftTop(),drawmode);
				break;
			}
			case 16:
			case 15:
			{
				printf("DrawBitmap: 16/15-bit unimplemented\n");
				break;
			}
			case 8:
			{
				printf("DrawBitmap: 8-bit unimplemented\n");
				break;
			}
			default:
			{
#ifdef DEBUG_DRIVER
printf("Clear: unknown bit depth %u\n",fbuffer->gcinfo.bits_per_pixel);
#endif
				break;
			}
		}
	}

	locker->Unlock();
}

void ScreenDriver::FillRect(BRect rect, uint8 *pattern)
{
	locker->Lock();

	FillRect(rect,highcol);

	locker->Unlock();
}

void ScreenDriver::FillRect(BRect rect, rgb_color col)
{
#ifdef DEBUG_DRIVER
printf("ScreenDriver::FillRect( (%f,%f,%f,%f), {%u,%u,%u,%u}\n",rect.left,rect.top,
	rect.right,rect.bottom,col.red,col.green,col.blue,col.alpha);
#endif
	locker->Lock();
	if(fbuffer->IsConnected())
	{
	//	int32 width=rect.IntegerWidth();
		for(int32 i=(int32)rect.top;i<=rect.bottom;i++)
	//		HLine(fbuffer->gcinfo,(int32)rect.left,i,width,col);
			StrokeLine(BPoint(rect.left,i),BPoint(rect.right,i),col);
	}
	locker->Unlock();
}

void ScreenDriver::FillRoundRect(BRect rect,float xradius, float yradius, uint8 *pattern)
{
#ifdef DEBUG_DRIVER
printf("ScreenDriver::FillRoundRect( (%f,%f,%f,%f), %llx) ---->Unimplemented<----\n",rect.left,rect.top,
	rect.right,rect.bottom,*((uint64*)pattern));
#endif
	FillRect(rect,pattern);
}

void ScreenDriver::FillTriangle(BPoint first, BPoint second, BPoint third, BRect rect, uint8 *pattern)
{
#ifdef DEBUG_DRIVER
printf("ScreenDriver::FillTriangle(%llx): --->Currently Unimplemented<---\n",*((uint64*)pattern));
printf("\tFirst: (%f,%f)\n\tSecond: (%f,%f)\n\tThird: (%f,%f)\n",first.x,first.y,
	second.x,second.y,third.x,third.y);
rect.PrintToStream();
#endif
	locker->Lock();
	if(fbuffer->IsConnected())
	{
		BPoint oldpos=penpos;
		MovePenTo(first); StrokeLine(second,pattern);
		MovePenTo(second); StrokeLine(third,pattern);
		MovePenTo(third); StrokeLine(first,pattern);
		penpos=oldpos;
	}
	locker->Unlock();
}

void ScreenDriver::FillTriangle(BPoint first, BPoint second, BPoint third, BRect rect, rgb_color col)
{
#ifdef DEBUG_DRIVER
printf("ScreenDriver::FillTriangle({%u,%u,%u,%u}): --->Currently Unimplemented<---\n",
	col.red,col.green,col.blue,col.alpha);
printf("\tFirst: (%f,%f)\n\tSecond: (%f,%f)\n\tThird: (%f,%f)\n",first.x,first.y,
	second.x,second.y,third.x,third.y);
rect.PrintToStream();
#endif
	locker->Lock();
	if(fbuffer->IsConnected())
	{
		StrokeLine(first,second,col);
		StrokeLine(first,third,col);
		StrokeLine(third,second,col);
	}
	locker->Unlock();
}

void ScreenDriver::SetPixel(int x, int y, uint8 *pattern)
{
	switch(fbuffer->gcinfo.bits_per_pixel)
	{
		case 32:
		case 24:
			SetPixel32(x,y,highcol);
			break;
		case 16:
		case 15:
			SetPixel16(x,y,FindClosestColor16(highcol));
			break;
		default:
			printf("SetPixel8 Unimplemented\n");
//			SetPixel8(x,y,FindClosestColor(highcol));
			break;
	}

}

void ScreenDriver::SetPixel32(int x, int y, rgb_color col)
{
	// Code courtesy of YNOP's SecondDriver
	union
	{
		uint8 bytes[4];
		uint32 word;
	}c1;
	
	c1.bytes[0]=col.blue; 
	c1.bytes[1]=col.green; 
	c1.bytes[2]=col.red; 
	c1.bytes[3]=col.alpha; 

	uint32 *bits=(uint32*)fbuffer->gcinfo.frame_buffer;
	*(bits + x + (y*fbuffer->gcinfo.width))=c1.word;
}

void ScreenDriver::SetPixel16(int x, int y, uint16 col)
{
#ifdef DEBUG_DRIVER
printf("SetPixel16 unimplemented\n");
#endif
}

void ScreenDriver::SetPixel8(int x, int y, uint8 col)
{
	// When the DisplayDriver API changes, we'll use the uint8 highcolor. Until then,
	// we'll use *pattern
	uint8 *bits=(uint8*)fbuffer->gcinfo.frame_buffer;
	*(bits + x + (y*fbuffer->gcinfo.bytes_per_row))=col;
}

void ScreenDriver::SetScreen(uint32 space)
{
#ifdef DEBUG_DRIVER
printf("ScreenDriver::SetScreen(%lu}\n",space);
#endif
	locker->Lock();
	if(fbuffer->IsConnected())
	{
		fbuffer->SetSpace(space);
		
		// We have changed the frame buffer info, so update the cached info
		graphics_card_info *info=fbuffer->CardInfo();
		fbuffer->gcinfo=*info;
	}
	locker->Unlock();
}

void ScreenDriver::StrokeLine(BPoint point, uint8 *pattern)
{
#ifdef DEBUG_DRIVER
printf("ScreenDriver::StrokeLine( (%f,%f), %llx}\n",point.x,point.y,*((uint64*)pattern));
#endif
	locker->Lock();
	if(fbuffer->IsConnected())
	{
		// Courtesy YNOP's SecondDriver with minor changes by DW
		int oct=0;
		int xoff=(int32)penpos.x;
		int yoff=(int32)penpos.y; 
		int32 x2=(int32)point.x-xoff;
		int32 y2=(int32)point.y-yoff; 
		int32 x1=0;
		int32 y1=0;
		if(y2<0){ y2=-y2; oct+=4; }//bit2=1
		if(x2<0){ x2=-x2; oct+=2;}//bit1=1
		if(x2<y2){ int t=x2; x2=y2; y2=t; oct+=1;}//bit0=1
		int x=x1,
			y=y1,
			sum=x2-x1,
			Dx=2*(x2-x1),
			Dy=2*(y2-y1);
		for(int i=0; i <= x2-x1; i++)
		{
			switch(oct)
			{
				case 0:SetPixel(( x)+xoff,( y)+yoff,pattern);break;
				case 1:SetPixel(( y)+xoff,( x)+yoff,pattern);break;
	 			case 3:SetPixel((-y)+xoff,( x)+yoff,pattern);break;
				case 2:SetPixel((-x)+xoff,( y)+yoff,pattern);break;
				case 6:SetPixel((-x)+xoff,(-y)+yoff,pattern);break;
				case 7:SetPixel((-y)+xoff,(-x)+yoff,pattern);break;
				case 5:SetPixel(( y)+xoff,(-x)+yoff,pattern);break;
				case 4:SetPixel(( x)+xoff,(-y)+yoff,pattern);break;
			}
			x++;
			sum-=Dy;
			if(sum < 0)
			{
				y++;
				sum += Dx;
			}
		}
	}
	// Not supposed to move to the LineTo position - this is similar to Windows in
	// that respect	
//	MovePenTo(point); // ends up in last position
	locker->Unlock();
}

void ScreenDriver::StrokeLine(BPoint start, BPoint end, rgb_color col)
{
#ifdef DEBUG_DRIVER
printf("ScreenDriver::StrokeLine( (%f,%f),(%f,%f), {%u,%u,%u,%u}\n",start.x,start.y,
	end.x,end.y,col.red,col.green,col.blue,col.alpha);
#endif
	locker->Lock();
	if(fbuffer->IsConnected())
	{
		// Courtesy YNOP's SecondDriver with minor changes by DW
		rgb_color savecol=highcol;
		SetHighColor(col.red,col.green,col.blue);
	
		int oct=0;
		int xoff=(int32)start.x;
		int yoff=(int32)start.y; 
		int32 x2=(int32)end.x-xoff;
		int32 y2=(int32)end.y-yoff; 
		int32 x1=0;
		int32 y1=0;
		if(y2<0){ y2=-y2; oct+=4; }//bit2=1
		if(x2<0){ x2=-x2; oct+=2;}//bit1=1
		if(x2<y2){ int t=x2; x2=y2; y2=t; oct+=1;}//bit0=1
		int x=x1,
			y=y1,
			sum=x2-x1,
			Dx=2*(x2-x1),
			Dy=2*(y2-y1);
		for(int i=0; i <= x2-x1; i++)
		{
			switch(oct)
			{
				case 0:SetPixel(( x)+xoff,( y)+yoff,NULL);break;
				case 1:SetPixel(( y)+xoff,( x)+yoff,NULL);break;
				case 3:SetPixel((-y)+xoff,( x)+yoff,NULL);break;
				case 2:SetPixel((-x)+xoff,( y)+yoff,NULL);break;
				case 6:SetPixel((-x)+xoff,(-y)+yoff,NULL);break;
				case 7:SetPixel((-y)+xoff,(-x)+yoff,NULL);break;
				case 5:SetPixel(( y)+xoff,(-x)+yoff,NULL);break;
				case 4:SetPixel(( x)+xoff,(-y)+yoff,NULL);break;
			}
			x++;
			sum-=Dy;
			if(sum < 0)
			{
				y++;
				sum += Dx;
			}
		}
		SetHighColor(savecol.red,savecol.green,savecol.blue);
	}
	locker->Unlock();
}

void ScreenDriver::StrokePolygon(int *x, int *y, int numpoints, bool is_closed)
{
#ifdef DEBUG_DRIVER
printf("ScreenDriver::StrokePolygon( is_closed=%s)\n",(is_closed==true)?"true":"false");
for(int debug=0;debug<numpoints; debug++)
	printf("Point %d: (%d,%d)\n",debug,x[debug],y[debug]);
#endif
	locker->Lock();
	if(fbuffer->IsConnected())
	{
		for(int32 i=0; i<(numpoints-1); i++)
			StrokeLine(BPoint(x[i],y[i]),BPoint(x[i+1],y[i+1]),highcol);
	
		if(is_closed)
			StrokeLine(BPoint(x[numpoints-1],y[numpoints-1]),BPoint(x[0],y[0]),highcol);
	}
	locker->Unlock();
}

void ScreenDriver::StrokeRect(BRect rect,uint8 *pattern)
{
#ifdef DEBUG_DRIVER
printf("ScreenDriver::StrokeRect( (%f,%f,%f,%f), %llx)\n",rect.left,rect.top,
	rect.right,rect.bottom,*((uint64*)pattern));
#endif
	locker->Lock();
	if(fbuffer->IsConnected())
	{
		BPoint oldpos=penpos;
		
		MovePenTo(rect.LeftTop()); StrokeLine(BPoint(rect.right,rect.top),pattern);
		MovePenTo(BPoint(rect.right,rect.top)); StrokeLine(BPoint(rect.right,rect.bottom),pattern);
		MovePenTo(BPoint(rect.right,rect.bottom)); StrokeLine(BPoint(rect.left,rect.bottom),pattern);
		MovePenTo(BPoint(rect.left,rect.bottom)); StrokeLine(BPoint(rect.left,rect.top),pattern);
	
		penpos=oldpos;
	}
	locker->Unlock();
}

void ScreenDriver::StrokeRect(BRect rect,rgb_color col)
{
#ifdef DEBUG_DRIVER
printf("ScreenDriver::StrokeRect( (%f,%f,%f,%f), {%u,%u,%u,%u}\n",rect.left,rect.top,
	rect.right,rect.bottom,col.red,col.green,col.blue,col.alpha);
#endif
	locker->Lock();
	if(fbuffer->IsConnected())
	{
		StrokeLine(BPoint(rect.left,rect.top),BPoint(rect.right,rect.top),col);
		StrokeLine(BPoint(rect.right,rect.top),BPoint(rect.right,rect.bottom),col);
		StrokeLine(BPoint(rect.right,rect.bottom),BPoint(rect.left,rect.bottom),col);
		StrokeLine(BPoint(rect.left,rect.bottom),BPoint(rect.left,rect.top),col);
	}
	locker->Unlock();
}

void ScreenDriver::StrokeRoundRect(BRect rect,float xradius, float yradius, uint8 *pattern)
{
#ifdef DEBUG_DRIVER
printf("ScreenDriver::StrokeRoundRect( (%f,%f,%f,%f), %llx) ---->Unimplemented<----\n",rect.left,rect.top,
	rect.right,rect.bottom,*((uint64*)pattern));
#endif
	StrokeRect(rect,pattern);
}

void ScreenDriver::StrokeTriangle(BPoint first, BPoint second, BPoint third, BRect rect, uint8 *pattern)
{
#ifdef DEBUG_DRIVER
printf("ScreenDriver::StrokeTriangle(%llx):\n",*((uint64*)pattern));
printf("\tFirst: (%f,%f)\n\tSecond: (%f,%f)\n\tThird: (%f,%f)\n",first.x,first.y,
	second.x,second.y,third.x,third.y);
rect.PrintToStream();
#endif
	locker->Lock();
	if(fbuffer->IsConnected())
	{
		BPoint oldpos=penpos;
		MovePenTo(first); StrokeLine(second,pattern);
		MovePenTo(second); StrokeLine(third,pattern);
		MovePenTo(third); StrokeLine(first,pattern);
		penpos=oldpos;
	}
	locker->Unlock();
}

void ScreenDriver::StrokeTriangle(BPoint first, BPoint second, BPoint third, BRect rect, rgb_color col)
{
#ifdef DEBUG_DRIVER
printf("ScreenDriver::StrokeTriangle({%u,%u,%u,%u}):\n",
	col.red,col.green,col.blue,col.alpha);
printf("\tFirst: (%f,%f)\n\tSecond: (%f,%f)\n\tThird: (%f,%f)\n",first.x,first.y,
	second.x,second.y,third.x,third.y);
rect.PrintToStream();
#endif
	locker->Lock();
	if(fbuffer->IsConnected())
	{
		StrokeLine(first,second,col);
		StrokeLine(first,third,col);
		StrokeLine(third,second,col);
	}
	locker->Unlock();
}

void Clear_32Bit(void *buffer, int width, int height, rgb_color col)
{
#ifdef DEBUG_DRIVER
printf("Clear_32Bit(*,%d,%d,(r=%u,g=%u,b=%u,a=%u)\n",width,height,col.red,col.green,
	col.blue,col.alpha);
#endif

	uint32 c=0,*pcolor=(uint32*)buffer;
	
	// apparently width=bytes per row
	uint32 bpr=width;
	
	// ARGB order
	c|=col.alpha * 0x01000000;
	c|=col.red * 0x010000;
	c|=col.green * 0x0100;
	c|=col.blue;

#ifdef DEBUG_DRIVER
printf("Clear color value: 0x%lx\n",c);
#endif
	for(int32 j=0;j<height;j++)
	{
		pcolor=(uint32*)buffer;
		pcolor+=j*bpr;
		for(int32 i=0;i<width;i++)
			pcolor[i]=c;
	}
}

void Clear_16Bit(void *buffer, int width, int height, uint16 col)
{
#ifdef DEBUG_DRIVER
printf("Clear_16Bit() Unimplemented\n");
#endif

}

void Clear_8Bit(void *buffer, int width, int height, uint8 col)
{
#ifdef DEBUG_DRIVER
printf("Clear_8Bit(*,%d,%d,color %d)\n",width,height,col);
#endif

	uint8 *pcolor=(uint8*)buffer;
	uint32 bpr=width;
	if( (bpr%2)!=0 )
		bpr++;
	
	for(int32 j=0;j<height;j++)
	{
		pcolor=(uint8*)buffer;
		pcolor+=j*bpr;

		memset(pcolor,col,bpr);
	}
}

void HLine(graphics_card_info i, uint16 x, uint16 y, uint16 length, rgb_color col)
{
	switch(i.bits_per_pixel)
	{
		case 32:
		case 24:
			HLine(i,x,y,length,col);
			break;
		case 16:
		case 15:
//			HLine(i,x,y,length,col);
			break;
		default:
//			HLine(i,x,y,length,col);
			break;
	}
}

void HLine_32Bit(graphics_card_info i, uint16 x, uint16 y, uint16 length, rgb_color col)
{
#ifdef DEBUG_DRIVER
printf("HLine() Unfinished\n");
#endif
	uint32 c=0,*pcolor,bytes;
	
	// apparently width=bytes per row
	uint32 bpr=i.width;
	
	// ARGB order
	c|=col.alpha * 0x01000000;
	c|=col.red * 0x010000;
	c|=col.green * 0x0100;
	c|=col.blue;

#ifdef DEBUG_DRIVER
printf("HLine color value: 0x%lx\n",c);
#endif
	bytes=(uint32(x+length)>bpr)?length-((x+length)-bpr):length;

	pcolor=(uint32*)i.frame_buffer;
	pcolor+=(y*bpr)+(x*4);
	for(int32 i=0;i<length;i++)
		pcolor[i]=c;
}

void HLine_16Bit(graphics_card_info i, uint16 x, uint16 y, uint16 length, uint16 col)
{
#ifdef DEBUG_DRIVER
printf("HLine() Unimplemented\n");
#endif
}

void HLine_8Bit(graphics_card_info i, uint16 x, uint16 y, uint16 length, uint8 col)
{
#ifdef DEBUG_DRIVER
printf("HLine(%u,%u,length %u,%u)\n",x,y,length,col);
#endif
	size_t bytes;
	uint32 bpr=i.width;
	if( (bpr%2)!=0 )
		bpr++;

	uint8 *pcolor=(uint8*)i.frame_buffer;
	
	pcolor+=(y*bpr)+x;

	bytes=(uint32(x+length)>bpr)?length-((x+length)-bpr):length;

	memset(pcolor,col,bytes);
}

void ScreenDriver::DrawServerBitmap32(ServerBitmap *sourcebmp,BPoint pt, int32 blendmode)
{
	if(!fbuffer->IsConnected())
		return;
	BRect sourcerect=sourcebmp->Bounds(),
		destbounds(0,0,(fbuffer->gcinfo.width/fbuffer->gcinfo.bytes_per_row)-1,
			(fbuffer->gcinfo.height/fbuffer->gcinfo.bytes_per_row)-1),
		destrect=sourcerect.OffsetByCopy(pt);
	
	int16 	red,green,blue,alpha,
			sred,sgreen,sblue,salpha,
			tempval=0,trans;
	float tfrac;

	// data vars for brush and for bitmap
	uint8 *src_data=sourcebmp->Buffer(),
			*dest_data=(uint8*)fbuffer->gcinfo.frame_buffer,
			*src_pos,*dest_pos;
	int8 	*src_pos_signed,*dest_pos_signed;
	uint32 	src_rowsize=sourcebmp->BytesPerRow(),
			dest_rowsize=fbuffer->gcinfo.bytes_per_row;

	uint32 rows,columns,
			lclip,rclip,tclip,bclip;	// integer vars for clipping rectangle

	// Clip source and dest rectangles to respective bitmaps
	if(!(sourcebmp->Bounds().Contains(sourcerect)))
	{
		sourcerect.LeftTop().ConstrainTo(sourcebmp->Bounds());
		sourcerect.RightBottom().ConstrainTo(sourcebmp->Bounds());
	}
	if(!(destbounds.Contains(destrect)))
	{
		destrect.LeftTop().ConstrainTo(destbounds);
		destrect.RightBottom().ConstrainTo(destbounds);
	}
	
	// Clip source to dest rectangle
	if(!(destbounds.Contains(sourcerect)))
	{
		sourcerect.LeftTop().ConstrainTo(destbounds);
		sourcerect.RightBottom().ConstrainTo(destbounds);
	}

	// Assign source bitmap clipping rectangle proper values
	lclip=(int32)sourcerect.left;
	tclip=(int32)sourcerect.top;
	rclip=(int32)sourcerect.right;
	bclip=(int32)sourcerect.bottom;

	// Jump to area in question in image
	src_data += int32((sourcerect.top*src_rowsize) + (sourcerect.left *4));
	dest_data += int32((destrect.top*dest_rowsize) + (destrect.left *4));

	for(rows=tclip;rows<=bclip;rows++)
	{	
		dest_pos=(uint8*)(dest_data+(dest_rowsize * (rows-tclip)));
		src_pos=(uint8*)(src_data+(src_rowsize * (rows-tclip)));
		dest_pos_signed=(int8 *)dest_pos;
		src_pos_signed=(int8 *)src_pos;
		for(columns=lclip;columns<=rclip;columns++)
		{
			sred=src_pos[2];
			sgreen=src_pos[1];
			sblue=src_pos[0];
			salpha=src_pos[3];

			if(salpha==0)
			{
				dest_pos+=4;
				src_pos+=4;
				continue;
			}

			switch(blendmode)
			{
				case DRAW_MAX:
				{	
					red=MAX(dest_pos[2],sred);
					green=MAX(dest_pos[1],sgreen);
					blue=MAX(dest_pos[0],sblue);
					break;
				}
				case DRAW_MIN:
				{	
					red=MIN(dest_pos[2],sred);
					green=MIN(dest_pos[1],sgreen);
					blue=MIN(dest_pos[0],sblue);
					break;
				}
				case DRAW_BLEND:
				{	
					red=(sred+dest_pos[2])/2;
					green=(sgreen|dest_pos[1])/2;
					blue=(sblue|dest_pos[0]);
					break;
				}
				case DRAW_ADD:
				{	
					red=dest_pos[2]+sred;
					green=dest_pos[1]+sgreen;
					blue=dest_pos[0]+sblue;
					if(red>255) red=255;
					if(green>255) green=255;
					if(blue>255) blue=255;
					break;
				}
				case DRAW_SUBTRACT:
				{	
					red=dest_pos[2]-sred;
					green=dest_pos[1]-sgreen;
					blue=dest_pos[0]-sblue;
					if(red<0) red=0;
					if(green<0) green=0;
					if(blue<0) blue=0;
					break;
				}
				case DRAW_INVERT:
				{
					red=dest_pos[2]^255;
					green=dest_pos[1]^255;
					blue=dest_pos[0]^255;
					break;
				}
				default:
				{
					blue=sblue;
					green=sgreen;
					red=sred;
				}
			}
			
			trans=salpha;
			tfrac=float(salpha)/255.0;

			tempval+=(blue>dest_pos[0])?int16(float(blue) * tfrac):int16(float(blue) * tfrac * -1);
			if(tempval>255)
				tempval=255;
			dest_pos[0] = blue;

			tempval+=(green>dest_pos[1])?int16(float(green) * tfrac):int16(float(green) * tfrac * -1);
			if(tempval>255)
				tempval=255;
			dest_pos[1] = green;

			tempval+=(red>dest_pos[2])?int16(float(red) * tfrac):int16(float(red) * tfrac * -1);
			if(tempval>255)
				tempval=255;
			dest_pos[2] = red;

			alpha=dest_pos[3]-salpha;
			alpha=dest_pos_signed[3]-salpha;
			alpha=dest_pos[3]+int16(float(dest_pos[3])*tfrac);
			tempval=dest_pos[3] + int16(float(alpha) * tfrac);
			if(tempval>255)
				tempval=255;
			if(blendmode==DRAW_COPY)
				tempval=salpha;
			dest_pos[3]=tempval;

			dest_pos+=4;
			src_pos+=4;
		}	// end for columns
	}	// end for rows
}