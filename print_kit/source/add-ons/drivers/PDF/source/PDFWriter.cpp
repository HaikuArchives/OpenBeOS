/*

PDF Writer printer driver.

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
#include "pdflib.h"

#define fmin(x, y) ( (x < y) ? x : y);

#define TRUETYPE_VERSION		0x00010000
#define OPENTYPE_CFF_VERSION	'OTTO'

#define TRUETTYPE_TABLE_NAME_TAG	'name'

static uint16 ttf_get_uint16(FILE * ttf);
static uint32 ttf_get_uint32(FILE *ttf);
static status_t ttf_get_fontname(const char * path, char * fontname, size_t fn_size);
static status_t psf_get_fontname(const char * path, char * fontname, size_t fn_size);

// Constructor & destructor
// ------------------------

PDFWriter::PDFWriter()
	:	PrinterDriver()
{
	fEmbedMaxFontSize = 250 * 1024;
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
		Iterate(pictures[i]);
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

	CreatePatterns();

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
			LookupFontFiles(path);

		which_dir++;
		};
		
	return B_OK;
}


// --------------------------------------------------
status_t
PDFWriter::LookupFontFiles
	(
	BPath	path
	)
{
	BDirectory 	dir(path.Path());
	BEntry 		entry;

	if (dir.InitCheck() != B_OK)
		return B_ERROR;

	fprintf(fLog, "--- From %s\n", path.Path());
	
	dir.Rewind();
	while (dir.GetNextEntry(&entry) >= 0)
		{
		BPath 		name;
		char 		fn[512];
		font_type	ft = unknown_type; // to keep the compiler silent.
		off_t 		size;
		char		buffer[1024];
		char *		parameter_name;
		status_t	status;
		
		entry.GetPath(&name);
		if (entry.IsDirectory())
			// recursivly lookup in sub-directories...
			LookupFontFiles(name);

		if (! entry.IsFile())
			continue;

		fn[0] = 0;
		parameter_name = NULL;
		
		// is it a truetype file?
		status = ttf_get_fontname(name.Path(), fn, sizeof(fn));
		if (status == B_OK )
			{
			ft = true_type_type;
			parameter_name = "FontOutline";
			}
		else
			{
			// okay, maybe it's a postscript type file?
			status = psf_get_fontname(name.Path(), fn, sizeof(fn));
			if (status == B_OK)
				{
				ft = type1_type;
				if (strstr(name.Leaf(), ".afm"))
					parameter_name = "FontAFM";
				else if (strstr(name.Leaf(), ".pfm"))
					parameter_name = "FontPFM";
				};
			}; 

		if (! parameter_name)
			// not a font file...
			continue;
										
		if (entry.GetSize(&size) != B_OK)
			size = 1024*1024*1024;
					
		fFontFiles.AddItem(new FontFile(fn, size, ft));
#ifndef __POWERPC__						
		snprintf(buffer, sizeof(buffer), "%s==%s", fn, name.Path());
#else
		sprintf(buffer, "%s==%s", fn, name.Path());
#endif
		fprintf(fLog, "%s: %s\n", parameter_name, buffer);
		
		PDF_set_parameter(fPdf, parameter_name, buffer);
		};	// while dir.GetNextEntry()...	

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


// --------------------------------------------------
static status_t psf_get_fontname(const char * path, char * fontname, size_t fn_size)
{
	FILE *		psf;
	status_t	status;
	int			i;
	char		line[1024];
	char * 		token;
	char *		name;

	// *.afm	search for "FontName <font_name_without_blank>" line
	// *.pfa 	search for "/FontName /<font_name_without_blank> def" line

	status = B_ERROR;
	
	psf = fopen(path, "r");
	if (! psf) 
		return status;

	name = NULL;
	
	i = 0;
	while ( fgets(line, sizeof(line), psf) != NULL )
		{
		i++;
		if ( i > 64 )
			// only check the first 64 lines of files...
			break;
		
		token = strtok(line, " \r\n");
		if (! token)
			continue;
			
		if (strcmp(token, "FontName") == 0)
			name = strtok(NULL, " \r\n");
		else if (strcmp(token, "/FontName") == 0)
			{
			name = strtok(NULL, " \r\n");
			if (name)
				name++;	// skip the '/'
			};

		if (name)
			break;
		};
		
	if (name)
		{
		strncpy(fontname, name, fn_size);
		status = B_OK;
		};
		
	fclose(psf);
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
PDFWriter::CreatePatterns() {
/*
	fPattern = -1;
	fprintf(fLog, "CreatePatterns\n");
	BBitmap bm(BRect(0, 0, 7, 7), B_RGB24);
	if (!bm.IsValid()) {
		fprintf(fLog, "CreatePattern could not create bitmap\n");
		return;
	}

	int8* c = (int8*)bm.Bits();
	c[0] = 255;
	c[3  +1] = 255;
	c[2*3+2] = 255;

	int image = PDF_open_image(fPdf, "raw", "memory", (const char *) bm.Bits(), bm.BytesPerRow()*8, 8, 8, 3, 8, 0);
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
	fPattern = pattern;
	PDF_place_image(fPdf, image, 0, 0, 1);
	PDF_end_pattern(fPdf);
	PDF_close_image(fPdf, image);
*/
}

// --------------------------------------------------
void
PDFWriter::SetPattern() {
//	PDF_setcolor(fPdf, "both", "pattern", fPattern, 0, 0, 0);
}

// --------------------------------------------------
void 
PDFWriter::StrokeOrClip() {
	if (IsDrawing()) {
		PDF_stroke(fPdf);
	} else {
		PDF_closepath(fPdf);
	}
}

// --------------------------------------------------
void 
PDFWriter::FillOrClip() {
	if (IsDrawing()) {
		PDF_fill(fPdf);
	} else {
		PDF_closepath(fPdf);
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
//	} else {
//		SetPattern();
//	} 
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


size_t	_WriteData(PDF *pdf, void *data, size_t size)			{ return ((PDFWriter *) PDF_get_opaque(pdf))->WriteData(data, size); }
void	_ErrorHandler(PDF *pdf, int type, const char *msg)	{ return ((PDFWriter *) PDF_get_opaque(pdf))->ErrorHandler(type, msg); }

