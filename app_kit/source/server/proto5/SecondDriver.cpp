#include <stdio.h>
#include <iostream.h>
#include <Message.h>
#include <Bitmap.h>
#include <OS.h>

#include "PortLink.h"
#include "ServerProtocol.h"
#include "ServerBitmap.h"
#include "SecondDriver.h"
#include "ServerCursor.h"
#include "DebugTools.h"

#include <Accelerant.h>
#include <GraphicsCard.h>
#include <image.h>
#include <FindDirectory.h>
#include <graphic_driver.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <malloc.h>
#include <errno.h>
#include <stdlib.h>


int find_device(){
   DIR *d;
   struct dirent *e;
   char name_buf[1024];
   int fd = -1;
   bool foundfirst = false; // hacky way of finding second video card in your box
      
   /* open directory apath */
   d = opendir("/dev/graphics");
   if(!d){ return B_ERROR; }
   while((e = readdir(d)) != NULL){
      if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..") || !strcmp(e->d_name, "stub")){
         continue;
      }
      strcpy(name_buf, "/dev/graphics");
      strcat(name_buf, "/");
      strcat(name_buf, e->d_name);
      fd = open(name_buf, B_READ_WRITE);
      if(fd >= 0){ 
         if(!foundfirst){
            foundfirst = true;
            printf("\tbut we are not going to use it as app_server is\n");
         }else{
            // found it :)
            closedir(d);
            return fd;
         }
      }
   }
   closedir(d);
   return B_ERROR;
}
image_id load_accelerant(int fd, GetAccelerantHook *hook) {
	status_t result;
	image_id image = -1;
	char
		signature[1024],
		path[PATH_MAX];
	struct stat st;
	const static directory_which vols[] = {
		B_USER_ADDONS_DIRECTORY,
		B_COMMON_ADDONS_DIRECTORY,
		B_BEOS_ADDONS_DIRECTORY
	};

	/* get signature from driver */
	result = ioctl(fd, B_GET_ACCELERANT_SIGNATURE, &signature, sizeof(signature));
	if (result != B_OK) goto done;
	printf("B_GET_ACCELERANT_SIGNATURE returned ->%s<-\n", signature);

	// note failure by default
	for(int32 i=0; i < (int32)(sizeof (vols) / sizeof (vols[0])); i++) {

		/* ---
			compute directory path to common or beos addon directory on
			floppy or boot volume
		--- */

		printf("attempting to get path for %ld (%d)\n", i, vols[i]);
		if (find_directory (vols[i], -1, false, path, PATH_MAX) != B_OK) {
			printf("find directory failed\n");
			continue;
		}

		strcat (path, "/accelerants/");
		strcat (path, signature);

		printf("about to stat(%s)\n", path);
		// don't try to load non-existant files
		if (stat(path, &st) != 0) continue;
		printf("Trying to load accelerant: %s\n", path);
		// load the image
		image = load_add_on(path);
		if (image >= 0) {
			printf("Accelerant loaded!\n");
			// get entrypoint from accelerant
			result = get_image_symbol(image, B_ACCELERANT_ENTRY_POINT,
#if defined(__INTEL__)
				B_SYMBOL_TYPE_ANY,
#else
				B_SYMBOL_TYPE_TEXT,
#endif
				(void **)hook);
			if (result == B_OK) {
				init_accelerant ia;
				printf("Entry point %s() found\n", B_ACCELERANT_ENTRY_POINT);
				ia = (init_accelerant)(*hook)(B_INIT_ACCELERANT, NULL);
				printf("init_accelerant is 0x%08lx\n", (uint32)ia);
				if (ia && ((result = ia(fd)) == B_OK)) {
					// we have a winner!
					printf("Accelerant %s accepts the job!\n", path);
					break;
				} else {
					printf("init_accelerant refuses the the driver: %ld\n", result);
				}
			} else {
				printf("Couldn't find the entry point :-(\n");
			}
			// unload the accelerant, as we must be able to init!
			unload_add_on(image);
		}
		if (image < 0) printf("image failed to load with reason %.8lx (%s)\n", image, strerror(image));
		// mark failure to load image
		image = -1;
	}

	printf("Add-on image id: %ld\n", image);

done:
	return image;
}

