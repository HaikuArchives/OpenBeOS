/*

PDF Writer printer driver.

Copyright (c) 2001 OpenBeOS. 

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
#include "pdflib.h"

#define fmin(x, y) ( (x < y) ? x : y);

#define TRUETYPE_VERSION		0x00010000
#define OPENTYPE_CFF_VERSION	'OTTO'

#define TRUETTYPE_TABLE_NAME_TAG	'name'

static uint16 ttf_get_uint16(FILE * ttf);
static uint32 ttf_get_uint32(FILE *ttf);
static status_t ttf_get_fontname(const char * path, char * fontname, size_t fn_size);

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
		_ClipToPicture,			// 21	ClipToPicture(void *user, BPicture *picture, BPoint pt, bool clip_to_inverse_picture)
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
	
	for (int i = 0; i < fFontFiles.CountItems(); i++) {
		delete (FontFile*)fFontFiles.ItemAt(i);
	}
	fFontFiles.MakeEmpty();
	
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
	BRect		paperRect;
	BRect 		printRect;
	BRect		*picRects;
	BPoint		*picPoints;
	BRegion		*picRegion;
	BPicture	**pictures;
	uint32		i;
	int32       orientation;
	

	status = B_OK;
	
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

	if (pageNumber == 1) {
		fLog = fopen("/boot/home/Desktop/pdf_writer.log", "w");

		PDF_boot();

		fPdf = PDF_new2(_ErrorHandler, NULL, NULL, NULL, this);	// set *this* as pdf cookie
		if ( fPdf == NULL )
			return B_ERROR;
		
		InitWriter();
	}
	
	BeginPage(paperRect, printRect);
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

	const char * compatibility;
	if (JobMsg()->FindString("pdf_compatibility", &compatibility) == B_OK) {
		PDF_set_parameter(fPdf, "compatibility", compatibility);
	}

	fprintf(fLog, ">>>> PDF_open_mem\n");	
	PDF_open_mem(fPdf, _WriteData);	// use callback to stream PDF document data to printer transport

	PDF_set_parameter(fPdf, "flush", "heavy");
		
	// find job title 
	if (JobFile()->ReadAttr("_spool/Description", B_STRING_TYPE, 0, buffer, sizeof(buffer)))
	    PDF_set_info(fPdf, "Title", buffer);
			
	// find job creator
	if (JobFile()->ReadAttr("_spool/MimeType", B_STRING_TYPE, 0, buffer, sizeof(buffer)))
	    PDF_set_info(fPdf, "Creator", buffer);

	int32 compression;
	if (JobMsg()->FindInt32("pdf_compression", &compression) == B_OK) {
	    PDF_set_value(fPdf, "compress", compression);
	}

    // PDF_set_parameter(fPdf, "warning", "false");

	PDF_set_parameter(fPdf, "fontwarning", "false");
	// PDF_set_parameter(fPdf, "native-unicode", "true");

	fprintf(fLog, "Start of fonts declaration:\n");	
/*
	PDF_set_parameter(fPdf, "resourcefile", "/boot/home/config/settings/pdflib.upr");
*/
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
PDFWriter::BeginPage(BRect paperRect, BRect printRect)
{
	float width = paperRect.Width() < 10 ? a4_width : paperRect.Width();
	float height = paperRect.Height() < 10 ? a4_height : paperRect.Height();
	
	fMode = kDrawingMode;
	
	ASSERT(fState == NULL);
	fState = new State();
	fState->height = height;
    PDF_begin_page(fPdf, width, fState->height);

	fprintf(fLog, ">>>> PDF_begin_page [%f, %f]\n", width, fState->height);
	
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

    PDF_end_page(fPdf);
	fprintf(fLog, ">>>> PDF_end_page\n");

	delete fState; fState = NULL;
	
	return B_OK;
}


#ifdef CODEWARRIOR
	#pragma mark [TrueType fonts handling]
#endif


