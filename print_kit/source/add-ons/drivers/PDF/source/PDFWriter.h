/*

PDF Writer printer driver.

Copyright (c) 2001 OpenBeOS. 

Authors: 
	Philippe Houdoin
	Simon Gauvin	
	Michael Pfeiffer
	
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

#ifndef PDFWRITER_H
#define PDFWRITER_H

#include <AppKit.h>
#include <InterfaceKit.h>
#include <String.h>
#include <List.h>

#include "math.h"

#include "PrinterDriver.h"
#include "PictureIterator.h"

#include "pdflib.h"

#define SCALE 8.3333f
#define SCREEN 72.0f
#define LETTER_WIDTH 8.5
#define LETTER_HEIGHT 11

#define RAD2DEGREE(r) (180.0 * r / PI)
#define DEGREE2RAD(d) (PI * d / 180.0)

class DrawShape;
class RotateShape;

enum font_encoding {
	macroman_encoding,
	// TrueType
	tt_encoding0,
	tt_encoding1,
	tt_encoding2,
	tt_encoding3,
	tt_encoding4,
	// Type 1
	t1_encoding0,
	t1_encoding1,
	t1_encoding2,
	t1_encoding3,
	t1_encoding4,
	// CJK
	japanese_encoding,
	chinese_cns1_encoding,
	chinese_gb1_encoding,
	korean_encoding // not implemented yet
};

enum font_type {
	true_type_type,
	type1_type,
	unknown_type
};

class PDFWriter : public PrinterDriver, public PictureIterator
	{
	
	friend class DrawShape;
	friend class RotateShape;
	
	public:
		// constructors / destructor
		PDFWriter();
		~PDFWriter();
		
		// public methods
		status_t 	PrintPage(int32 pageNumber, int32 pageCount);
		status_t	InitWriter();
		status_t	BeginPage(BRect paperRect, BRect printRect);
		status_t	EndPage();
		
		// PDFLib callbacks
		size_t		WriteData(void *data, size_t size);
		void		ErrorHandler(int type, const char *msg);
		
		// Image support
		void		*CreateMask(BRect src, int32 bytesPerRow, int32 pixelFormat, int32 flags, void *data);
		BBitmap		*ConvertBitmap(BRect src, int32 bytesPerRow, int32 pixelFormat, int32 flags, void *data);

		// String handling
		bool		BeginsChar(char byte) { return (byte & 0xc0) != 0x80; }
		void		ToUtf8(uint32 encoding, const char *string, BString &utf8);
		void		ToUnicode(const char *string, BString &unicode);
		uint16		CodePointSize(const char *s);
		void		DrawChar(uint16 unicode, const char *utf8, int16 size);
		void		ClipChar(BFont* font, const char* unicode, const char *utf8, int16 size);
		bool   		EmbedFont(const char* n);
		status_t	DeclareFonts();
		status_t	LookupFontFiles(BPath path);	

		// BPicture playback handlers
		void		Op(int number);
		void		MovePenBy(BPoint delta);
		void		StrokeLine(BPoint start, BPoint end);
		void		StrokeRect(BRect rect);
		void		FillRect(BRect rect);
		void		StrokeRoundRect(BRect rect, BPoint radii);
		void		FillRoundRect(BRect rect, BPoint radii);
		void		StrokeBezier(BPoint *control);
		void		FillBezier(BPoint *control);
		void		StrokeArc(BPoint center, BPoint radii, float startTheta, float arcTheta);
		void		FillArc(BPoint center, BPoint radii, float startTheta, float arcTheta);
		void		StrokeEllipse(BPoint center, BPoint radii);
		void		FillEllipse(BPoint center, BPoint radii);
		void		StrokePolygon(int32 numPoints, BPoint *points, bool isClosed);
		void		FillPolygon(int32 numPoints, BPoint *points, bool isClosed);
		void        StrokeShape(BShape *shape);
		void        FillShape(BShape *shape);
		void		DrawString(char *string, float deltax, float deltay);
		void		DrawPixels(BRect src, BRect dest, int32 width, int32 height, int32 bytesPerRow, int32 pixelFormat, int32 flags, void *data);
		void		SetClippingRects(BRect *rects, uint32 numRects);
		void    	ClipToPicture(BPicture *picture, BPoint point, bool clip_to_inverse_picture);
		void		PushState();
		void		PopState();
		void		EnterStateChange();
		void		ExitStateChange();
		void		EnterFontState();
		void		ExitFontState();
		void		SetOrigin(BPoint pt);
		void		SetPenLocation(BPoint pt);
		void		SetDrawingMode(drawing_mode mode);
		void		SetLineMode(cap_mode capMode, join_mode joinMode, float miterLimit);
		void		SetPenSize(float size);
		void		SetForeColor(rgb_color color);
		void		SetBackColor(rgb_color color);
		void		SetStipplePattern(pattern p);
		void		SetScale(float scale);
		void		SetFontFamily(char *family);
		void		SetFontStyle(char *style);
		void		SetFontSpacing(int32 spacing);
		void		SetFontSize(float size);
		void		SetFontRotate(float rotation);
		void		SetFontEncoding(int32 encoding);
		void		SetFontFlags(int32 flags);
		void		SetFontShear(float shear);
		void		SetFontFace(int32 flags);
		
	private:
	
		class State {
		public:
			State			*prev;
			BFont           beFont;
			int				font;
			bool			fontChanged;
			float			height;
			float			x0;
			float			y0;
			float           scale;
			float			penX;
			float			penY;
			drawing_mode 	drawingMode;
			rgb_color		foregroundColor;
			rgb_color		backgroundColor;
			rgb_color       currentColor;
			cap_mode 		capMode;
			join_mode 		joinMode;
			float 			miterLimit;
			float           penSize;
			pattern         pattern;
			int32           fontSpacing;
			
			// initialize with defalt values
			State() {
				static rgb_color white    = {255, 255, 255, 255};
				static rgb_color black    = {0, 0, 0, 255};
				prev = NULL;
				font             = 0;
				fontChanged      = false;
				height           = a4_height;
				x0               = 0;
				y0               = 0;
				scale            = 1.0;
				penX             = 0;
				penY             = 0;
				drawingMode      = B_OP_COPY; 
				foregroundColor  = white;
				backgroundColor  = black;
				currentColor     = black;
				capMode          = B_BUTT_CAP; 
				joinMode         = B_MITER_JOIN; 
				miterLimit       = B_DEFAULT_MITER_LIMIT; 
				penSize          = 1; 
				pattern          = B_SOLID_HIGH; 
				fontSpacing      = B_STRING_SPACING; 
			}

			State(State *prev) {
				*this = *prev;
				this->prev = prev;
			}
		};

		class Font {
		public:
			Font(char *n, int f, font_encoding e) : name(n), font(f), encoding(e) { }
			BString name;
			int     font;
			font_encoding encoding;
		};

		class FontFile {
		public:
			FontFile(char *n, int64 s, font_type t) : name(n), size(s), type(t) { }
			BString   name;
			int64     size;
			font_type type;
		};
	
		FILE			*fLog;
		PDF				*fPdf;
		State			*fState;
		int32           fStateDepth;
		BList           fFontCache;
		BList           fFontFiles;
		int64           fEmbedMaxFontSize;
		int             fPattern;
		enum {
			kDrawingMode,
			kClippingMode
		}				fMode;

		inline float tx(float x)    { return fState->x0 + fState->scale*x; }
		inline float ty(float y)    { return fState->height - (fState->y0 + fState->scale*y); }
		inline float scale(float f) { return fState->scale * f; }

		inline bool IsDrawing() const  { return fMode == kDrawingMode; }
		inline bool IsClipping() const { return fMode == kClippingMode; }

		bool StoreTranslatorBitmap(BBitmap *bitmap, char *filename, uint32 type);

		void GetFontName(BFont *font, char *fontname, bool &embed, font_encoding encoding);
		int FindFont(char *fontname, bool embed, font_encoding encoding);

		void PushInternalState();
		bool PopInternalState();

		void CreatePatterns();
		void SetColor(rgb_color toSet);
		void SetColor();
		void SetPattern();
		
		void StrokeOrClip();
		void FillOrClip();
		void Paint(bool stroke);
		void PaintRoundRect(BRect rect, BPoint radii, bool stroke);
		void PaintArc(BPoint center, BPoint radii, float startTheta, float arcTheta, int stroke);
		void PaintEllipse(BPoint center, BPoint radii, bool stroke);
	};


class DrawShape : public BShapeIterator {
	PDFWriter *fWriter;
	bool       fStroke;
	bool       fDrawn;
	
	inline FILE *Log()			{ return fWriter->fLog; }
	inline PDF *Pdf()			{ return fWriter->fPdf; }
	inline float tx(float x)	{ return fWriter->tx(x); }
	inline float ty(float y)	{ return fWriter->ty(y); }

	inline bool IsDrawing() const  { return fWriter->IsDrawing(); }
	inline bool IsClipping() const { return fWriter->IsClipping(); }
	
public:
	DrawShape(PDFWriter *writer, bool stroke);
	~DrawShape();
	status_t IterateBezierTo(int32 bezierCount, BPoint *bezierPoints);
	status_t IterateClose(void);
	status_t IterateLineTo(int32 lineCount, BPoint *linePoints);
	status_t IterateMoveTo(BPoint *point);
	
	void Draw();
};

// PDFLib C callbacks class instance redirectors
size_t	_WriteData(PDF *p, void *data, size_t size);
void	_ErrorHandler(PDF *p, int type, const char *msg);

#endif	// #if PDFWRITER_H