static const char *spaceToString(uint32 cs) {
	const char *s;
	switch (cs) {
#define s2s(a) case a: s = #a ; break
		s2s(B_RGB32);
		s2s(B_RGBA32);
		s2s(B_RGB32_BIG);
		s2s(B_RGBA32_BIG);
		s2s(B_RGB16);
		s2s(B_RGB16_BIG);
		s2s(B_RGB15);
		s2s(B_RGBA15);
		s2s(B_RGB15_BIG);
		s2s(B_RGBA15_BIG);
		s2s(B_CMAP8);
		s2s(B_GRAY8);
		s2s(B_GRAY1);
		s2s(B_YCbCr422);
		s2s(B_YCbCr420);
		s2s(B_YUV422);
		s2s(B_YUV411);
		s2s(B_YUV9);
		s2s(B_YUV12);
		default:
			s = "unknown"; break;
#undef s2s
	}
	return s;
}

void dump_mode(display_mode *dm) {
	display_timing *t = &(dm->timing);
	printf("  pixel_clock: %ldKHz\n", t->pixel_clock);
	printf("            H: %4d %4d %4d %4d\n", t->h_display, t->h_sync_start, t->h_sync_end, t->h_total);
	printf("            V: %4d %4d %4d %4d\n", t->v_display, t->v_sync_start, t->v_sync_end, t->v_total);
	printf(" timing flags:");
	if (t->flags & B_BLANK_PEDESTAL) printf(" B_BLANK_PEDESTAL");
	if (t->flags & B_TIMING_INTERLACED) printf(" B_TIMING_INTERLACED");
	if (t->flags & B_POSITIVE_HSYNC) printf(" B_POSITIVE_HSYNC");
	if (t->flags & B_POSITIVE_VSYNC) printf(" B_POSITIVE_VSYNC");
	if (t->flags & B_SYNC_ON_GREEN) printf(" B_SYNC_ON_GREEN");
	if (!t->flags) printf(" (none)\n");
	else printf("\n");
	printf(" refresh rate: %4.2f\n", ((double)t->pixel_clock * 1000) / ((double)t->h_total * (double)t->v_total));
	printf("  color space: %s\n", spaceToString(dm->space));
	printf(" virtual size: %dx%d\n", dm->virtual_width, dm->virtual_height);
	printf("dispaly start: %d,%d\n", dm->h_display_start, dm->v_display_start);

	printf("   mode flags:");
	if (dm->flags & B_SCROLL) printf(" B_SCROLL");
	if (dm->flags & B_8_BIT_DAC) printf(" B_8_BIT_DAC");
	if (dm->flags & B_HARDWARE_CURSOR) printf(" B_HARDWARE_CURSOR");
	if (dm->flags & B_PARALLEL_ACCESS) printf(" B_PARALLEL_ACCESS");
//	if (dm->flags & B_SUPPORTS_OVERLAYS) printf(" B_SUPPORTS_OVERLAYS");
	if (!dm->flags) printf(" (none)\n");
	else printf("\n");
}