// --------------------------------------------------
status_t 
PDFWriter::DeclareFonts()
{
	char 				fn[512];
	char				buffer[1024];
	BDirectory *		dir;
    BEntry 				entry;
	BPath				path;
	directory_which	*	which_dir;
	directory_which		lookup_dirs[] =
		{
		B_BEOS_FONTS_DIRECTORY,
		// B_COMMON_FONTS_DIRECTORY,	// seem to be the same directory than B_USER_FONTS_DIRECTORY!!!
		B_USER_FONTS_DIRECTORY,
		(directory_which) -1
		};

	which_dir = lookup_dirs;
	while (*which_dir >= 0)
		{
		if ( find_directory(*which_dir, &path) == B_OK )
			{		
			path.Append("ttfonts");
			
			dir = new BDirectory(path.Path());
			if ( dir->InitCheck() == B_OK )
				{
				fprintf(fLog, "--- From %s\n", path.Path());
				while ( dir->GetNextEntry(&entry) == B_OK )
					{
					if (! entry.IsFile())
						continue;

					entry.GetPath(&path);
	
					fn[0] = 0;
					if ( ttf_get_fontname(path.Path(), fn, sizeof(fn)) != B_OK )
						continue;
										
					BFile f(&entry, B_READ_ONLY);
					off_t size;
					if (f.GetSize(&size) != B_OK) size = 1024*1024*1024;
					
					fFontFiles.AddItem(new FontFile(fn, size, true_type_type));
#ifndef __POWERPC__						
					snprintf(buffer, sizeof(buffer), "%s==%s", fn, path.Path());
#else
					sprintf(buffer, "%s==%s", fn, path.Path());
#endif
					fprintf(fLog, "%s\n", buffer);
					PDF_set_parameter(fPdf, "FontOutline", buffer);
					};
				};
			delete dir;
			};
			
		which_dir++;
		};
		
	return B_OK;
}


// --------------------------------------------------
static uint16 ttf_get_uint16(FILE * ttf)
{
    uint16 v;

	if (fread(&v, 1, 2, ttf) != 2)
		return 0;

	return B_BENDIAN_TO_HOST_INT16(v);
}

// --------------------------------------------------
static uint32 ttf_get_uint32(FILE *ttf)
{
    uint32 buf;

    if (fread(&buf, 1, 4, ttf) != 4)
		return 0;

	return B_BENDIAN_TO_HOST_INT32(buf);
}


