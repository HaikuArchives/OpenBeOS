/*

	PDFWriter.cpp

Copyright (c) 2001, 2002 OpenBeOS. 

Authors: 
	Philippe Houdoin
	Simon Gauvin	
	Michael Pfeiffer
	
Based on:
 - gdevbjc.c from Aladdin GhostScript
 - LMDriver.cpp from Ryan Lockhart Lexmark 5/7xxxx BeOS driver 

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include <stdio.h>
#include <string.h>			// for memset()
#include <math.h>

#include <Debug.h>
#include <StorageKit.h>
#include <TranslationKit.h>
#include <support/UTF8.h>

#include "PDFWriter.h"
#include "Bezier.h"
#include "pdflib.h"

// Constructor & destructor
// ------------------------

PDFWriter::PDFWriter()
	:	PrinterDriver()
{
	fEmbedMaxFontSize = 250 * 1024;
	fScreen = new BScreen();
	fFonts = NULL;
}


// --------------------------------------------------
PDFWriter::~PDFWriter()
{
	const int n = fFontCache.CountItems();
	for (int i = 0; i < n; i++) {
		delete (Font*)fFontCache.ItemAt(i);
	}
	fFontCache.MakeEmpty();
	
	for (int i = 0; i < fPatterns.CountItems(); i++) {
		delete (Pattern*)fPatterns.ItemAt(i);
	}
	fPatterns.MakeEmpty();
	
	if (Transport())
		CloseTransport();

	delete fScreen;
	delete fFonts;
}


#ifdef CODEWARRIOR
	#pragma mark [Public methods]
#endif

// Public methods
// --------------

// --------------------------------------------------
status_t 
PDFWriter::PrintPage(int32	pageNumber, int32 pageCount)
{
	status_t	status;
	BRect		r;
	uint32 		pictureCount;
	BRect		paperRect;
	BRect 		printRect;
	BRect		*picRects;
	BPoint		*picPoints;
	BRegion		*picRegion;
	BPicture	**pictures;
	uint32		i;
	int32       orientation;
	
	status = B_OK;

	if (pageNumber == 1) {
		if (MakesPattern()) 
			fprintf(fLog, ">>>>> Collecting patterns...\n");
		else if (MakesPDF())
			fprintf(fLog, ">>>>> Generating PDF...\n");
	}
	
	paperRect = JobMsg()->FindRect("paper_rect");
	printRect = JobMsg()->FindRect("printable_rect");
	if (B_OK != JobMsg()->FindInt32("orientation", &orientation)) orientation = 0;
	if (orientation == 1) 
		printRect.Set(printRect.top, printRect.left,
			printRect.bottom, printRect.right);

	JobFile()->Read(&pictureCount, sizeof(uint32));

	pictures = (BPicture **) 	malloc(pictureCount * sizeof(BPicture *));
	picRects = (BRect *)		malloc(pictureCount * sizeof(BRect));
	picPoints = (BPoint *)		malloc(pictureCount * sizeof(BPoint));
	picRegion = new BRegion();
	
	for (i = 0; i < pictureCount; i++) {
		JobFile()->Seek(40 + sizeof(off_t), SEEK_CUR);
		JobFile()->Read(&picPoints[i], sizeof(BPoint));
		JobFile()->Read(&picRects[i], sizeof(BRect));
		pictures[i] = new BPicture();
		pictures[i]->Unflatten(JobFile());
		picRegion->Include(picRects[i]);
	}
	
	r  = picRegion->Frame();
	delete picRegion;

	BeginPage(paperRect, printRect);
	for (i = 0; i < pictureCount; i++) {
		Iterate(pictures[i]);
		delete pictures[i];
	}
	EndPage();
	
	free(pictures);
	free(picRects);
	free(picPoints);
	
	return status;
}


// --------------------------------------------------
status_t 
PDFWriter::BeginJob() 
{
	fLog = fopen("/tmp/pdf_writer.log", "w");

	PDF_boot();

	fPdf = PDF_new2(_ErrorHandler, NULL, NULL, NULL, this);	// set *this* as pdf cookie
	if ( fPdf == NULL )
		return B_ERROR;

	fFonts = new Fonts();
	fFonts->CollectFonts();
	BMessage fonts;
	if (B_OK == JobMsg()->FindMessage("fonts", &fonts)) {
		fFonts->SetTo(&fonts);
	}
		
	return InitWriter();
}


// --------------------------------------------------
status_t 
PDFWriter::EndJob() 
{
	PDF_close(fPdf);
	fprintf(fLog, ">>>> PDF_close\n");

   	PDF_delete(fPdf);
    PDF_shutdown();

	fclose(fLog);
	return B_OK;
}


// --------------------------------------------------
status_t 
PDFWriter::InitWriter()
{
	char	buffer[512];
	BString s;

	const char * compatibility;
	if (JobMsg()->FindString("pdf_compatibility", &compatibility) == B_OK) {
		PDF_set_parameter(fPdf, "compatibility", compatibility);
	}

	fprintf(fLog, ">>>> PDF_open_mem\n");	
	PDF_open_mem(fPdf, _WriteData);	// use callback to stream PDF document data to printer transport

	PDF_set_parameter(fPdf, "flush", "heavy");

	// set document info
	BMessage doc;
	bool setTitle = true;
	bool setCreator = true;
	if (JobMsg()->FindMessage("doc_info", &doc) == B_OK) {
		char *name;
		uint32 type;
		int32 count;
		for (int32 i = 0; doc.GetInfo(B_STRING_TYPE, i, &name, &type, &count) != B_BAD_INDEX; i++) {
			if (type == B_STRING_TYPE) {
				BString value;
				if (doc.FindString(name, &value) == B_OK && value != "") {
					BString s;
					ToPDFUnicode(value.String(), s);
					PDF_set_info(fPdf, name, s.String());
				}
			}
		}
		BString s;
		if (doc.FindString("Title", &s) == B_OK && s != "") setTitle = false;
		if (doc.FindString("Creator", &s) == B_OK && s != "") setCreator = false;
	}
		
	// find job title 
	if (setTitle && JobFile()->ReadAttr("_spool/Description", B_STRING_TYPE, 0, buffer, sizeof(buffer))) {
	    ToPDFUnicode(buffer, s); PDF_set_info(fPdf, "Title", s.String());
	}
				
	// find job creator
	if (setCreator && JobFile()->ReadAttr("_spool/MimeType", B_STRING_TYPE, 0, buffer, sizeof(buffer))) {
	    ToPDFUnicode(buffer, s); PDF_set_info(fPdf, "Creator", s.String());
	}
	
	int32 compression;
	if (JobMsg()->FindInt32("pdf_compression", &compression) == B_OK) {
	    PDF_set_value(fPdf, "compress", compression);
	}

    // PDF_set_parameter(fPdf, "warning", "false");

	PDF_set_parameter(fPdf, "fontwarning", "false");
	// PDF_set_parameter(fPdf, "native-unicode", "true");

	fprintf(fLog, "Start of fonts declaration:\n");	

	PDF_set_parameter(fPdf, "Encoding", "t1enc0==/boot/home/config/settings/PDF Writer/t1enc0.enc");
	PDF_set_parameter(fPdf, "Encoding", "t1enc1==/boot/home/config/settings/PDF Writer/t1enc1.enc");
	PDF_set_parameter(fPdf, "Encoding", "t1enc2==/boot/home/config/settings/PDF Writer/t1enc2.enc");
	PDF_set_parameter(fPdf, "Encoding", "t1enc3==/boot/home/config/settings/PDF Writer/t1enc3.enc");
	PDF_set_parameter(fPdf, "Encoding", "t1enc4==/boot/home/config/settings/PDF Writer/t1enc4.enc");

	PDF_set_parameter(fPdf, "Encoding", "ttenc0==/boot/home/config/settings/PDF Writer/ttenc0.cpg");
	PDF_set_parameter(fPdf, "Encoding", "ttenc1==/boot/home/config/settings/PDF Writer/ttenc1.cpg");
	PDF_set_parameter(fPdf, "Encoding", "ttenc2==/boot/home/config/settings/PDF Writer/ttenc2.cpg");
	PDF_set_parameter(fPdf, "Encoding", "ttenc3==/boot/home/config/settings/PDF Writer/ttenc3.cpg");
	PDF_set_parameter(fPdf, "Encoding", "ttenc4==/boot/home/config/settings/PDF Writer/ttenc4.cpg");

	DeclareFonts();

	fprintf(fLog, "End of fonts declaration.\n");	

	fState = NULL;
	fStateDepth = 0;

	return B_OK;
}


// --------------------------------------------------
status_t
PDFWriter::DeclareFonts()
{
	char buffer[1024];
	char *parameter_name;

	for (int i = 0; i < fFonts->Length(); i++) {
		FontFile* f = fFonts->At(i);
//		fprintf(fLog, "path= %s\n", f->Path());		
		if (f->Type() == true_type_type) {
			parameter_name = "FontOutline";
		} else { // f->Type() == type1_type
			if (strstr(f->Path(), ".afm"))
				parameter_name = "FontAFM";
			else if (strstr(f->Path(), ".pfm"))
				parameter_name = "FontPFM";
			else // should not reach here! 
				continue;		
		}
#ifndef __POWERPC__						
		snprintf(buffer, sizeof(buffer), "%s==%s", f->Name(), f->Path());
#else
		sprintf(buffer, "%s==%s", f->Name(), f->Path());
#endif
//		fprintf(fLog, "%s: %s\n", parameter_name, buffer);
	
		PDF_set_parameter(fPdf, parameter_name, buffer);
	}
	return B_OK;
}


// --------------------------------------------------
status_t 
PDFWriter::BeginPage(BRect paperRect, BRect printRect)
{
	float width = paperRect.Width() < 10 ? a4_width : paperRect.Width();
	float height = paperRect.Height() < 10 ? a4_height : paperRect.Height();
	
	fMode = kDrawingMode;
	
	ASSERT(fState == NULL);
	fState = new State();
	fState->height = height;
    if (MakesPDF())
    	PDF_begin_page(fPdf, width, fState->height);

	fprintf(fLog, ">>>> PDF_begin_page [%f, %f]\n", width, fState->height);
	
	if (MakesPDF())
		PDF_initgraphics(fPdf);
	
	fState->fontChanged 	= true;

	fState->x0 = printRect.left;
	fState->y0 = printRect.top;
	fState->penX = 0;
	fState->penY = 0;

	// XXX should we clip to the printRect here?

	PushState(); // so that fState->prev != NULL

	return B_OK;
}


// --------------------------------------------------
status_t 
PDFWriter::EndPage()
{
	while (fState->prev != NULL) PopState();

    if (MakesPDF())
    	PDF_end_page(fPdf);
	fprintf(fLog, ">>>> PDF_end_page\n");

	delete fState; fState = NULL;
	
	return B_OK;
}


#ifdef CODEWARRIOR
	#pragma mark [PDFlib callbacks]
#endif

// --------------------------------------------------
size_t 
PDFWriter::WriteData(void *data, size_t	size)
{
	fprintf(fLog, ">>>> WriteData %p, %ld\n", data, size);

	return Transport()->Write(data, size);
}


// --------------------------------------------------
void 
PDFWriter::ErrorHandler(int	type, const char *msg)
{
	fprintf(fLog, ">>>> ErrorHandler %d: %s\n", type, msg);
}

#ifdef CODEWARRIOR
	#pragma mark [Generic drawing support routines]
#endif


// --------------------------------------------------
void
PDFWriter::PushInternalState() 
{
	fprintf(fLog, "PushInternalState\n");
	fState = new State(fState); fStateDepth ++;
}


// --------------------------------------------------
bool
PDFWriter::PopInternalState() 
{
	fprintf(fLog, "PopInternalState\n");
	if (fStateDepth != 0) {
		State* s = fState; fStateDepth --;
		fState = fState->prev;
		delete s;
		fprintf(fLog, "height = %f x0 = %f y0 = %f\n", fState->height, fState->x0, fState->y0);
		return true;
	} else {
		fprintf(fLog, "State stack underflow!\n");
		return false;
	}
}


// --------------------------------------------------
void 
PDFWriter::SetColor(rgb_color color) 
{
	if (!MakesPDF()) return;
	if (fState->currentColor.red != color.red || 
		fState->currentColor.blue != color.blue || 
		fState->currentColor.green != color.green || 
		fState->currentColor.alpha != color.alpha) {
		fState->currentColor = color;
		float red   = color.red / 255.0;
		float green = color.green / 255.0;
		float blue  = color.blue / 255.0;
		PDF_setcolor(fPdf, "both", "rgb", red, green, blue, 0.0);	
	}
}


// --------------------------------------------------
int
PDFWriter::FindPattern() 
{
	// TODO: use hashtable instead of BList for fPatterns
	const int n = fPatterns.CountItems();
	for (int i = 0; i < n; i ++) {
		Pattern* p = (Pattern*)fPatterns.ItemAt(i);
		if (p->Matches(fState->pattern, fState->backgroundColor, fState->foregroundColor)) return p->patternId;
	}
	return -1;
}


// --------------------------------------------------
void
PDFWriter::CreatePattern() 
{
	// TODO: create mask for transparent pixels
	fprintf(fLog, "CreatePattern\n");
	uint8 bitmap[8*8*3];
	uint8* data = (uint8*)fState->pattern.data;

	uint8* b = bitmap;
	for (int8 y = 0; y <= 7; y ++, data ++) {
		uint8 d = *data;
		for (int8 x = 0; x <= 7; x ++, d >>= 1, b += 3) {
			if (d & 1 == 1) {
				b[0] = fState->foregroundColor.red;
				b[1] = fState->foregroundColor.green;
				b[2] = fState->foregroundColor.blue;
			} else {
				b[0] = fState->backgroundColor.red;
				b[1] = fState->backgroundColor.green;
				b[2] = fState->backgroundColor.blue;
			}
		} 
	}

	int image = PDF_open_image(fPdf, "raw", "memory", (const char *) bitmap, sizeof(bitmap), 8, 8, 3, 8, 0);
	if (image == -1) {
		fprintf(fLog, "CreatePattern could not create image\n");
		return;
	}	
	int pattern = PDF_begin_pattern(fPdf, 8, 8, 8, 8, 2);
	if (pattern == -1) {
		fprintf(fLog, "CreatePattern could not create pattern\n");
		PDF_close_image(fPdf, image);
		return;
	}
	PDF_place_image(fPdf, image, 0, 0, 1);
	PDF_end_pattern(fPdf);
	PDF_close_image(fPdf, image);
	
	Pattern* p = new Pattern(fState->pattern, fState->backgroundColor, fState->foregroundColor, pattern);
	fPatterns.AddItem(p);
}


// --------------------------------------------------
void
PDFWriter::SetPattern() 
{
#if PATTERN_SUPPORT
	fprintf(fLog, "SetPattern (bitmap version)\n");
	if (MakesPattern()) {
		if (FindPattern() == -1) CreatePattern();
	} else {
		// assert(MakesPDF());
		int pattern = FindPattern();
		if (pattern != -1) {
			PDF_setcolor(fPdf, "both", "pattern", pattern, 0, 0, 0);
		} else {
			// TODO: fall back to another method
			fprintf(fLog, "error: pattern missing!");
		}
	}
#else
	fprintf(fLog, "SetPattern (color version)\n");
	if (IsSame(fState->pattern, B_MIXED_COLORS)) {
		rgb_color mixed;  // approximate mixed colors
		mixed.red    = (fState->foregroundColor.red + fState->backgroundColor.red) / 2;
		mixed.green  = (fState->foregroundColor.green + fState->backgroundColor.green) / 2;
		mixed.blue   = (fState->foregroundColor.blue + fState->backgroundColor.blue) / 2;
		mixed.alpha  = (fState->foregroundColor.alpha + fState->backgroundColor.alpha) / 2;
		SetColor(mixed);
	} else {
		SetColor(fState->foregroundColor);
	}
#endif
}


// --------------------------------------------------
void
PDFWriter::CreateLinePath(BPoint start, BPoint end, float width) {
	BPoint d = end - start; // calculate direction vector
	float len = sqrt(d.x*d.x + d.y*d.y); // normalize to length of 1.0
	d.x = d.x * width / len; d.y = d.y * width / len;
	d.x /= 2; d.y /= 2;
	BPoint r(-d.y, d.x); // rotate 90 degrees
	// generate rectangle that represents the line with penSize width 
	PDF_moveto(fPdf, tx(start.x + r.x), ty(start.y + r.y));
	PDF_lineto(fPdf, tx(start.x - r.x), ty(start.y - r.y));
	PDF_lineto(fPdf, tx(end.x - r.x),   ty(end.y - r.y));
	PDF_lineto(fPdf, tx(end.x + r.x),   ty(end.y + r.y));
	PDF_closepath(fPdf);
	
	fprintf(fLog, "LinePath: ");
	fprintf(fLog, "[%f, %f] ", tx(start.x + r.x), ty(start.y + r.y));
	fprintf(fLog, "[%f, %f] ", tx(start.x - r.x), ty(start.y - r.y));
	fprintf(fLog, "[%f, %f] ", tx(end.x - r.x),   ty(end.y - r.y));
	fprintf(fLog, "[%f, %f]\n", tx(end.x + r.x),   ty(end.y + r.y));
}


// --------------------------------------------------
// curve contains 4 points
void
PDFWriter::CreateBezierPath(BPoint *curve, float width) {
	Bezier bezier(curve, 4);
	BPoint start = bezier.PointAt(0);
	const int n = 10; // XXX find a heuristic to calculate this value
	for (int i = 1; i <= n; i ++) {
		BPoint end = bezier.PointAt(i / (float) n);
		CreateLinePath(start, end, width);
		start = end;
	}
}


// --------------------------------------------------
// curve contains 3 points
void
PDFWriter::CreateBezierPath(BPoint start, BPoint *curve, float width) {
	BPoint curve1[4] = { start, curve[0], curve[1], curve[2] };
	CreateBezierPath(curve1, width);
}


// --------------------------------------------------
void 
PDFWriter::StrokeOrClip() 
{
	if (IsDrawing()) {
		PDF_stroke(fPdf);
	} else {
		fprintf(fLog, "Warning: Clipping not implemented for this primitive!!!\n");
		PDF_closepath(fPdf);
	}
}


// --------------------------------------------------
void 
PDFWriter::FillOrClip() 
{
	if (IsDrawing()) {
		PDF_fill(fPdf);
	} else {
		PDF_closepath(fPdf);
	}
}


// --------------------------------------------------
void
PDFWriter::Paint(bool stroke) 
{
	if (stroke) {
		StrokeOrClip();
	} else {
		FillOrClip();
	}
}


// --------------------------------------------------
bool 
PDFWriter::IsSame(const pattern &p1, const pattern &p2)
{
	char *a = (char*)p1.data;
	char *b = (char*)p2.data;
	return strncmp(a, b, 8) == 0;
}


// --------------------------------------------------
bool 
PDFWriter::IsSame(const rgb_color &c1, const rgb_color &c2)
{
	char *a = (char*)&c1;
	char *b = (char*)&c1;
	return strncmp(a, b, sizeof(rgb_color)) == 0;
}


// --------------------------------------------------
void 
PDFWriter::SetColor() 
{
	if (IsSame(fState->pattern, B_SOLID_HIGH)) {
		SetColor(fState->foregroundColor);
	} else if (IsSame(fState->pattern, B_SOLID_LOW)) {
		SetColor(fState->backgroundColor);
	} else {
		SetPattern();
	} 
}


#ifdef CODEWARRIOR
	#pragma mark [Image drawing support routines]
#endif


// --------------------------------------------------
int32 
PDFWriter::BytesPerPixel(int32 pixelFormat) 
{
	switch (pixelFormat) {
		case B_RGB32:      // fall through
		case B_RGB32_BIG:  // fall through
		case B_RGBA32:     // fall through
		case B_RGBA32_BIG: return 4;

		case B_RGB24_BIG:  // fall through
		case B_RGB24:      return 3;

		case B_RGB16:      // fall through
		case B_RGB16_BIG:  // fall through
		case B_RGB15:      // fall through
		case B_RGB15_BIG:  // fall through
		case B_RGBA15:     // fall through
		case B_RGBA15_BIG: return 2;

		case B_GRAY8:      // fall through
		case B_CMAP8:      return 1;
		case B_GRAY1:      return 0;
		default: return -1;
	}
}


// --------------------------------------------------
bool 
PDFWriter::NeedsAlphaCheck(int32 pixelFormat) 
{
	switch (pixelFormat) {
		case B_RGB32:      // fall through
		case B_RGB32_BIG:  // fall through
		case B_RGBA32:     // fall through
		case B_RGBA32_BIG: // fall through
//		case B_RGB24:      // fall through
//		case B_RGB24_BIG:  // fall through
//		case B_RGB16:      // fall through
//		case B_RGB16_BIG:  // fall through
		case B_RGB15:      // fall through
		case B_RGB15_BIG:  // fall through
		case B_RGBA15:     // fall through
		case B_RGBA15_BIG: // fall through
		case B_CMAP8:      return true;
		default: return false;
	}
}


// --------------------------------------------------
bool 
PDFWriter::IsTransparentRGB32(uint8* in) 
{
	return *((uint32*)in) == B_TRANSPARENT_MAGIC_RGBA32;
}


// --------------------------------------------------
bool 
PDFWriter::IsTransparentRGB32_BIG(uint8* in) 
{
	return *(uint32*)in == B_TRANSPARENT_MAGIC_RGBA32_BIG;
}


// --------------------------------------------------
bool 
PDFWriter::IsTransparentRGBA32(uint8* in) 
{
	return in[3] < 128 || IsTransparentRGB32(in);
}


// --------------------------------------------------
bool 
PDFWriter::IsTransparentRGBA32_BIG(uint8* in) 
{
	return in[0] < 127 || IsTransparentRGB32_BIG(in);
}

/*
// --------------------------------------------------
bool 
PDFWriter::IsTransparentRGB24(uint8* in) 
{
	return false;
}


// --------------------------------------------------
bool 
PDFWriter::IsTransparentRGB24_BIG(uint8* in) 
{
}


// --------------------------------------------------
bool 
PDFWriter::IsTransparentRGB16(uint8* in) 
{
	return false;
}


// --------------------------------------------------
bool 
PDFWriter::IsTransparentRGB16_BIG(uint8* in) 
{
}
*/