status_t get_and_set_mode(GetAccelerantHook gah, display_mode *dm) {

	accelerant_mode_count gmc;
	uint32 mode_count;
	get_mode_list gml;
	display_mode *mode_list, target, high, low;
	propose_display_mode pdm;
	status_t result = B_ERROR;
	set_display_mode sdm;

	/* find the propose mode hook */
	pdm = (propose_display_mode)gah(B_PROPOSE_DISPLAY_MODE, NULL);
	if (!pdm) {
		printf("No B_PROPOSE_DISPLAY_MODE\n");
		goto exit0;
	}
	/* and the set mode hook */
	sdm = (set_display_mode)gah(B_SET_DISPLAY_MODE, NULL);
	if (!sdm) {
		printf("No B_SET_DISPLAY_MODE\n");
		goto exit0;
	}

	/* how many modes does the driver support */
	gmc = (accelerant_mode_count)gah(B_ACCELERANT_MODE_COUNT, NULL);
	if (!gmc) {
		printf("No B_ACCELERANT_MODE_COUNT\n");
		goto exit0;
	}
	mode_count = gmc();
	printf("mode_count = %lu\n", mode_count);
	if (mode_count == 0) goto exit0;

	/* get a list of graphics modes from the driver */
	gml = (get_mode_list)gah(B_GET_MODE_LIST, NULL);
	if (!gml) {
		printf("No B_GET_MODE_LIST\n");
		goto exit0;
	}
	mode_list = (display_mode *)calloc(sizeof(display_mode), mode_count);
	if (!mode_list) {
		printf("Couldn't calloc() for mode list\n");
		goto exit0;
	}
	if (gml(mode_list) != B_OK) {
		printf("mode list retrieval failed\n");
		goto free_mode_list;
	}

	/* take the first mode in the list */
	//dump_mode(&mode_list[69]);
	target = high = low = mode_list[69];
	/* make as tall a virtual height as possible */
	target.virtual_height = high.virtual_height = 0xffff;
	/* propose the display mode */
	if (pdm(&target, &low, &high) == B_ERROR) {
		printf("propose_display_mode failed\n");
		goto free_mode_list;
	}
	printf("Target display mode: \n");
	dump_mode(&target);
	/* we got a display mode, now set it */
	if (sdm(&target) == B_ERROR) {
		printf("set display mode failed\n");
		goto free_mode_list;
	}
	/* note the mode and success */
	*dm = target;
	result = B_OK;
	
free_mode_list:
	free(mode_list);
exit0:
	return result;
}


void get_frame_buffer(GetAccelerantHook gah, frame_buffer_config *fbc) {
	get_frame_buffer_config gfbc;
	gfbc = (get_frame_buffer_config)gah(B_GET_FRAME_BUFFER_CONFIG, NULL);
	gfbc(fbc);
}

sem_id get_sem(GetAccelerantHook gah) {
	accelerant_retrace_semaphore ars;
	ars = (accelerant_retrace_semaphore)gah(B_ACCELERANT_RETRACE_SEMAPHORE, NULL);
	return ars();
}

void set_palette(GetAccelerantHook gah) {
	set_indexed_colors sic;
	sic = (set_indexed_colors)gah(B_SET_INDEXED_COLORS, NULL);
	if (sic) {
		/* booring grey ramp for now */
		uint8 map[3 * 256];
		uint8 *p = map;
		int i;
		for (i = 0; i < 256; i++) {
			*p++ = i;
			*p++ = i;
			*p++ = i;
		}
		sic(256, 0, map, 0);
	}
}

SecondDriver::SecondDriver(void):DisplayDriver(){
}

SecondDriver::~SecondDriver(void){
   if(is_initialized){
   }
}

