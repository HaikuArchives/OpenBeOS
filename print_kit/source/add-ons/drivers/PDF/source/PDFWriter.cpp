/*

PDF Writer printer driver.

Copyright (c) 2001 OpenBeOS. 

Authors: 
	Philippe Houdoin
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
#include "pdflib.h"

#define fmin(x, y) ( (x < y) ? x : y);


// Private Variables
// -----------------

static void *
playbackHandlers[] = {
		_op0,					// 0	no operation
		_MovePenBy,				// 1	MovePenBy(void *user, BPoint delta)
		_StrokeLine,			// 2	StrokeLine(void *user, BPoint start, BPoint end)
		_StrokeRect,			// 3	StrokeRect(void *user, BRect rect)
		_FillRect,				// 4	FillRect(void *user, BRect rect)
		_StrokeRoundRect,		// 5	StrokeRoundRect(void *user, BRect rect, BPoint radii)
		_FillRoundRect,			// 6	FillRoundRect(void *user, BRect rect, BPoint radii)
		_StrokeBezier,			// 7	StrokeBezier(void *user, BPoint *control)
		_FillBezier,			// 8	FillBezier(void *user, BPoint *control)
		_StrokeArc,				// 9	StrokeArc(void *user, BPoint center, BPoint radii, float startTheta, float arcTheta)
		_FillArc,				// 10	FillArc(void *user, BPoint center, BPoint radii, float startTheta, float arcTheta)
		_StrokeEllipse,			// 11	StrokeEllipse(void *user, BPoint center, BPoint radii)
		_FillEllipse,			// 12	FillEllipse(void *user, BPoint center, BPoint radii)
		_StrokePolygon,			// 13	StrokePolygon(void *user, int32 numPoints, BPoint *points, bool isClosed)
		_FillPolygon,			// 14	FillPolygon(void *user, int32 numPoints, BPoint *points, bool isClosed)
		_StrokeShape,			// 15	StrokeShape(void *user, BShape *shape)
		_FillShape,				// 16	FillShape(void *user, BShape *shape)
		_DrawString,			// 17	DrawString(void *user, char *string, float deltax, float deltay)
		_DrawPixels,			// 18	DrawPixels(void *user, BRect src, BRect dest, int32 width, int32 height, int32 bytesPerRow, int32 pixelFormat, int32 flags, void *data)
		_op19,					// 19	*reserved*
		_SetClippingRects,		// 20	SetClippingRects(void *user, BRect *rects, uint32 numRects)
		_ClipToPicture,			// 21	ClipToPicture(void *user, BPicture *picture, BPoint pt, uint32 unknown)
		_PushState,				// 22	PushState(void *user)
		_PopState,				// 23	PopState(void *user)
		_EnterStateChange,		// 24	EnterStateChange(void *user)
		_ExitStateChange,		// 25	ExitStateChange(void *user)
		_EnterFontState,		// 26	EnterFontState(void *user)
		_ExitFontState,			// 27	ExitFontState(void *user)
		_SetOrigin,				// 28	SetOrigin(void *user, BPoint pt)
		_SetPenLocation,		// 29	SetPenLocation(void *user, BPoint pt)
		_SetDrawingMode,		// 30	SetDrawingMode(void *user, drawing_mode mode)
		_SetLineMode,			// 31	SetLineMode(void *user, cap_mode capMode, join_mode joinMode, float miterLimit)
		_SetPenSize,			// 32	SetPenSize(void *user, float size)
		_SetForeColor,			// 33	SetForeColor(void *user, rgb_color color)
		_SetBackColor,			// 34	SetBackColor(void *user, rgb_color color)
		_SetStipplePattern,		// 35	SetStipplePattern(void *user, pattern p)
		_SetScale,				// 36	SetScale(void *user, float scale)
		_SetFontFamily,			// 37	SetFontFamily(void *user, char *family)
		_SetFontStyle,			// 38	SetFontStyle(void *user, char *style)
		_SetFontSpacing,		// 39	SetFontSpacing(void *user, int32 spacing)
		_SetFontSize,			// 40	SetFontSize(void *user, float size)
		_SetFontRotate,			// 41	SetFontRotate(void *user, float rotation)
		_SetFontEncoding,		// 42	SetFontEncoding(void *user, int32 encoding)
		_SetFontFlags,			// 43	SetFontFlags(void *user, int32 flags)
		_SetFontShear,			// 44	SetFontShear(void *user, float shear)
		_op45,					// 45	*reserved*
		_SetFontFace,			// 46	SetFontFace(void *user, int32 flags)
		_op47,
		_op48,
		_op49,

		NULL
	}; 


// Constructor & destructor
// ------------------------

PDFWriter::PDFWriter()
	:	PrinterDriver()
{
}

PDFWriter::~PDFWriter()
{
	const int n = fFontCache.CountItems();
	for (int i = 0; i < n; i++) {
		delete (Font*)fFontCache.ItemAt(i);
	}
	fFontCache.MakeEmpty();
	
	if (Transport())
		CloseTransport();
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
	BRect 		printRect;
	BRect		*picRects;
	BPoint		*picPoints;
	BRegion		*picRegion;
	BPicture	**pictures;
	uint32		i;
	int32       orientation;
	

	status = B_OK;
	
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

	if (pageNumber == 1) {
		fLog = fopen("/boot/home/Desktop/pdf_writer.log", "w");

		PDF_boot();

		fPdf = PDF_new2(_ErrorHandler, NULL, NULL, NULL, this);	// set *this* as pdf cookie
		if ( fPdf == NULL )
			return B_ERROR;
		
		PDF_open_mem(fPdf, _WriteData);	// use callback to stream PDF document data to printer transport
		InitWriter();
	}
	
	BeginPage(printRect);
	for (i = 0; i < pictureCount; i++) {
		pictures[i]->Play(playbackHandlers, 50, this);
		delete pictures[i];
	}
	EndPage();
	
	free(pictures);
	free(picRects);
	free(picPoints);
	
	if (pageNumber == pageCount) {
		PDF_close(fPdf);
		fprintf(fLog, ">>>> PDF_close\n");

    	PDF_delete(fPdf);
	    PDF_shutdown();

		fclose(fLog);
	}

	return status;
}


// --------------------------------------------------
status_t 
PDFWriter::InitWriter()
{
	char	buffer[512];

	fprintf(fLog, ">>>> PDF_open_mem\n");
		
	PDF_set_parameter(fPdf, "flush", "heavy");
		
	// find job title 
	if (JobFile()->ReadAttr("_spool/Description", B_STRING_TYPE, 0, buffer, sizeof(buffer)))
	    PDF_set_info(fPdf, "Title", buffer);
			
	// find job creator
	if (JobFile()->ReadAttr("_spool/MimeType", B_STRING_TYPE, 0, buffer, sizeof(buffer)))
	    PDF_set_info(fPdf, "Creator", buffer);

	const char * compatibility;
	if (JobMsg()->FindString("pdf_compatibility", &compatibility) == B_OK)
		PDF_set_parameter(fPdf, "compatibility", compatibility);
		
	int32 compression;
	if (JobMsg()->FindInt32("pdf_compression", &compression) == B_OK)
	    PDF_set_value(fPdf, "compress", compression);

    // PDF_set_parameter(fPdf, "warning", "false");

	PDF_set_parameter(fPdf, "fontwarning", "false");
	// PDF_set_parameter(fPdf, "native-unicode", "true");

	PDF_set_parameter(fPdf, "resourcefile", "/boot/home/config/settings/pdflib.upr");
/*
	PDF_set_parameter(fPdf, "FontOutline", "Swis721 BT-Roman==/boot/beos/etc/fonts/ttfonts/Swiss721.ttf");
	PDF_set_parameter(fPdf, "FontOutline", "Swis721 BT-Bold==/boot/beos/etc/fonts/ttfonts/Swiss721_Bold.ttf");
	PDF_set_parameter(fPdf, "FontOutline", "Swis721 BT-Italic==/boot/beos/etc/fonts/ttfonts/Swiss721_Italic.ttf");
	PDF_set_parameter(fPdf, "FontOutline", "Swis721 BT-Bold Italic==/boot/beos/etc/fonts/ttfonts/Swiss721_BoldItalic.ttf");
*/
	fState = NULL;
	fStateDepth = 0;

	return B_OK;
}