// --------------------------------------------------
bool 
PDFWriter::IsTransparentRGB15(uint8* in) 
{
	return *((uint16*)in) == B_TRANSPARENT_MAGIC_RGBA15;
}


// --------------------------------------------------
bool 
PDFWriter::IsTransparentRGB15_BIG(uint8* in) 
{
	// 01234567 01234567
	// 00123434 01201234
	// -RRRRRGG GGGBBBBB
	return *(uint16*)in == B_TRANSPARENT_MAGIC_RGBA15_BIG;
}


// --------------------------------------------------
bool 
PDFWriter::IsTransparentRGBA15(uint8* in) 
{
	// 01234567 01234567
	// 01201234 00123434
	// GGGBBBBB ARRRRRGG
	return in[1] & 1 == 0 || IsTransparentRGB15(in);
}


// --------------------------------------------------
bool 
PDFWriter::IsTransparentRGBA15_BIG(uint8* in) 
{
	// 01234567 01234567
	// 00123434 01201234
	// ARRRRRGG GGGBBBBB
	return in[0] & 1 == 0 || IsTransparentRGB15_BIG(in);
}


// --------------------------------------------------
bool 
PDFWriter::IsTransparentCMAP8(uint8* in) 
{
	return *in == B_TRANSPARENT_MAGIC_CMAP8;
}