void SecondDriver::Initialize(void){
   et = NULL;
   bits = NULL;
   
   // Find a driver
   fd =  find_device();
   if(fd < 0){
      is_initialized = false;
   }
   /* load the accelerant */
   image = load_accelerant(fd, &gah);
   if (image >= 0){
      // Set up the display to a valid resolution
      if(get_and_set_mode(gah,&dm) == B_OK){
         // Here we are all good
         if(dm.space == B_CMAP8){
            set_palette(gah);
         }
         get_frame_buffer(gah, &fbc);
         bits = (uint32*)fbc.frame_buffer/*_dma*/;
         bpr = fbc.bytes_per_row/4; // sould be by depth
         
         // Lets chech out tha hardwar cursoer stuff :P
         scs = (set_cursor_shape)gah(B_SET_CURSOR_SHAPE,NULL);
         mc = (move_cursor)gah(B_MOVE_CURSOR,NULL);
         sc = (show_cursor)gah(B_SHOW_CURSOR,NULL);
         
         // Ok. now lets see if we can add some spiffy
         // 2D acceleration to this :)
         accelerant_engine_count aec = (accelerant_engine_count)gah(B_ACCELERANT_ENGINE_COUNT,NULL);
         uint32 count = aec();
         
         if(count > 0){
            acquire_engine ae = (acquire_engine)gah(B_ACQUIRE_ENGINE,NULL);
            re = (release_engine)gah(B_RELEASE_ENGINE,NULL);
            if(ae(B_2D_ACCELERATION,1000,&st,&et) == B_OK){
               // Now lets see if we have some functions
               s2sb = (screen_to_screen_blit)gah(B_SCREEN_TO_SCREEN_BLIT,NULL);
               fr = (fill_rectangle)gah(B_FILL_RECTANGLE,NULL);
               ir = (invert_rectangle)gah(B_INVERT_RECTANGLE,NULL);
               s2stb = (screen_to_screen_transparent_blit)gah(B_SCREEN_TO_SCREEN_TRANSPARENT_BLIT,NULL);
               fs = (fill_span)gah(B_FILL_SPAN,NULL);
            }
         }
         
         is_initialized = true;
      }else{
         // failed to set mode
         is_initialized = false;
      }
   }else{
      // image failed to load
      is_initialized = false;
   }
}

void SecondDriver::Shutdown(void){
   if(et){
      if(re(et,&st) != B_OK){
         printf("Failed to release engine\n");
      }
   }
   
   uninit_accelerant ua = (uninit_accelerant)gah(B_UNINIT_ACCELERANT,NULL);
   if(ua){ ua();}
   
   unload_add_on(image);
   
   // clsoe the file now...
   close(fd);
   
   
   is_initialized=false;
}

bool SecondDriver::IsInitialized(void){
   return is_initialized;
}

void SecondDriver::SafeMode(void){
}

void SecondDriver::Reset(void){
}

void SecondDriver::SetScreen(uint32 space){
  // Clear(51,102,152);
}

void SecondDriver::Clear(uint8 red, uint8 green, uint8 blue){
   rgb_color r;
   r.red = red;
   r.blue = blue;
   r.green = green;   
   Clear(r);
}

void SecondDriver::Clear(rgb_color col){
   highcol = col;
   FillRect(BRect(0,0,GetHeight()-1,GetWidth()-1),NULL);
   printf("calling clear\n");
}

int32 SecondDriver::GetHeight(void){
   // Gets the height of the current mode
   return dm.virtual_height;
}

int32 SecondDriver::GetWidth(void){
   // Gets the width of the current mode
   return dm.virtual_width;
}

int SecondDriver::GetDepth(void){
   // Gets the color depth of the current mode
   return 0;
}

void SecondDriver::Blit(BRect src, BRect dest){
}

void SecondDriver::DrawBitmap(ServerBitmap *bitmap){
}

void SecondDriver::DrawChar(char c, BPoint point){
	char string[2];
	string[0]=c;
	string[1]='\0';
	DrawString(string,2,point);
}

void SecondDriver::DrawString(char *string, int length, BPoint point){
}

void SecondDriver::FillArc(int centerx, int centery, int xradius, int yradius, float angle, float span, uint8 *pattern){
}

void SecondDriver::FillBezier(BPoint *points, uint8 *pattern){
}

void SecondDriver::FillEllipse(float centerx, float centery, float x_radius, float y_radius,uint8 *pattern){
}

void SecondDriver::FillPolygon(int *x, int *y, int numpoints, bool is_closed){
}