// --------------------------------------------------
static status_t ttf_get_fontname(const char * path, char * fontname, size_t fn_size)
{
	FILE *		ttf;
	status_t	status;
	uint16		nb_tables, nb_records;
	uint16		i;
	uint32		tag;
	uint32		checksum, table_offset, length;
	uint32		strings_offset;
	char		family_name[256];
	char		face_name[256];
	int			names_found;

	status = B_ERROR;
	
	ttf = fopen(path, "rb");
	if (! ttf) 
		return status;

    tag = ttf_get_uint32(ttf);		/* version */
	switch(tag)
		{
		case TRUETYPE_VERSION:
		case OPENTYPE_CFF_VERSION:
			break;
			
		default:
			goto exit;
		};

    /* set up table directory */
    nb_tables = ttf_get_uint16(ttf);

	fseek(ttf, 12, SEEK_SET);
	
	table_offset = 0;	// quiet the compiler...

    for (i = 0; i < nb_tables; ++i)
		{
		tag				= ttf_get_uint32(ttf);
		checksum		= ttf_get_uint32(ttf);
		table_offset	= ttf_get_uint32(ttf);
		length			= ttf_get_uint32(ttf);
	    
		if (tag == TRUETTYPE_TABLE_NAME_TAG)
			break;
		};

	if (tag != TRUETTYPE_TABLE_NAME_TAG)
		// Mandatory name table not found!
		goto exit;
		
	// move to name table start
	fseek(ttf, table_offset, SEEK_SET);
		
	ttf_get_uint16(ttf);	// name table format (must be 0!)
    nb_records		= ttf_get_uint16(ttf);
	strings_offset	= table_offset + ttf_get_uint16(ttf); // string storage offset is from table offset

	//    offs = ttf->dir[idx].offset + tp->offsetStrings;

	// printf("  pid   eid   lid   nid   len offset value\n");
        //  65536 65536 65536 65536 65536 65536  ......

	family_name[0] = 0;
	face_name[0] = 0;
	names_found = 0;

	for (i = 0; i < nb_records; ++i)
		{
		uint16	platform_id, encoding_id, language_id, name_id;
		uint16	string_len, string_offset;

		platform_id		= ttf_get_uint16(ttf);
		encoding_id		= ttf_get_uint16(ttf);
		language_id		= ttf_get_uint16(ttf);
		name_id			= ttf_get_uint16(ttf);
		string_len		= ttf_get_uint16(ttf);
		string_offset	= ttf_get_uint16(ttf);

		if ( name_id != 1 && name_id != 2 )
			continue;

		// printf("%5d %5d %5d %5d %5d %5d ", 
		// 	platform_id, encoding_id, language_id, name_id, string_len, string_offset);

		if (string_len != 0)
			{
			long	pos;
			char *	buffer;

			pos = ftell(ttf);
			fseek(ttf, strings_offset + string_offset, SEEK_SET);

			buffer = (char *) malloc(string_len + 16);

			fread(buffer, 1, string_len, ttf); 
			buffer[string_len] = '\0';

			fseek(ttf, pos, SEEK_SET);
			
			if ( (platform_id == 3 && encoding_id == 1) || // Windows Unicode
				 (platform_id == 0) )// Unicode
				{
				// dirty unicode -> ascii conversion
				int k;

				for (k=0; k < string_len/2; k++)
					buffer[k] = buffer[2*k + 1];
				buffer[k] = '\0';
				};

			// printf("%s\n", buffer);
			
			if (name_id == 1)
				strncpy(family_name, buffer, sizeof(family_name));
			else if (name_id == 2)
				strncpy(face_name, buffer, sizeof(face_name));

			names_found += name_id;

			free(buffer);
			}
		// else
			// printf("<null>\n");

		if (names_found == 3)
			break;
		};
		
	if (names_found == 3)
		{
#ifndef __POWERPC__
		snprintf(fontname, fn_size, "%s-%s", family_name, face_name);
#else
		sprintf(fontname, "%s-%s", family_name, face_name);
#endif
		status = B_OK;
		};
		
exit:;
	fclose(ttf);
	return status;
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
PDFWriter::PushInternalState() {
	fprintf(fLog, "PushInternalState\n");
	fState = new State(fState); fStateDepth ++;
}

// --------------------------------------------------
bool
PDFWriter::PopInternalState() {
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
void
PDFWriter::Paint(bool stroke) {
	if (stroke) {
		StrokeOrClip();
	} else {
		FillOrClip();
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
	uint8   *inRow;
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
		
	if (pixelFormat != B_RGB32 && pixelFormat != B_RGBA32)
		return NULL;

	// Image Mask
	inRow = (uint8 *) data;
	inRow += bytesPerRow * (int) src.top + 4 * (int) src.left;

	maskWidth = (width+7)/8;
	maskRow = mask = new uint8[maskWidth * height];
	memset(mask, 0, maskWidth*height);
	alpha = false;
	
	for (y = height; y > 0; y--) {
		in = inRow;
		out = maskRow;
		shift = 7;
		
		for (x = width; x > 0; x-- ) {
//			fprintf(fLog, "(%d, %d) %d %d %d\n", x, y, (mout - mask), (int)shift, (int)*(in+3));
			// For each pixel
			
			if ((pixelFormat == B_RGBA32 && in[3] < 128) ||
			    (pixelFormat == B_RGB32 && in[0] == transcolor.blue &&
			    in[1] == transcolor.green && in[2] == transcolor.red &&
			    in[3] == transcolor.alpha) ) {
				out[0] |= (1 << shift);
				alpha = true;
			}
			// next pixel			
			if (shift == 0) out ++;
			shift = (shift + 7) & 0x07;		
			in += 4;	
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
BBitmap *
PDFWriter::ConvertBitmap(BRect src, int32 bytesPerRow, int32 pixelFormat, int32 flags, void *data)
{
	uint8	*in;
	uint8   *inLeft;
	uint8	*out;
	uint8   *outLeft;
	int32	x, y;
	
	if (pixelFormat != B_RGB32)
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
	inLeft += bytesPerRow * (int)src.top + 4 * (int)src.left; 
	outLeft	= (uint8*)bm->Bits();
		
	for (y = height; y >= 0; y--) {
		in = inLeft;
		out = outLeft;

		for (x = width; x >= 0; x--) {
//			fprintf(fLog, "(%d, %d) %d %d %d\n", x, y, (mout - mask), (int)shift, (int)*(in+3));
			// For each pixel
			*((rgb_color*)out) = *((rgb_color*)in);
			in += 4;	// next pixel
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
	, fDrawn(false)
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
		PDF_curveto(Pdf(), 
			tx(control[0].x), ty(control[0].y),
			tx(control[1].x), ty(control[1].y),
	    	tx(control[2].x), ty(control[2].y));
		fprintf(Log(), "(%f %f), (%f %f), (%f %f)\n", 
			control[0].x, control[0].y,
			control[1].x, control[1].y,
	    	control[2].x, control[2].y);
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
DrawShape::Draw() {
	if (!fDrawn) {
		fDrawn = true;
		if (IsDrawing()) {
			if (fStroke) 
				PDF_stroke(Pdf()); 
			else {
				PDF_fill(Pdf());
			}
		} else {
			PDF_clip(Pdf());
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

		PDF_lineto(Pdf(), tx(p->x), ty(p->y));
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
	PDF_moveto(Pdf(), tx(point->x), ty(point->y)); 
	fprintf(Log(), "(%f, %f)\n", point->x, point->y);
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
// The quarter ellipses in the corners of the rectangle are
// approximated with bezier curves.
// The constant 0.555... is taken from gobeProductive.
void	
PDFWriter::PaintRoundRect(BRect rect, BPoint radii, bool stroke) {
	BPoint center;
	
	float sx = scale(radii.x);
	float sy = scale(radii.y);
	
	float ax = sx;
	float bx = 0.5555555555555 * sx;
	float ay = sy;
	float by = 0.5555555555555 * sy;

	SetColor();

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
// Note the pen size is also scaled!
// We should approximate it with bezier curves too!
void
PDFWriter::PaintArc(BPoint center, BPoint radii, float startTheta, float arcTheta, int stroke) {
	float sx = scale(radii.x);
	float sy = scale(radii.y);
	float smax = sx > sy ? sx : sy;
	SetColor();

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
PDFWriter::PaintEllipse(BPoint center, BPoint radii, bool stroke) {
	float sx = scale(radii.x);
	float sy = scale(radii.y);
	
	center.x = tx(center.x); center.y = ty(center.y);

	float ax = sx;
	float bx = 0.5555555555555 * sx;
	float ay = sy;
	float by = 0.5555555555555 * sy;

	SetColor();

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
PDFWriter::ClipToPicture(BPicture *picture, BPoint point, bool clip_to_inverse_picture)
{
	fprintf(fLog, "ClipToPicture at (%f, %f) clip_to_inverse_picture = %s\n", point.x, point.y, clip_to_inverse_picture ? "true" : "false");
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
		picture->Play(playbackHandlers, 50, this);
		fMode = kDrawingMode;

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
	PushInternalState();
	fprintf(fLog, "height = %f x0 = %f y0 = %f\n", fState->height, fState->x0, fState->y0);
	PDF_save(fPdf);
}


// --------------------------------------------------
void
PDFWriter::PopState()
{
	if (PopInternalState()) {
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
	// MP: PDF 1.2 and later supports patterns
	// We need to map the Be pattern to PDF pattern.
	// I don't know if xpdf and BePDF support them.
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
//	fState->beFont.SetFace(flags);
//	fState->fontChanged = true;
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
void	_ClipToPicture(void * p, BPicture *picture, BPoint point, bool clip_to_inverse_picture)	{ return ((PDFWriter *) p)->ClipToPicture(picture, point, clip_to_inverse_picture); }
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