/*
// --------------------------------------------------
bool 
PDFWriter::IsTransparentGRAY8(uint8* in)
{
}


// --------------------------------------------------
bool 
PDFWriter::IsTransparentGRAY1(uint8* in, int8 bit)
{
}
*/


// --------------------------------------------------
void *
PDFWriter::CreateMask(BRect src, int32 bytesPerRow, int32 pixelFormat, int32 flags, void *data)
{
	uint8	*in;
	uint8   *inRow;
	int32	x, y;
	
	uint8	*mask;
	uint8	*maskRow;
	uint8	*out;
	int32	maskWidth;
	uint8	shift;
	bool	alpha;
	int32   bpp = 4; 

	bpp = BytesPerPixel(pixelFormat);	
	if (bpp < 0) 
		return NULL;
	
	int32	width = src.IntegerWidth() + 1;
	int32	height = src.IntegerHeight() + 1;
		
	if (!NeedsAlphaCheck(pixelFormat))
		return NULL;

	// Image Mask
	inRow = (uint8 *) data;
	inRow += bytesPerRow * (int) src.top + bpp * (int) src.left;

	maskWidth = (width+7)/8;
	maskRow = mask = new uint8[maskWidth * height];
	memset(mask, 0, maskWidth*height);
	alpha = false;
	
	for (y = height; y > 0; y--) {
		in = inRow;
		out = maskRow;
		shift = 7;
		
		bool a = false;			

		for (x = width; x > 0; x-- ) {
			// For each pixel
			switch (pixelFormat) {
				case B_RGB32:      a = IsTransparentRGB32(in); break;
				case B_RGB32_BIG:  a = IsTransparentRGB32_BIG(in); break;
				case B_RGBA32:     a = IsTransparentRGBA32(in); break;
				case B_RGBA32_BIG: a = IsTransparentRGBA32_BIG(in); break;
				//case B_RGB24:      a = IsTransparentRGB24(in); break;
				//case B_RGB24_BIG:  a = IsTransparentRGB24_BIG(in); break;
				//case B_RGB16:      a = IsTransparentRGB16(in); break;
				//case B_RGB16_BIG:  a = IsTransparentRGB16_BIG(in); break;
				case B_RGB15:      a = IsTransparentRGB15(in); break;
				case B_RGB15_BIG:  a = IsTransparentRGB15_BIG(in); break;
				case B_RGBA15:     a = IsTransparentRGBA15(in); break;
				case B_RGBA15_BIG: a = IsTransparentRGBA15_BIG(in); break;
				case B_CMAP8:      a = IsTransparentCMAP8(in); break;
				//case B_GRAY8:      a = IsTransparentGRAY8(in); break;
				//case B_GRAY1:      a = false; break;
				default: a = false; // should not reach here
					fprintf(fLog, "ERROR: CreatMask: non transparent able pixelFormat\n");
			}

			if (a) {
				out[0] |= (1 << shift);
				alpha = true;
			}
			// next pixel			
			if (shift == 0) out ++;
			shift = (shift + 7) & 0x07;		
			in += bpp;
		}

		// next row
		inRow += bytesPerRow;
		maskRow += maskWidth;
	}
	
	if (!alpha) {
		delete []mask;
		mask = NULL;
	}
	return mask;
}