void SecondDriver::FillRect(BRect rect, uint8 *pattern){
   if(fr){
      // usieng accelertated 2d file
      union{
         uint8 bytes[4];
         uint32 word;
      }c1;

      printf("Accelerated Fill\n");
      fill_rect_params frp;
      frp.left =(uint16) rect.left;
      frp.top =(uint16) rect.top;
      frp.right =(uint16) rect.right;
      frp.bottom =(uint16) rect.bottom;
      c1.bytes[0] = highcol.blue;
      c1.bytes[1] = highcol.green;
      c1.bytes[2] = highcol.red;
      fr(et,c1.word,&frp,1);
   }else{
      for(int32 i = 0;i < rect.Height()-1;i++){
         MovePenTo(BPoint(rect.left,rect.top+i));
         StrokeLine(BPoint(rect.right,rect.top+i),pattern);
      }
   }
}

void SecondDriver::FillRect(BRect rect, rgb_color col)
{
}

void SecondDriver::FillRegion(BRegion *region){
}

void SecondDriver::FillRoundRect(BRect rect,float xradius, float yradius, uint8 *pattern){
}

void SecondDriver::FillShape(BShape *shape){
}

void SecondDriver::FillTriangle(BPoint first, BPoint second, BPoint third, BRect rect, uint8 *pattern){
}

void SecondDriver::HideCursor(void){
   if(sc){
      sc(false);
      cursor_visible = false;
   }else{
      printf("Shoftware Hide Cursor\n");
   }
}

bool SecondDriver::IsCursorHidden(void){
   return cursor_visible;
}

void SecondDriver::ObscureCursor(void){
   // Hides cursor until mouse is moved
   HideCursor();
   show_on_move = true;
}

void SecondDriver::MoveCursorTo(float x, float y){
//   printf("Callingmove cursoer to %f %f\n",x,y);
   if(show_on_move){
      ShowCursor();
   }
   if(mc){
      mc((short unsigned int)x,(short unsigned int)y);
   }else{
      printf("Software move Cursor\n");
   }   
}

void SecondDriver::MovePenTo(BPoint pt){
   penpos = pt;
}

BPoint SecondDriver::PenPosition(void){
   return penpos;
}

float SecondDriver::PenSize(void){
   return pensize;
}

void SecondDriver::SetCursor(int32 value){
   // Uh what is this all about?
}

 uint8 andMask[] = { // cursor
   0x00, 0x00, 0x00, 0x0e , 0x00, 0x1e, 0x00, 0x3e,
   0x00, 0x5c, 0x00, 0x88 , 0x01, 0x10, 0x02, 0x20, 
   0x04, 0x40, 0x08, 0x80, 0x11, 0x00, 0x22, 0x00, 
   0x64, 0x00, 0x78, 0x00, 0xf0, 0x00, 0xc0, 0x00
   };
 uint8 xorMask[] = { // mask
   0x00, 0x00, 0x00, 0x0e, 0x00, 0x12, 0x00, 0x32,
   0x00, 0x7c, 0x00, 0xf8, 0x01, 0xf0, 0x03, 0xe0,
   0x07, 0xc0, 0x0f, 0x80, 0x1f, 0x00, 0x3e, 0x00,
   0x7c, 0x00, 0x78, 0x00, 0xf0, 0x00, 0xc0, 0x00
};

void SecondDriver::SetCursor(ServerCursor *cursor){
   printf("Setting cursor\n");
   current_cursor = cursor;
//   uint8 *andMask;
 //  uint8 *xorMask;
   if(scs){
      scs(cursor->width,cursor->height,(short unsigned int)cursor->hotspot.x,(short unsigned int)cursor->hotspot.y,andMask,xorMask);
   }else{
      printf("Setting software cursor\n");
   }
}

void SecondDriver::SetPenSize(float size){
   pensize = size;
}

void SecondDriver::SetHighColor(uint8 r,uint8 g,uint8 b,uint8 a=255){
   highcol.red = r;
   highcol.green = g;
   highcol.blue = b;
   highcol.alpha = a;   
}