// --------------------------------------------------
status_t 
PDFWriter::BeginPage(BRect printRect)
{
	float width = printRect.Width() < 10 ? a4_width : printRect.Width();
	float height = printRect.Height() < 10 ? a4_height : printRect.Height();
	
	fMode = kDrawingMode;
	
	ASSERT(fState == NULL);
	fState = new State();
	fState->height = height;
    PDF_begin_page(fPdf, width, fState->height);

	fprintf(fLog, ">>>> PDF_begin_page [%f, %f]\n", width, fState->height);
	
	PDF_initgraphics(fPdf);
	
	fState->fontChanged 	= true;

	fState->x0 = fState->penX = 0;
	fState->y0 = fState->penY = 0;

	PushState(); // so that fState->prev != NULL

	return B_OK;
}


// --------------------------------------------------
status_t 
PDFWriter::EndPage()
{
	while (fState->prev != NULL) PopState();

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
PDFWriter::SetColor(rgb_color color) 
{
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
void 
PDFWriter::StrokeOrClip() {
	if (IsDrawing()) {
		PDF_stroke(fPdf);
	} else {
		PDF_clip(fPdf);
	}
}

// --------------------------------------------------
void 
PDFWriter::FillOrClip() {
	if (IsDrawing()) {
		PDF_fill(fPdf);
	} else {
		PDF_clip(fPdf);
	}
}

// --------------------------------------------------
static bool 
IsSame(pattern p1, pattern p2) {
	char *a = (char*)p1.data;
	char *b = (char*)p2.data;
	return strncmp(a, b, 8) == 0;
}

// --------------------------------------------------
void 
PDFWriter::SetColor() {
	if (IsSame(fState->pattern, B_SOLID_HIGH)) {
		SetColor(fState->foregroundColor);
	} else if (IsSame(fState->pattern, B_SOLID_LOW)) {
		SetColor(fState->backgroundColor);
	} else if (IsSame(fState->pattern, B_MIXED_COLORS)) {
		rgb_color mixed; // XXX
		mixed.red    = (fState->foregroundColor.red + fState->backgroundColor.red) / 2; 
		mixed.green  = (fState->foregroundColor.green + fState->backgroundColor.green) / 2; 
		mixed.blue   = (fState->foregroundColor.blue + fState->backgroundColor.blue) / 2; 
		mixed.alpha  = (fState->foregroundColor.alpha + fState->backgroundColor.alpha) / 2; 
		SetColor(mixed);
	} else {
		SetColor(fState->foregroundColor);
	}
}


#ifdef CODEWARRIOR
	#pragma mark [Image drawing support routines]
#endif

// --------------------------------------------------
void *
PDFWriter::CreateMask(BRect src, int32 bytesPerRow, int32 pixelFormat, int32 flags, void *data)
{
	uint8	*in;
	int32	x, y;
	
	uint8	*mask;
	uint8	*maskRow;
	uint8	*out;
	int32	maskWidth;
	uint8	shift;
	bool	alpha;
	rgb_color	transcolor = B_TRANSPARENT_COLOR;

	int32	width = src.IntegerWidth() + 1;
	int32	height = src.IntegerHeight() + 1;
		
	if (pixelFormat != B_RGB32 &&
		pixelFormat != B_RGBA32)
		return NULL;

	// Image Mask
	maskWidth = (width+7)/8;
	maskRow = mask = new uint8[maskWidth * height];
	memset(mask, 0, maskWidth*height);
	alpha = false;
	
	for (y = (int) src.top; y <= (int) src.bottom; y++) {
		in = (uint8 *) data;
		in += y * bytesPerRow;
		in += 4 * (int) src.left;

		out = maskRow;
		shift = 7;
		
		for (x = (int) src.left; x <= (int) src.right; x++ )
			{
//			fprintf(fLog, "(%d, %d) %d %d %d\n", x, y, (mout - mask), (int)shift, (int)*(in+3));
			// For each pixel
			
			if ((pixelFormat == B_RGBA32 && in[3] < 128) ||
			    (pixelFormat == B_RGB32 && in[0] == transcolor.blue &&
			    in[1] == transcolor.green && in[2] == transcolor.red &&
			    in[3] == transcolor.alpha) ) {
				out[0] |= (1 << shift);
				alpha = true;
			}			
			if (shift == 0) out ++;
			shift = (shift + 7) & 0x07;		
			in += 4;	// next pixel
			};

		// next row
		maskRow += maskWidth;
	}
	
	if (!alpha) {
		delete []mask;
		mask = NULL;
	}
	return mask;
}

// --------------------------------------------------
BBitmap *
PDFWriter::ConvertBitmap(BRect src, int32 bytesPerRow, int32 pixelFormat, int32 flags, void *data)
{
	void	*buffer;
	uint8	*in;
	uint8	*out;
	int32	x, y;
	
	if (pixelFormat != B_RGB32)
		return NULL;

	BBitmap *	bm = new BBitmap(BRect(0, 0, src.Width(), src.Height()), B_RGB32);
	if (!bm->IsValid()) {
		delete bm;
		fprintf(fLog, "BBitmap constructor failed\n");
		return NULL;
	}
	// Reused same buffer area
	buffer 	= bm->Bits();

	for (y = (int) src.top; y <= (int) src.bottom; y++) {
		in = (uint8 *) data;
		in += y * bytesPerRow;
		in += 4 * (int) src.left;

		out = (uint8 *) buffer;
		out += y * bm->BytesPerRow();

		for (x = (int) src.left; x <= (int) src.right; x++ )
			{
//			fprintf(fLog, "(%d, %d) %d %d %d\n", x, y, (mout - mask), (int)shift, (int)*(in+3));
			// For each pixel
			*((rgb_color*)out) = *((rgb_color*)in);
			in += 4;	// next pixel
			out += 4;
			};

		// next row
	}
	
	return bm;
}

// --------------------------------------------------
bool 
PDFWriter::StoreTranslatorBitmap(BBitmap *bitmap, char *filename, uint32 type)
{
	BTranslatorRoster *roster = BTranslatorRoster::Default();
	if (!roster) {
		fprintf(fLog, "roster == NULL!\n");
		return false;
	}
	BBitmapStream stream(bitmap); // init with contents of bitmap
	BFile file(filename, B_CREATE_FILE | B_WRITE_ONLY | B_ERASE_FILE);
	bool res = roster->Translate(&stream, NULL, NULL, &file, type) == B_OK;
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
{
}

// --------------------------------------------------
status_t 
DrawShape::IterateBezierTo(int32 bezierCount, BPoint *control)
{
	for (int32 i = 0; i < bezierCount; i++, control += 3) {
		PDF_curveto(Pdf(), 
			tx(control[0].x), ty(control[0].y),
			tx(control[1].x), ty(control[1].y),
	    	tx(control[2].x), ty(control[2].y));
	}
	return B_OK;
}

// --------------------------------------------------
status_t 
DrawShape::IterateClose(void)
{
	PDF_closepath(Pdf());
	if (IsDrawing()) {
		if (fStroke) 
			PDF_stroke(Pdf()); 
		else {
			PDF_fill(Pdf());
		}
	} else {
		PDF_clip(Pdf());
	}
	return B_OK;
}

// --------------------------------------------------
status_t 
DrawShape::IterateLineTo(int32 lineCount, BPoint *linePoints)
{
	BPoint *p = linePoints;
	for (int32 i = 0; i < lineCount; i++) {
		PDF_lineto(Pdf(), tx(p->x), ty(p->y)); p++;
		PDF_lineto(Pdf(), tx(p->x), ty(p->y)); p++;
	}
	return B_OK;
}

// --------------------------------------------------
status_t 
DrawShape::IterateMoveTo(BPoint *point)
{
	PDF_moveto(Pdf(), tx(point->x), ty(point->y)); 
	return B_OK;
}

#ifdef CODEWARRIOR
	#pragma mark -- BPicture playback handlers
#endif

// BPicture::Play() handlers

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
	PDF_moveto(fPdf, tx(start.x), ty(start.y));
	PDF_lineto(fPdf, tx(end.x),   ty(end.y));
	StrokeOrClip();
}


// --------------------------------------------------
void PDFWriter::StrokeRect(BRect rect)
{
	fprintf(fLog, "StrokeRect rect=[%f, %f, %f, %f]\n",
			rect.left, rect.top, rect.right, rect.bottom);

	SetColor();			
	PDF_rect(fPdf, tx(rect.left), ty(rect.bottom), scale(rect.Width()), scale(rect.Height()));
	StrokeOrClip();

}


// --------------------------------------------------
void	
PDFWriter::FillRect(BRect rect)
{
	fprintf(fLog, "FillRect rect=[%f, %f, %f, %f]\n",
			rect.left, rect.top, rect.right, rect.bottom);

	SetColor();			
	PDF_rect(fPdf, tx(rect.left), ty(rect.bottom), scale(rect.Width()), scale(rect.Height()));
	FillOrClip();

}


// --------------------------------------------------
void	
PDFWriter::StrokeRoundRect(BRect rect, BPoint radii)
{
	fprintf(fLog, "StrokeRoundRect center=[%f, %f, %f, %f], radii=[%f, %f]\n",
			rect.left, rect.top, rect.right, rect.bottom, radii.x, radii.y);
	StrokeRect(rect);
}


// --------------------------------------------------
void	
PDFWriter::FillRoundRect(BRect rect, BPoint	radii)
{
	fprintf(fLog, "FillRoundRect rect=[%f, %f, %f, %f], radii=[%f, %f]\n",
			rect.left, rect.top, rect.right, rect.bottom, radii.x, radii.y);
	FillRect(rect);
}


// --------------------------------------------------
void	
PDFWriter::StrokeBezier(BPoint	*control)
{
	fprintf(fLog, "StrokeBezier\n");
	SetColor();
	PDF_moveto(fPdf, tx(control[0].x), ty(control[0].y));
	PDF_curveto(fPdf, tx(control[1].x), ty(control[1].y),
	            tx(control[2].x), ty(control[2].y),
	            tx(control[3].x), ty(control[3].y));
	StrokeOrClip();
}


// --------------------------------------------------
void	
PDFWriter::FillBezier(BPoint *control)
{
	fprintf(fLog, "FillBezier\n");
	SetColor();
	PDF_moveto(fPdf, tx(control[0].x), ty(control[0].y));
	PDF_curveto(fPdf, tx(control[1].x), ty(control[1].y),
	            tx(control[2].x), ty(control[2].y),
	            tx(control[3].x), ty(control[3].y));
	PDF_closepath(fPdf);
	FillOrClip();
}


// --------------------------------------------------
void
PDFWriter::StrokeArc(BPoint center, BPoint radii, float startTheta, float arcTheta)
{
	fprintf(fLog, "StrokeArc center=[%f, %f], radii=[%f, %f], startTheta=%f, arcTheta=%f\n",
			center.x, center.y, radii.x, radii.y, startTheta, arcTheta);
	float r = fmin(radii.x, radii.y);
	SetColor();
	PDF_arc(fPdf, tx(center.x), ty(center.y), scale(r), startTheta, arcTheta);
	StrokeOrClip();
}


// --------------------------------------------------
void 
PDFWriter::FillArc(BPoint center, BPoint radii, float startTheta, float arcTheta)
{
	fprintf(fLog, "FillArc center=[%f, %f], radii=[%f, %f], startTheta=%f, arcTheta=%f\n",
			center.x, center.y, radii.x, radii.y, startTheta, arcTheta);
	float r = fmin(radii.x, radii.y);
	SetColor();
	PDF_arc(fPdf, tx(center.x), ty(center.y), scale(r), startTheta, arcTheta);
	PDF_closepath(fPdf);
	FillOrClip();
}


// --------------------------------------------------
void
PDFWriter::StrokeEllipse(BPoint center, BPoint radii)
{
	fprintf(fLog, "StrokeEllipse center=[%f, %f], radii=[%f, %f]\n",
			center.x, center.y, radii.x, radii.y);
	float r = fmin(radii.x, radii.y);
	SetColor();
	PDF_circle(fPdf, tx(center.x), ty(center.y), scale(r));
	StrokeOrClip();
}


// --------------------------------------------------
void
PDFWriter::FillEllipse(BPoint center, BPoint radii)
{
	fprintf(fLog, "FillEllipse center=[%f, %f], radii=[%f, %f]\n",
			center.x, center.y, radii.x, radii.y);
	float r = fmin(radii.x, radii.y);
	SetColor();
	PDF_circle(fPdf, tx(center.x), ty(center.y), scale(r));
	FillOrClip();
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
	DrawShape iterator(this, true);
	iterator.Iterate(shape);
}


// --------------------------------------------------
void PDFWriter::FillShape(BShape *shape)
{
	fprintf(fLog, "FillShape\n");
	SetColor();			
	DrawShape iterator(this, false);
	iterator.Iterate(shape);
}


// --------------------------------------------------
void
PDFWriter::ClipToPicture(BPicture *picture, BPoint point, uint32 unknown)
{
	fprintf(fLog, "ClipToPicture at (%f, %f) unknown = %ld\n", point.x, point.y, unknown);
	if (fMode == kDrawingMode) {
		fMode = kClippingMode;
		picture->Play(playbackHandlers, 50, this);
		fMode = kDrawingMode;
	} else {
		fprintf(fLog, "Nested call of ClipToPicture not implemented yet!\n");
	}
}

#if 0
// --- String Handling ----
struct FontSubst {
	char *pdfFont, 
	     *fontFamily, 
	     *fontStyle;
};

// index: {symbolic:12, fixed:8, serif:4, sans-serif:0} + bold*2 + italic
static FontSubst fontSubst[16] = {
 { "Helvetica",				"Swis721 BT",			"Roman"},
 { "Helvetica-Oblique",		"Swis721 BT",			"Italic"},
 { "Helvetica-Bold",		"Swis721 BT",			"Bold"},
 { "Helvetica-BoldOblique",	"Swis721 BT",			"Bold Italic"},
 { "Times-Roman",			"Dutch801 Rm BT",		"Roman"},
 { "Times-Italic",			"Dutch801 Rm BT",		"Italic"},
 { "Times-Bold",			"Dutch801 Rm BT",		"Bold"},
 { "Times-BoldItalic",		"Dutch801 Rm BT",		"Bold Italic"},
 { "Courier",				"Courier10 BT",			"Roman"},
 { "Courier-Oblique",		"Courier10 BT",			"Italic"},
 { "Courier-Bold",			"Courier10 BT",			"Bold"},
 { "Courier-BoldOblique",	"Courier10 BT",			"Bold Italic"},
 { "Symbol",				"SymbolProp BT",		"Regular"},
 { "Symbol",				"SymbolProp BT",		"Regular"},
 { "Symbol",				"SymbolProp BT",		"Regular"},
 { "Symbol",				"SymbolProp BT",		"Regular"}
};
#endif

void 
PDFWriter::GetFontName(BFont *font, char *fontname, int *embed) 
{
	font_family family;
	font_style  style;

	font->GetFamilyAndStyle(&family, &style);
#if 0
	for (int i = 0; i < 16; i++) {
		if (strcmp(fontSubst[i].fontFamily, family) == 0 &&
		    strcmp(fontSubst[i].fontStyle, style) == 0) {
		    strcpy(fontname, fontSubst[i].pdfFont);
			*embed = 0;
		    return;
		}
	}
	
	// substitute font by face description
/*
	uint16 face  = font->Face();
	uint8 italic = (face & B_ITALIC_FACE) != 0 ? 1 : 0;
	uint8 bold   = (face & B_BOLD_FACE) != 0 ? 2 : 0;
	uint8 fixed  = font->IsFixed() ? 8 : 0;
	strcpy(fontname, fontSubst[fixed+italic+bold].pdfFont);	 
*/
#endif

	*embed = 1;
	strcat(strcat(strcpy(fontname, family), "-"), style);
}

int 
PDFWriter::FindFont(char* fontName, int embed) 
{
	Font *cache = NULL;
	const int n = fFontCache.CountItems();
	int i;
	for (i = 0; i < n; i++) {
		cache = (Font*)fFontCache.ItemAt(i);
		if (strcmp(cache->name.String(), fontName) == 0) return cache->font;
	}
	int font = PDF_findfont(fPdf, fontName, "macroman", embed);
	if (font != -1) {
		cache = new Font(fontName, font);
		fFontCache.AddItem(cache);
	}
	return font;
}

// --------------------------------------------------
void	
PDFWriter::DrawString(char *string, float deltax, float deltay)
{
	int32	srcLen;
	int32	destLen;
	char *	dest;
	int32	state;

	fprintf(fLog, "DrawString string=\"%s\", deltax=%f, deltay=%f, at %f, %f\n",
			string, deltax, deltay, fState->penX, fState->penY);

	if (IsClipping()) {
		fprintf(fLog, "DrawPixels for clipping not implemented yet!");
		return;
	}

	if (fState->fontChanged) {
		char 	fontName[B_FONT_FAMILY_LENGTH+B_FONT_STYLE_LENGTH+1];
		int		font;
		int     embed;

		GetFontName(&fState->beFont, fontName, &embed);
		font = FindFont(fontName, embed);	
		if (font < 0) {
			fprintf(fLog, "**** PDF_findfont(%s) failed, back to default font\n", fontName);
			font = PDF_findfont(fPdf, "Helvetica", "host", 0);
		}

		fState->font = font;
		fState->fontChanged = false;

		uint16 face = fState->beFont.Face();
		PDF_set_parameter(fPdf, "underline", (face & B_UNDERSCORE_FACE) != 0 ? "true" : "false");
		PDF_set_parameter(fPdf, "strikeout", (face & B_STRIKEOUT_FACE) != 0 ? "true" : "false");
		PDF_set_value(fPdf, "textrendering", (face & B_OUTLINED_FACE) != 0 ? 1 : 0); 
	}

	SetColor();
	// XXX: scaling font size required?
	PDF_setfont(fPdf, fState->font, scale(fState->beFont.Size()));

	const float x = tx(fState->penX + deltax);
	const float y = ty(fState->penY + deltay);
	const float rotation = fState->beFont.Rotation();
	const bool rotate = rotation != 0.0;

	if (rotate) {
		PDF_save(fPdf);
		PDF_translate(fPdf, x, y);
		PDF_rotate(fPdf, 180.0 / PI * rotation);
	    PDF_set_text_pos(fPdf, 0, 0);
	} else 
	    PDF_set_text_pos(fPdf, x, y);

	// try to convert from utf8 in the font encoding schema...
	srcLen = strlen(string);
	destLen = srcLen * 3;
	dest = (char *) malloc(destLen);
	state = 0;
	
	float w ; 
	// how is string encoded?
	// if it is fState->beFont->Encoding then string has to be converted to utf8 first and then to macroman!
	if (convert_from_utf8(B_MAC_ROMAN_CONVERSION, string, &srcLen, dest, &destLen, &state) == B_OK) {
		PDF_show2(fPdf, dest, destLen);
	} else {
		// conversion failed, send unconverted string...
		PDF_show(fPdf, string);
	}
	w = fState->beFont.StringWidth(string);

	if (rotate) {
		PDF_restore(fPdf);
		fState->penX += w*cos(rotation);
		fState->penY += w*sin(rotation);
	} else {
		fState->penX += w;
		fState->penY += 0;
	}
			
	free(dest);
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

	if (IsClipping()) {
		fprintf(fLog, "DrawPixels for clipping not implemented yet!");
		return;
	}

	mask = CreateMask(src, bytesPerRow, pixelFormat, flags, data);

	float scaleX = (dest.right  - dest.left+1) / (src.right  - src.left+1);
	float scaleY = (dest.bottom - dest.top+1) / (src.bottom - src.top+1);

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
	
	if (!StoreTranslatorBitmap(bm, "/tmp/pdfwriter.png", 'PNG ')) {
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

	image = PDF_open_image_file(fPdf, "png", "/tmp/pdfwriter.png", 
		maskId == -1 ? "" : "masked", maskId == -1 ? 0 : maskId);
	if ( image >= 0 ) {
		PDF_place_image(fPdf, image, x, y, 1.0);
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
	fState = new State(fState); fStateDepth ++;
	fprintf(fLog, "height = %f x0 = %f y0 = %f\n", fState->height, fState->x0, fState->y0);
	PDF_save(fPdf);
}


// --------------------------------------------------
void
PDFWriter::PopState()
{
	fprintf(fLog, "PopState\n");
	if (fStateDepth != 0) {
		State* s = fState; fStateDepth --;
		fState = fState->prev;
		delete s;
		fprintf(fLog, "height = %f x0 = %f y0 = %f\n", fState->height, fState->x0, fState->y0);
		PDF_restore(fPdf);
	} else {
		fprintf(fLog, "State stack underflow!\n");
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
	PDF_setlinewidth(fPdf, size);
	// XXX scaling required?
	fState->penSize = scale(size);
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
	// XXX don't know how to convert this to PDF drawing primitives
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
	fState->beFont.SetRotation(rotation);
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
	fState->beFont.SetFace(flags);
	fState->fontChanged = true;
}

#ifdef CODEWARRIOR
	#pragma mark [Redirectors to instance callbacks/handlers]
#endif


size_t	_WriteData(PDF *pdf, void *data, size_t size)			{ return ((PDFWriter *) PDF_get_opaque(pdf))->WriteData(data, size); }
void	_ErrorHandler(PDF *pdf, int type, const char *msg)	{ return ((PDFWriter *) PDF_get_opaque(pdf))->ErrorHandler(type, msg); }

// BPicture playback handlers class instance redirectors
void	_MovePenBy(void *p, BPoint delta) 														{ return ((PDFWriter *) p)->MovePenBy(delta); }
void	_StrokeLine(void *p, BPoint start, BPoint end) 										{ return ((PDFWriter *) p)->StrokeLine(start, end); }
void	_StrokeRect(void *p, BRect rect) 														{ return ((PDFWriter *) p)->StrokeRect(rect); }
void	_FillRect(void *p, BRect rect) 														{ return ((PDFWriter *) p)->FillRect(rect); }
void	_StrokeRoundRect(void *p, BRect rect, BPoint radii) 									{ return ((PDFWriter *) p)->StrokeRoundRect(rect, radii); }
void	_FillRoundRect(void *p, BRect rect, BPoint radii)  									{ return ((PDFWriter *) p)->FillRoundRect(rect, radii); }
void	_StrokeBezier(void *p, BPoint *control)  												{ return ((PDFWriter *) p)->StrokeBezier(control); }
void	_FillBezier(void *p, BPoint *control)  												{ return ((PDFWriter *) p)->FillBezier(control); }
void	_StrokeArc(void *p, BPoint center, BPoint radii, float startTheta, float arcTheta)		{ return ((PDFWriter *) p)->StrokeArc(center, radii, startTheta, arcTheta); }
void	_FillArc(void *p, BPoint center, BPoint radii, float startTheta, float arcTheta)		{ return ((PDFWriter *) p)->FillArc(center, radii, startTheta, arcTheta); }
void	_StrokeEllipse(void *p, BPoint center, BPoint radii)									{ return ((PDFWriter *) p)->StrokeEllipse(center, radii); }
void	_FillEllipse(void *p, BPoint center, BPoint radii)										{ return ((PDFWriter *) p)->FillEllipse(center, radii); }
void	_StrokePolygon(void *p, int32 numPoints, BPoint *points, bool isClosed) 				{ return ((PDFWriter *) p)->StrokePolygon(numPoints, points, isClosed); }
void	_FillPolygon(void *p, int32 numPoints, BPoint *points, bool isClosed)					{ return ((PDFWriter *) p)->FillPolygon(numPoints, points, isClosed); }
void	_StrokeShape(void * p, BShape *shape)													{ return ((PDFWriter *) p)->StrokeShape(shape); }
void	_FillShape(void * p, BShape *shape)														{ return ((PDFWriter *) p)->FillShape(shape); }
void	_DrawString(void *p, char *string, float deltax, float deltay)							{ return ((PDFWriter *) p)->DrawString(string, deltax, deltay); }
void	_DrawPixels(void *p, BRect src, BRect dest, int32 width, int32 height, int32 bytesPerRow, int32 pixelFormat, int32 flags, void *data)
						{ return ((PDFWriter *) p)->DrawPixels(src, dest, width, height, bytesPerRow, pixelFormat, flags, data); }
void	_SetClippingRects(void *p, BRect *rects, uint32 numRects)								{ return ((PDFWriter *) p)->SetClippingRects(rects, numRects); }
void	_ClipToPicture(void * p, BPicture *picture, BPoint point, uint32 unknown)				{ return ((PDFWriter *) p)->ClipToPicture(picture, point, unknown); }
void	_PushState(void *p)  																	{ return ((PDFWriter *) p)->PushState(); }
void	_PopState(void *p)  																	{ return ((PDFWriter *) p)->PopState(); }
void	_EnterStateChange(void *p) 															{ return ((PDFWriter *) p)->EnterStateChange(); }
void	_ExitStateChange(void *p) 																{ return ((PDFWriter *) p)->ExitStateChange(); }
void	_EnterFontState(void *p) 																{ return ((PDFWriter *) p)->EnterFontState(); }
void	_ExitFontState(void *p) 																{ return ((PDFWriter *) p)->ExitFontState(); }
void	_SetOrigin(void *p, BPoint pt)															{ return ((PDFWriter *) p)->SetOrigin(pt); }
void	_SetPenLocation(void *p, BPoint pt)													{ return ((PDFWriter *) p)->SetPenLocation(pt); }
void	_SetDrawingMode(void *p, drawing_mode mode)											{ return ((PDFWriter *) p)->SetDrawingMode(mode); }
void	_SetLineMode(void *p, cap_mode capMode, join_mode joinMode, float miterLimit)			{ return ((PDFWriter *) p)->SetLineMode(capMode, joinMode, miterLimit); }
void	_SetPenSize(void *p, float size)														{ return ((PDFWriter *) p)->SetPenSize(size); }
void	_SetForeColor(void *p, rgb_color color)												{ return ((PDFWriter *) p)->SetForeColor(color); }
void	_SetBackColor(void *p, rgb_color color)												{ return ((PDFWriter *) p)->SetBackColor(color); }
void	_SetStipplePattern(void *p, pattern pat)												{ return ((PDFWriter *) p)->SetStipplePattern(pat); }
void	_SetScale(void *p, float scale)														{ return ((PDFWriter *) p)->SetScale(scale); }
void	_SetFontFamily(void *p, char *family)													{ return ((PDFWriter *) p)->SetFontFamily(family); }
void	_SetFontStyle(void *p, char *style)													{ return ((PDFWriter *) p)->SetFontStyle(style); }
void	_SetFontSpacing(void *p, int32 spacing)												{ return ((PDFWriter *) p)->SetFontSpacing(spacing); }
void	_SetFontSize(void *p, float size)														{ return ((PDFWriter *) p)->SetFontSize(size); }
void	_SetFontRotate(void *p, float rotation)												{ return ((PDFWriter *) p)->SetFontRotate(rotation); }
void	_SetFontEncoding(void *p, int32 encoding)												{ return ((PDFWriter *) p)->SetFontEncoding(encoding); }
void	_SetFontFlags(void *p, int32 flags)													{ return ((PDFWriter *) p)->SetFontFlags(flags); }
void	_SetFontShear(void *p, float shear)													{ return ((PDFWriter *) p)->SetFontShear(shear); }
void	_SetFontFace(void * p, int32 flags)														{ return ((PDFWriter *) p)->SetFontFace(flags); }

// undefined or undocumented operation handlers...
void	_op0(void * p)	{ return ((PDFWriter *) p)->Op(0); }
void	_op19(void * p)	{ return ((PDFWriter *) p)->Op(19); }
void	_op45(void * p)	{ return ((PDFWriter *) p)->Op(45); }
void	_op47(void * p)	{ return ((PDFWriter *) p)->Op(47); }
void	_op48(void * p)	{ return ((PDFWriter *) p)->Op(48); }
void	_op49(void * p)	{ return ((PDFWriter *) p)->Op(49); }