// --------------------------------------------------
void 
PDFWriter::ConvertFromRGB32(uint8* in, uint8 *out)
{
	*((rgb_color*)out) = *((rgb_color*)in);
}


// --------------------------------------------------
void 
PDFWriter::ConvertFromRGBA32(uint8* in, uint8 *out)
{
	*((rgb_color*)out) = *((rgb_color*)in);
}


// --------------------------------------------------
void 
PDFWriter::ConvertFromRGB24(uint8* in, uint8 *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = 255;
}


// --------------------------------------------------
void 
PDFWriter::ConvertFromRGB16(uint8* in, uint8 *out)
{
	// 01234567 01234567
	// 01201234 01234345
	// GGGBBBBB RRRRRGGG
	out[0] = in[0] & 0xf8; // blue
	out[1] = ((in[0] & 7) << 2) | (in[1] & 0xe0); // green
	out[2] = in[1] << 3; // red
	out[3] = 255;
}


// --------------------------------------------------
void 
PDFWriter::ConvertFromRGB15(uint8* in, uint8 *out)
{
	// 01234567 01234567
	// 01201234 00123434
	// GGGBBBBB -RRRRRGG
	out[0] = in[0] & 0xf8; // blue
	out[1] = ((in[0] & 7) << 3) | (in[1] & 0xc0); // green
	out[2] = (in[1] & ~1) << 2; // red
	out[3] = 255;
}


// --------------------------------------------------
void 
PDFWriter::ConvertFromRGBA15(uint8* in, uint8 *out)
{
	// 01234567 01234567
	// 01201234 00123434
	// GGGBBBBB ARRRRRGG
	out[0] = in[0] & 0xf8; // blue
	out[1] = ((in[0] & 7) << 3) | (in[1] & 0xc0); // green
	out[2] = (in[1] & ~1) << 2; // red
	out[3] = in[1] << 7;
}


// --------------------------------------------------
void 
PDFWriter::ConvertFromCMAP8(uint8* in, uint8 *out)
{
	rgb_color c = fScreen->ColorForIndex(in[0]);
	out[0] = c.blue;
	out[1] = c.green;
	out[2] = c.red;
	out[3] = c.alpha;
}


// --------------------------------------------------
void 
PDFWriter::ConvertFromGRAY8(uint8* in, uint8 *out)
{
	out[0] = in[0];
	out[1] = in[0];
	out[2] = in[0];
	out[3] = 255;
}


// --------------------------------------------------
void 
PDFWriter::ConvertFromGRAY1(uint8* in, uint8 *out, int8 bit)
{
	uint8 gray = (in[0] & (1 << bit)) ? 255 : 0;
	out[0] = gray;
	out[1] = gray;
	out[2] = gray;
	out[3] = 255;
}


// --------------------------------------------------
void 
PDFWriter::ConvertFromRGB32_BIG(uint8* in, uint8 *out)
{
	out[0] = in[3];
	out[1] = in[2];
	out[2] = in[1];
	out[3] = 255;
}


// --------------------------------------------------
void 
PDFWriter::ConvertFromRGBA32_BIG(uint8* in, uint8 *out)
{
	out[0] = in[3];
	out[1] = in[2];
	out[2] = in[1];
	out[3] = in[0];
}


// --------------------------------------------------
void 
PDFWriter::ConvertFromRGB24_BIG(uint8* in, uint8 *out)
{
	out[0] = in[2];
	out[1] = in[1];
	out[2] = in[0];
	out[3] = 255;
}


// --------------------------------------------------
void 
PDFWriter::ConvertFromRGB16_BIG(uint8* in, uint8 *out)
{
	// 01234567 01234567
	// 01234345 01201234
	// RRRRRGGG GGGBBBBB
	out[0] = in[2] & 0xf8; // blue
	out[1] = ((in[1] & 7) << 2) | (in[0] & 0xe0); // green
	out[2] = in[0] << 3; // red
	out[3] = 255;
}


// --------------------------------------------------
void 
PDFWriter::ConvertFromRGB15_BIG(uint8* in, uint8 *out)
{
	// 01234567 01234567
	// 00123434 01201234
	// -RRRRRGG GGGBBBBB
	out[0] = in[1] & 0xf8; // blue
	out[1] = ((in[1] & 7) << 3) | (in[0] & 0xc0); // green
	out[2] = (in[0] & ~1) << 2; // red
	out[3] = 255;
}


// --------------------------------------------------
void 
PDFWriter::ConvertFromRGBA15_BIG(uint8* in, uint8 *out)
{
	// 01234567 01234567
	// 00123434 01201234
	// ARRRRRGG GGGBBBBB
	out[0] = in[1] & 0xf8; // blue
	out[1] = ((in[1] & 7) << 3) | (in[0] & 0xc0); // green
	out[2] = (in[0] & ~1) << 2; // red
	out[3] = in[0] << 7;
}