void SecondDriver::SetLowColor(uint8 r,uint8 g,uint8 b,uint8 a=255){
   lowcol.red = r;
   lowcol.green = g;
   lowcol.blue = b;
   lowcol.alpha = a;
}

void SecondDriver::SetPixel(int x, int y, uint8 *pattern){
   union{
      uint8 bytes[4];
      uint32 word;
   }c1;
   
   c1.bytes[0] = highcol.blue; 
   c1.bytes[1] = highcol.green; 
   c1.bytes[2] = highcol.red; 
   c1.bytes[3] = highcol.alpha; 
 
   *(bits + x + y*bpr) = c1.word;
}

void SecondDriver::ShowCursor(void){
   show_on_move = false;
   cursor_visible = true;
   if(sc){
      sc(true);
   }else{
      printf("Software show cursor\n");
   }
}

void SecondDriver::StrokeArc(int centerx, int centery, int xradius, int yradius, float angle, float span, uint8 *pattern){
}

void SecondDriver::StrokeBezier(BPoint *points, uint8 *pattern){
}

void SecondDriver::StrokeEllipse(float centerx, float centery, float x_radius, float y_radius,uint8 *pattern){
}

void SecondDriver::StrokeLine(BPoint point, uint8 *pattern){

   int oct = 0;
   int xoff = (int32)penpos.x;
   int yoff = (int32)penpos.y; 
   int32 x2 = (int32)point.x-xoff;
   int32 y2 = (int32)point.y-yoff; 
   int32 x1 = 0;
   int32 y1 = 0;
   //if(y2==0){ if (x1>x2){int t=x1;x1=x2;x2=t;}horizLine(x1+xoff,x2+xoff,y1+yoff); return;}
   if(y2<0){ y2 = -y2; oct+=4; }//bit2=1
   if(x2<0){ x2 = -x2; oct+=2;}//bit1=1
   if(x2<y2){ int t=x2; x2=y2; y2=t; oct+=1;}//bit0=1
   int x=x1,
       y=y1,
       sum=x2-x1,
       Dx=2*(x2-x1),
       Dy=2*(y2-y1);
   for(int i=0; i <= x2-x1; i++){ 
      switch(oct){
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
      if(sum < 0){
         y++;
         sum += Dx;
      }
   }

   MovePenTo(point); // ends up in last position
}

void SecondDriver::StrokePolygon(int *x, int *y, int numpoints, bool is_closed){
   BPoint first(x[0],y[0]);
   MovePenTo(first);
   for(int32 i = 1; i < numpoints; i++){
      StrokeLine(BPoint(x[i],y[i]),NULL);
   }
   if(is_closed){
      //StrokeLine();
   }
}

void SecondDriver::StrokeRect(BRect rect,uint8 *pattern){
   MovePenTo(BPoint(rect.left,rect.top));
   StrokeLine(BPoint(rect.right,rect.top),pattern);
   StrokeLine(BPoint(rect.right,rect.bottom),pattern);
   StrokeLine(BPoint(rect.left,rect.bottom),pattern);
   StrokeLine(BPoint(rect.left,rect.top),pattern);
   printf("Strinking rect\n");
}

void SecondDriver::StrokeRect(BRect rect,rgb_color col)
{
}

void SecondDriver::StrokeRoundRect(BRect rect,float xradius, float yradius, uint8 *pattern){
}

void SecondDriver::StrokeShape(BShape *shape){
}

void SecondDriver::StrokeTriangle(BPoint first, BPoint second, BPoint third, BRect rect, uint8 *pattern){
   MovePenTo(first);
   StrokeLine(second,pattern);
   StrokeLine(third,pattern);
   StrokeLine(first,pattern);
}

#ifdef DEBUG_DRIVER_MODULE
#undef DEBUG_DRIVER_MODULE
#endif