// --------------------------------------------------
// Convert and clip bits to colorspace B_RGBA32
BBitmap *
PDFWriter::ConvertBitmap(BRect src, int32 bytesPerRow, int32 pixelFormat, int32 flags, void *data)
{
	uint8	*in;
	uint8   *inLeft;
	uint8	*out;
	uint8   *outLeft;
	int32	x, y;
	int8    bit;
	int32   bpp = 4; 

	bpp = BytesPerPixel(pixelFormat);	
	if (bpp < 0) 
		return NULL;

	int32 width  = src.IntegerWidth();
	int32 height = src.IntegerHeight();
	BBitmap *	bm = new BBitmap(BRect(0, 0, width, height), B_RGB32);
	if (!bm->IsValid()) {
		delete bm;
		fprintf(fLog, "BBitmap constructor failed\n");
		return NULL;
	}

	inLeft  = (uint8 *)data;
	inLeft += bytesPerRow * (int)src.top + bpp * (int)src.left; 
	outLeft	= (uint8*)bm->Bits();
		
	for (y = height; y >= 0; y--) {
		in = inLeft;
		out = outLeft;

		for (x = 0; x <= width; x++) {
			// For each pixel
			switch (pixelFormat) {
				case B_RGB32:      ConvertFromRGB32(in, out); break;
				case B_RGBA32:     ConvertFromRGBA32(in, out); break;
				case B_RGB24:      ConvertFromRGB24(in, out); break;
				case B_RGB16:      ConvertFromRGB16(in, out); break;
				case B_RGB15:      ConvertFromRGB15(in, out); break;
				case B_RGBA15:     ConvertFromRGBA15(in, out); break;
				case B_CMAP8:      ConvertFromCMAP8(in, out); break;
				case B_GRAY8:      ConvertFromGRAY8(in, out); break;
				case B_GRAY1:      
					bit = x & 7;
					ConvertFromGRAY1(in, out, bit);
					if (bit == 7) in ++; 
					break;
				case B_RGB32_BIG:  ConvertFromRGB32_BIG(in, out); break;
				case B_RGBA32_BIG: ConvertFromRGBA32_BIG(in, out); break;
				case B_RGB24_BIG:  ConvertFromRGB24_BIG(in, out); break;
				case B_RGB16_BIG:  ConvertFromRGB16_BIG(in, out); break;
				case B_RGB15_BIG:  ConvertFromRGB15_BIG(in, out); break;
				case B_RGBA15_BIG: ConvertFromRGBA15_BIG(in, out); break;
				default:; // should not reach here
			}
			in += bpp;	// next pixel
			out += 4;
		}

		// next row
		inLeft += bytesPerRow;
		outLeft += bm->BytesPerRow();
	}
	
	return bm;
}


// --------------------------------------------------
bool 
PDFWriter::StoreTranslatorBitmap(BBitmap *bitmap, const char *filename, uint32 type)
{
	BTranslatorRoster *roster = BTranslatorRoster::Default();
	if (!roster) {
		fprintf(fLog, "roster == NULL!\n");
		return false;
	}
	BBitmapStream stream(bitmap); // init with contents of bitmap
/*
	translator_info info, *i = NULL;
	if (quality != -1.0 && roster->Identify(&stream, NULL, &info) == B_OK) {
		#ifdef DEBUG
		fprintf(fLog, ">>> translator_info:\n");
		fprintf(fLog, "   type = %4.4s\n", (char*)&info.type);
		fprintf(fLog, "   id = %d\n", info.translator);
		fprintf(fLog, "   group = %4.4s\n", (char*)&info.group);
		fprintf(fLog, "   quality = %f\n", info.quality);
		fprintf(fLog, "   capability = %f\n", info.type);
		fprintf(fLog, "   name = %s\n", info.name);
		fprintf(fLog, "   MIME = %s\n", info.MIME);
		#endif
		
		int32 numInfo;
		translator_info *tInfo = NULL;
		if (roster->GetTranslators(&stream, NULL, &tInfo, &numInfo, 
			info.type, NULL, type) == B_OK) {

//			#ifdef DEBUG
			for (int j = 0; j < numInfo; j++) {
				fprintf(fLog, ">>> translator_info [%d]:\n", j);
				fprintf(fLog, "   type = %4.4s\n", (char*)&tInfo[j].type);
				fprintf(fLog, "   id = %d\n", tInfo[j].translator);
				fprintf(fLog, "   group = %4.4s\n", (char*)&tInfo[j].group);
				fprintf(fLog, "   quality = %f\n", tInfo[j].quality);
				fprintf(fLog, "   capability = %f\n", tInfo[j].type);
				fprintf(fLog, "   name = %s\n", tInfo[j].name);
				fprintf(fLog, "   MIME = %s\n", tInfo[j].MIME);
			}
//			#endif

			BMessage m;
			status_t s = roster->GetConfigurationMessage(tInfo[0].translator, &m);
			if (s == B_OK) {
				fMessagePrinter.Print(&m);
			} else {
				fprintf(fLog, "ERROR: could not get configuration message %d\n", s);
			}
			
			info = tInfo[0];
			info.quality = quality;
			delete []tInfo;

			i = &info;
		}
	}
*/
	BFile file(filename, B_CREATE_FILE | B_WRITE_ONLY | B_ERASE_FILE);
	bool res = roster->Translate(&stream, NULL /*i*/, NULL, &file, type) == B_OK;
	BBitmap *bm = NULL; stream.DetachBitmap(&bm); // otherwise bitmap destructor crashes here!
	ASSERT(bm == bitmap);
	return res;
}

#ifdef CODEWARRIOR
	#pragma mark [BShape drawing support routines]
#endif


// --------------------------------------------------
DrawShape::DrawShape(PDFWriter *writer, bool stroke)
	: fWriter(writer)
	, fStroke(stroke)
	, fDrawn(false)
	, fCurrentPoint(0, 0)
{
}


// --------------------------------------------------
DrawShape::~DrawShape()
{
	Draw();
}


// --------------------------------------------------
status_t 
DrawShape::IterateBezierTo(int32 bezierCount, BPoint *control)
{
	fprintf(Log(), "IterateBezierTo %d\n", (int)bezierCount);
	for (int32 i = 0; i < bezierCount; i++, control += 3) {
		if (TransformPath()) {
			fWriter->CreateBezierPath(fCurrentPoint, control, PenSize());
		} else {
			PDF_curveto(Pdf(), 
				tx(control[0].x), ty(control[0].y),
				tx(control[1].x), ty(control[1].y),
	    		tx(control[2].x), ty(control[2].y));
	    }
		fprintf(Log(), "(%f %f), (%f %f), (%f %f)\n", 
			control[0].x, control[0].y,
			control[1].x, control[1].y,
	    	control[2].x, control[2].y);
		fCurrentPoint = control[2];
	}
	return B_OK;
}


// --------------------------------------------------
status_t 
DrawShape::IterateClose(void)
{
	fprintf(Log(), "IterateClose %s\n", IsDrawing() ? (fStroke ? "stroke" : "fill") : "clip");
	if (fDrawn) fprintf(Log(), ">>> IterateClose called multiple times!");
	PDF_closepath(Pdf());
	Draw();
	return B_OK;
}


// --------------------------------------------------
void
DrawShape::Draw()
{
	if (!fDrawn) {
		fDrawn = true;
		if (IsDrawing()) {
			if (fStroke) 
				PDF_stroke(Pdf()); 
			else {
				PDF_fill(Pdf());
			}
		} else {
			PDF_closepath(Pdf());
		}
	}
}


// --------------------------------------------------
status_t 
DrawShape::IterateLineTo(int32 lineCount, BPoint *linePoints)
{
	fprintf(Log(), "IterateLineTo %d\n", (int)lineCount);
	BPoint *p = linePoints;
	for (int32 i = 0; i < lineCount; i++) {
		fprintf(Log(), "(%f, %f) ", p->x, p->y);

		if (TransformPath()) {
			fWriter->CreateLinePath(fCurrentPoint, *p, PenSize());
		} else {
			PDF_lineto(Pdf(), tx(p->x), ty(p->y));
		}
		fCurrentPoint = *p;
		p++;
	}
	fprintf(Log(), "\n");
	return B_OK;
}


// --------------------------------------------------
status_t 
DrawShape::IterateMoveTo(BPoint *point)
{
	fprintf(Log(), "IterateMoveTo ");
	if (!TransformPath()) {
		PDF_moveto(Pdf(), tx(point->x), ty(point->y)); 
	}
	fprintf(Log(), "(%f, %f)\n", point->x, point->y);
	fCurrentPoint = *point;
	return B_OK;
}

#ifdef CODEWARRIOR
	#pragma mark -- BPicture playback handlers
#endif


// --------------------------------------------------
void PDFWriter::Op(int number)
{
	fprintf(fLog, "Unhandled operand %d\n", number);
}


// --------------------------------------------------
void	
PDFWriter::MovePenBy(BPoint delta)
{
	fprintf(fLog, "MovePenBy delta=[%f, %f]\n",
			delta.x, delta.y);
	fState->penX += delta.x;
	fState->penY += delta.y;
}


// --------------------------------------------------
void
PDFWriter::StrokeLine(BPoint start,	BPoint end)
{
	fprintf(fLog, "StrokeLine start=[%f, %f], end=[%f, %f]\n",
			start.x, start.y, end.x, end.y);

	SetColor();			
	if (!MakesPDF()) return;
	if (IsClipping()) {
		CreateLinePath(start, end, fState->penSize);
	} else {
		PDF_moveto(fPdf, tx(start.x), ty(start.y));
		PDF_lineto(fPdf, tx(end.x),   ty(end.y));
		StrokeOrClip();
	}
}


// --------------------------------------------------
void 
PDFWriter::StrokeRect(BRect rect)
{
	fprintf(fLog, "StrokeRect rect=[%f, %f, %f, %f]\n",
			rect.left, rect.top, rect.right, rect.bottom);

	SetColor();			
	if (!MakesPDF()) return;
	if (IsClipping()) {
		CreateLinePath(BPoint(rect.left, rect.top), BPoint(rect.right, rect.top), fState->penSize);
		CreateLinePath(BPoint(rect.right, rect.top), BPoint(rect.right, rect.bottom), fState->penSize);
		CreateLinePath(BPoint(rect.right, rect.bottom), BPoint(rect.left, rect.bottom), fState->penSize);
		CreateLinePath(BPoint(rect.left, rect.bottom), BPoint(rect.left, rect.top), fState->penSize);
	} else {
		PDF_rect(fPdf, tx(rect.left), ty(rect.bottom), scale(rect.Width()), scale(rect.Height()));
		StrokeOrClip();
	}
}


// --------------------------------------------------
void	
PDFWriter::FillRect(BRect rect)
{
	fprintf(fLog, "FillRect rect=[%f, %f, %f, %f]\n",
			rect.left, rect.top, rect.right, rect.bottom);

	SetColor();			
	if (!MakesPDF()) return;
	PDF_rect(fPdf, tx(rect.left), ty(rect.bottom), scale(rect.Width()), scale(rect.Height()));
	FillOrClip();
}


// --------------------------------------------------
// The quarter ellipses in the corners of the rectangle are
// approximated with bezier curves.
// The constant 0.555... is taken from gobeProductive.
void	
PDFWriter::PaintRoundRect(BRect rect, BPoint radii, bool stroke) {
	SetColor();
	if (!MakesPDF()) return;

	BPoint center;
	
	float sx = scale(radii.x);
	float sy = scale(radii.y);
	
	float ax = sx;
	float bx = 0.5555555555555 * sx;
	float ay = sy;
	float by = 0.5555555555555 * sy;

	center.x = tx(rect.left) + sx;
	center.y = ty(rect.top) - sy;
	
	PDF_moveto(fPdf, center.x - ax, center.y);
	PDF_curveto(fPdf, 
		center.x - ax, center.y + by,
		center.x - bx, center.y + ay,
		center.x     , center.y + ay);
	
	center.x = tx(rect.right) - sx;
	PDF_lineto(fPdf, center.x, center.y + ay);
	PDF_curveto(fPdf, 
		center.x + bx, center.y + ay,
		center.x + ax, center.y + by,
		center.x + ax, center.y);
		
	center.y = ty(rect.bottom) + sy;
	PDF_lineto(fPdf, center.x + sx, center.y); 
	PDF_curveto(fPdf, 
		center.x + ax, center.y - by,
		center.x + bx, center.y - ay,
		center.x     , center.y - ay);
	
	center.x = tx(rect.left) + sx;	
	PDF_lineto(fPdf, center.x, center.y - ay);
	PDF_curveto(fPdf, 
		center.x - bx, center.y - ay,
		center.x - ax, center.y - by,
		center.x - ax, center.y);

	PDF_closepath(fPdf);
	
	Paint(stroke);
}


// --------------------------------------------------
void	
PDFWriter::StrokeRoundRect(BRect rect, BPoint radii)
{
	fprintf(fLog, "StrokeRoundRect center=[%f, %f, %f, %f], radii=[%f, %f]\n",
			rect.left, rect.top, rect.right, rect.bottom, radii.x, radii.y);
	PaintRoundRect(rect, radii, true);
}


// --------------------------------------------------
void	
PDFWriter::FillRoundRect(BRect rect, BPoint	radii)
{
	fprintf(fLog, "FillRoundRect rect=[%f, %f, %f, %f], radii=[%f, %f]\n",
			rect.left, rect.top, rect.right, rect.bottom, radii.x, radii.y);
	PaintRoundRect(rect, radii, false);
}


// --------------------------------------------------
void	
PDFWriter::StrokeBezier(BPoint	*control)
{
	fprintf(fLog, "StrokeBezier\n");
	SetColor();
	if (!MakesPDF()) return;
	if (IsClipping()) {
		CreateBezierPath(control, fState->penSize);
	} else {
		PDF_moveto(fPdf, tx(control[0].x), ty(control[0].y));
		PDF_curveto(fPdf, tx(control[1].x), ty(control[1].y),
		            tx(control[2].x), ty(control[2].y),
		            tx(control[3].x), ty(control[3].y));
		StrokeOrClip();
	}
}


// --------------------------------------------------
void	
PDFWriter::FillBezier(BPoint *control)
{
	fprintf(fLog, "FillBezier\n");
	SetColor();
	if (!MakesPDF()) return;
	PDF_moveto(fPdf, tx(control[0].x), ty(control[0].y));
	PDF_curveto(fPdf, tx(control[1].x), ty(control[1].y),
	            tx(control[2].x), ty(control[2].y),
	            tx(control[3].x), ty(control[3].y));
	PDF_closepath(fPdf);
	FillOrClip();
}


// --------------------------------------------------
// Note the pen size is also scaled!
// We should approximate it with bezier curves too!
void
PDFWriter::PaintArc(BPoint center, BPoint radii, float startTheta, float arcTheta, int stroke) {
	float sx = scale(radii.x);
	float sy = scale(radii.y);
	float smax = sx > sy ? sx : sy;

	SetColor();
	if (!MakesPDF()) return;

	PDF_save(fPdf);
	PDF_scale(fPdf, sx, sy);
	PDF_setlinewidth(fPdf, fState->penSize / smax);
	PDF_arc(fPdf, tx(center.x) / sx, ty(center.y) / sy, 1, startTheta, startTheta + arcTheta);	
	Paint(stroke);
	PDF_restore(fPdf);
}


// --------------------------------------------------
void
PDFWriter::StrokeArc(BPoint center, BPoint radii, float startTheta, float arcTheta)
{
	fprintf(fLog, "StrokeArc center=[%f, %f], radii=[%f, %f], startTheta=%f, arcTheta=%f\n",
			center.x, center.y, radii.x, radii.y, startTheta, arcTheta);
	PaintArc(center, radii, startTheta, arcTheta, true);
}


// --------------------------------------------------
void 
PDFWriter::FillArc(BPoint center, BPoint radii, float startTheta, float arcTheta)
{
	fprintf(fLog, "FillArc center=[%f, %f], radii=[%f, %f], startTheta=%f, arcTheta=%f\n",
			center.x, center.y, radii.x, radii.y, startTheta, arcTheta);
	PaintArc(center, radii, startTheta, arcTheta, false);
}


// --------------------------------------------------
void
PDFWriter::PaintEllipse(BPoint center, BPoint radii, bool stroke)
{
	float sx = scale(radii.x);
	float sy = scale(radii.y);
	
	center.x = tx(center.x); center.y = ty(center.y);

	float ax = sx;
	float bx = 0.5555555555555 * sx;
	float ay = sy;
	float by = 0.5555555555555 * sy;

	SetColor();
	if (!MakesPDF()) return;

	PDF_moveto(fPdf, center.x - ax, center.y);
	PDF_curveto(fPdf, 
		center.x - ax, center.y - by,
		center.x - bx, center.y - ay,
		center.x     , center.y - ay);
	PDF_curveto(fPdf, 
		center.x + bx, center.y - ay,
		center.x + ax, center.y - by,
		center.x + ax, center.y);
	PDF_curveto(fPdf, 
		center.x + ax, center.y + by,
		center.x + bx, center.y + ay,
		center.x     , center.y + ay);
	PDF_curveto(fPdf, 
		center.x - bx, center.y + ay,
		center.x - ax, center.y + by,
		center.x - ax, center.y);
	
	PDF_closepath(fPdf);
	
	Paint(stroke);
}

// --------------------------------------------------
void
PDFWriter::StrokeEllipse(BPoint center, BPoint radii)
{
	fprintf(fLog, "StrokeEllipse center=[%f, %f], radii=[%f, %f]\n",
			center.x, center.y, radii.x, radii.y);
	PaintEllipse(center, radii, true);
}


// --------------------------------------------------
void
PDFWriter::FillEllipse(BPoint center, BPoint radii)
{
	fprintf(fLog, "FillEllipse center=[%f, %f], radii=[%f, %f]\n",
			center.x, center.y, radii.x, radii.y);
	PaintEllipse(center, radii, false);
}


// --------------------------------------------------
void 
PDFWriter::StrokePolygon(int32 numPoints, BPoint *points, bool isClosed)
{
	int32	i;
	float   x0, y0;
	fprintf(fLog, "StrokePolygon numPoints=%ld, isClosed=%d\npoints=",
			numPoints, isClosed);

	if (numPoints <= 1) return;
	
	x0 = y0 = 0.0;
		
	SetColor();		
	if (!MakesPDF()) return;

	if (IsClipping()) {
		fprintf(fLog, " clipping");
		fprintf(fLog, " [%f, %f]", points->x, points->y);
		x0 = points->x;
		y0 = points->y;
		BPoint p(x0, y0);
		for (i = 1, points++; i < numPoints; i++, points++ ) {
			fprintf(fLog, " [%f, %f]", points->x, points->y);
			CreateLinePath(p, *points, fState->penSize);
			p = *points;
		}
		if (isClosed)
			CreateLinePath(p, BPoint(x0, y0), fState->penSize);
	} else {
		for ( i = 0; i < numPoints; i++, points++ ) {
			fprintf(fLog, " [%f, %f]", points->x, points->y);
			if (i != 0) {
				PDF_lineto(fPdf, tx(points->x), ty(points->y));
			} else {
				x0 = tx(points->x);
				y0 = ty(points->y);
				PDF_moveto(fPdf, x0, y0);
			}
		}
		if (isClosed) 
			PDF_lineto(fPdf, x0, y0);
		StrokeOrClip();
	}
	fprintf(fLog, "\n");
}


// --------------------------------------------------
void 
PDFWriter::FillPolygon(int32 numPoints, BPoint *points, bool isClosed)
{
	int32	i;
	
	fprintf(fLog, "FillPolygon numPoints=%ld, isClosed=%dpoints=\n",
			numPoints, isClosed);
			
	SetColor();		
	if (!MakesPDF()) return;

	for ( i = 0; i < numPoints; i++, points++ ) {
		fprintf(fLog, " [%f, %f]", points->x, points->y);
		if (i != 0) {
			PDF_lineto(fPdf, tx(points->x), ty(points->y));
		} else {
			PDF_moveto(fPdf, tx(points->x), ty(points->y));
		}
	}
	PDF_closepath(fPdf);
	FillOrClip();
	fprintf(fLog, "\n");
}


// --------------------------------------------------
void PDFWriter::StrokeShape(BShape *shape)
{
	fprintf(fLog, "StrokeShape\n");
	SetColor();			
	if (!MakesPDF()) return;
	DrawShape iterator(this, true);
	iterator.Iterate(shape);
}


// --------------------------------------------------
void PDFWriter::FillShape(BShape *shape)
{
	fprintf(fLog, "FillShape\n");
	SetColor();			
	if (!MakesPDF()) return;
	DrawShape iterator(this, false);
	iterator.Iterate(shape);
}


// --------------------------------------------------
void
PDFWriter::ClipToPicture(BPicture *picture, BPoint point, bool clip_to_inverse_picture)
{
	fprintf(fLog, "ClipToPicture at (%f, %f) clip_to_inverse_picture = %s\n", point.x, point.y, clip_to_inverse_picture ? "true" : "false");
	if (!MakesPDF()) return;
	if (clip_to_inverse_picture) {
		fprintf(fLog, "Clipping to inverse picture not implemented!\n");
		return;
	}
	if (fMode == kDrawingMode) {
		const bool set_origin = point.x != 0 || point.y != 0;
		if (set_origin) { 
			PushInternalState(); SetOrigin(point); PushInternalState();
		}

		fMode = kClippingMode;
		// create subpath(s) for clipping
		Iterate(picture);
		fMode = kDrawingMode;
		// and clip to it/them
		PDF_clip(fPdf);
				
		if (set_origin) {
			PopInternalState(); PopInternalState();
		}

		fprintf(fLog, "Returning from ClipToPicture\n");
	} else {
		fprintf(fLog, "Nested call of ClipToPicture not implemented yet!\n");
	}
}

// --------------------------------------------------
void	
PDFWriter::DrawPixels(BRect src, BRect dest, int32 width, int32 height, int32 bytesPerRow, int32 pixelFormat, 
	int32 flags, void *data)
{
	int		image;
	void 	*mask = NULL;
	int     maskId = -1;

	fprintf(fLog, "DrawPixels src=[%f, %f, %f, %f], dest=[%f, %f, %f, %f], "
					"width=%ld, height=%ld, bytesPerRow=%ld, pixelFormat=%ld, "
					"flags=%ld, data=%p\n",
					src.left, src.top, src.right, src.bottom,
					dest.left, dest.top, dest.right, dest.bottom,
					width, height, bytesPerRow, pixelFormat, flags, data);

	if (!MakesPDF()) return;
	
	if (IsClipping()) {
		fprintf(fLog, "DrawPixels for clipping not implemented yet!");
		return;
	}

	mask = CreateMask(src, bytesPerRow, pixelFormat, flags, data);

	float scaleX = (dest.Width()+1) / (src.Width()+1);
	float scaleY = (dest.Height()+1) / (src.Height()+1);

	if (mask) {
		int32 w = (width+7)/8;
		int32 h = height;
		maskId = PDF_open_image(fPdf, "raw", "memory", (const char *) mask, w*h, width, height, 1, 1, "mask");
		delete []mask;
	}

	BBitmap * bm = ConvertBitmap(src, bytesPerRow, pixelFormat, flags, data);
	if (!bm) {
		fprintf(fLog, "ConvertBitmap failed!\n");
		if (maskId != -1) PDF_close_image(fPdf, maskId);
		return;
	}

	char *pdfLibFormat   = "png";
	char *bitmapFileName = "/tmp/pdfwriter.png";	
	const uint32 beosFormat    = B_PNG_FORMAT;

	if (!StoreTranslatorBitmap(bm, bitmapFileName, beosFormat)) {
		delete bm;
		fprintf(fLog, "StoreTranslatorBitmap failed\n");
		if (maskId != -1) PDF_close_image(fPdf, maskId);
		return;
	}
	delete bm;
	
	PDF_save(fPdf);
	PDF_scale(fPdf, scaleX, scaleY);	

	float x = tx(dest.left)   / scaleX;
	float y = ty(dest.bottom) / scaleY;

	image = PDF_open_image_file(fPdf, pdfLibFormat, bitmapFileName, 
		maskId == -1 ? "" : "masked", maskId == -1 ? 0 : maskId);
	if ( image >= 0 ) {
		PDF_place_image(fPdf, image, x, y, scale(1.0));
		PDF_close_image(fPdf, image);
	} else 
		fprintf(fLog, "PDF_open_image_file failed!\n");

	if (maskId != -1) PDF_close_image(fPdf, maskId);
	PDF_restore(fPdf);
}


// --------------------------------------------------
void	
PDFWriter::SetClippingRects(BRect *rects, uint32 numRects)
{
	uint32	i;
	
	fprintf(fLog, "SetClippingRects numRects=%ld\nrects=",
			numRects);

	if (!MakesPDF()) return;
	
	for ( i = 0; i < numRects; i++, rects++ ) {
		fprintf(fLog, " [%f, %f, %f, %f]",
				rects->left, rects->top, rects->right, rects->bottom);
		PDF_moveto(fPdf, tx(rects->left),  ty(rects->top)); 
		PDF_lineto(fPdf, tx(rects->right), ty(rects->top));
		PDF_lineto(fPdf, tx(rects->right), ty(rects->bottom));
		PDF_lineto(fPdf, tx(rects->left),  ty(rects->bottom));
		PDF_closepath(fPdf);
	}
	if (numRects > 0) PDF_clip(fPdf);
	fprintf(fLog, "\n");
}


// --------------------------------------------------
void
PDFWriter::PushState()
{
	fprintf(fLog, "PushState\n");
	PushInternalState();
	fprintf(fLog, "height = %f x0 = %f y0 = %f\n", fState->height, fState->x0, fState->y0);
	if (!MakesPDF()) return;
	PDF_save(fPdf);
}


// --------------------------------------------------
void
PDFWriter::PopState()
{
	fprintf(fLog, "PopState\n");
	if (PopInternalState()) {
		if (!MakesPDF()) return;
		PDF_restore(fPdf);
	}
}


// --------------------------------------------------
void
PDFWriter::EnterStateChange()
{
	fprintf(fLog, "EnterStateChange\n");
	// nothing to do
}


// --------------------------------------------------
void
PDFWriter::ExitStateChange()
{
	fprintf(fLog, "ExitStateChange\n");
	// nothing to do
}


// --------------------------------------------------
void
PDFWriter::EnterFontState()
{
	fprintf(fLog, "EnterFontState\n");
	// nothing to do
}


// --------------------------------------------------
void
PDFWriter::ExitFontState()
{
	fprintf(fLog, "ExitFontState\n");
	// nothing to do
}


// --------------------------------------------------
void
PDFWriter::SetOrigin(BPoint pt)
{
	fprintf(fLog, "SetOrigin pt=[%f, %f]\n",
			pt.x, pt.y);

	// XXX scale pt with current scaling factor or with
	//     scaling factor of previous state? (fState->prev->scale)			
	fState->x0 = fState->prev->x0 + fState->scale*pt.x;
	fState->y0 = fState->prev->y0 + fState->scale*pt.y;
}


// --------------------------------------------------
void	
PDFWriter::SetPenLocation(BPoint pt)
{
	fprintf(fLog, "SetPenLocation pt=[%f, %f]\n",
			pt.x, pt.y);
		
	fState->penX = pt.x;
	fState->penY = pt.y;
}



// --------------------------------------------------
void
PDFWriter::SetDrawingMode(drawing_mode mode)
{
	fprintf(fLog, "SetDrawingMode mode=%d\n", mode);
	fState->drawingMode = mode;
}



// --------------------------------------------------
void
PDFWriter::SetLineMode(cap_mode capMode, join_mode joinMode, float miterLimit)
{
	fprintf(fLog, "SetLineMode\n");
	fState->capMode    = capMode;
	fState->joinMode   = joinMode;
	fState->miterLimit = miterLimit;
	if (!MakesPDF()) return;
	int m = 0;
	switch (capMode) {
		case B_BUTT_CAP:   m = 0; break;
		case B_ROUND_CAP:  m = 1; break;
		case B_SQUARE_CAP: m = 2; break;
	}
	PDF_setlinecap(fPdf, m);
	
	m = 0;
	switch (joinMode) {
		case B_BUTT_JOIN: // fall through XXX: check this; no equivalent in PDF?
		case B_MITER_JOIN: m = 0; break;
		case B_ROUND_JOIN: m = 1; break;
		case B_SQUARE_JOIN: // fall through XXX: check this too
		case B_BEVEL_JOIN: m = 2; break;
	}
	PDF_setlinejoin(fPdf, m);

	PDF_setmiterlimit(fPdf, miterLimit);
	
}


// --------------------------------------------------
void
PDFWriter::SetPenSize(float size)
{
	fprintf(fLog, "SetPenSize size=%f\n", size);
	if (size <= 0.00001) size = 1;
	// XXX scaling required?
	fState->penSize = scale(size);
	if (!MakesPDF()) return;
	PDF_setlinewidth(fPdf, size);
}


// --------------------------------------------------
void
PDFWriter::SetForeColor(rgb_color color)
{
	float red, green, blue;
	
	red 	= color.red / 255.0;
	green 	= color.green / 255.0;
	blue 	= color.blue / 255.0;

	fprintf(fLog, "SetForColor color=[%d, %d, %d, %d] -> [%f, %f, %f]\n",
			color.red, color.green, color.blue, color.alpha,
			red, green, blue);
			
	fState->foregroundColor = color;
}


// --------------------------------------------------
void
PDFWriter::SetBackColor(rgb_color color)
{
	float red, green, blue;
	
	red 	= color.red / 255.0;
	green 	= color.green / 255.0;
	blue 	= color.blue / 255.0;

	fprintf(fLog, "SetBackColor color=[%d, %d, %d, %d] -> [%f, %f, %f]\n",
			color.red, color.green, color.blue, color.alpha,
			red, green, blue);
			
	fState->backgroundColor = color;
}


// --------------------------------------------------
void
PDFWriter::SetStipplePattern(pattern pat)
{
	fprintf(fLog, "SetStipplePattern\n");
	fState->pattern = pat;
}


// --------------------------------------------------
void
PDFWriter::SetScale(float scale)
{
	fprintf(fLog, "SetScale scale=%f\n", scale);
	fState->scale = scale * fState->prev->scale;
}


// --------------------------------------------------
void
PDFWriter::SetFontFamily(char *family)
{
	fprintf(fLog, "SetFontFamily family=\"%s\"\n", family);
	
	fState->fontChanged = true;
	fState->beFont.SetFamilyAndStyle(family, NULL);
}


// --------------------------------------------------
void
PDFWriter::SetFontStyle(char *style)
{
	fprintf(fLog, "SetFontStyle style=\"%s\"\n", style);

	fState->fontChanged = true;
	fState->beFont.SetFamilyAndStyle(NULL, style);
}


// --------------------------------------------------
void
PDFWriter::SetFontSpacing(int32 spacing)
{
	fprintf(fLog, "SetFontSpacing spacing=%ld\n", spacing);
	// XXX scaling required?
	// if it is, do it when the font is used...
	fState->beFont.SetSpacing(spacing);
}


// --------------------------------------------------
void
PDFWriter::SetFontSize(float size)
{
	fprintf(fLog, "SetFontSize size=%f\n", size);
	
	fState->beFont.SetSize(size);
}


// --------------------------------------------------
void
PDFWriter::SetFontRotate(float rotation)
{
	fprintf(fLog, "SetFontRotate rotation=%f\n", rotation);
	fState->beFont.SetRotation(RAD2DEGREE(rotation));
}


// --------------------------------------------------
void
PDFWriter::SetFontEncoding(int32 encoding)
{
	fprintf(fLog, "SetFontEncoding encoding=%ld\n", encoding);
	fState->beFont.SetEncoding(encoding);
}


// --------------------------------------------------
void
PDFWriter::SetFontFlags(int32 flags)
{
	fprintf(fLog, "SetFontFlags flags=%ld (0x%lx)\n", flags, flags);
	fState->beFont.SetFlags(flags);
}


// --------------------------------------------------
void
PDFWriter::SetFontShear(float shear)
{
	fprintf(fLog, "SetFontShear shear=%f\n", shear);
	fState->beFont.SetShear(shear);
}


// --------------------------------------------------
void
PDFWriter::SetFontFace(int32 flags)
{
	fprintf(fLog, "SetFontFace flags=%ld (0x%lx)\n", flags, flags);
//	fState->beFont.SetFace(flags);
//	fState->fontChanged = true;
}

#ifdef CODEWARRIOR
	#pragma mark [Redirectors to instance callbacks/handlers]
#endif


// --------------------------------------------------
size_t	
_WriteData(PDF *pdf, void *data, size_t size)
{ 
	return ((PDFWriter *) PDF_get_opaque(pdf))->WriteData(data, size); 
}


// --------------------------------------------------
void
_ErrorHandler(PDF *pdf, int type, const char *msg)
{
	return ((PDFWriter *) PDF_get_opaque(pdf))->ErrorHandler(type, msg);
}

