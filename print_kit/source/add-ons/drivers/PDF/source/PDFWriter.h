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

#include "PrinterDriver.h"

#include "pdflib.h"

#define SCALE 8.3333f
#define SCREEN 72.0f
#define LETTER_WIDTH 8.5
#define LETTER_HEIGHT 11

class DrawShape;

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
	type1_type
};

class PDFWriter : public PrinterDriver
	{
	
	friend class DrawShape;
	
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
		bool   BeginsChar(char byte) { return (byte & 0xc0) != 0x80; }
		void   ToUtf8(uint32 encoding, const char *string, BString &utf8);
		void   ToUnicode(const char *string, BString &unicode);
		uint16 CodePointSize(const char *s);
		void   DrawChar(uint16 unicode, const char *utf8, int16 size);
		bool   EmbedFont(const char* n);
		status_t DeclareFonts();

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

		void SetColor(rgb_color toSet);
		void SetColor();
		
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

// BPicture playback handlers class instance redirectors
void	_MovePenBy(void *p, BPoint delta);
void	_StrokeLine(void *p, BPoint start, BPoint end);
void	_StrokeRect(void *p, BRect rect);
void	_FillRect(void *p, BRect rect);
void	_StrokeRoundRect(void *p, BRect rect, BPoint radii);
void	_FillRoundRect(void *p, BRect rect, BPoint radii);
void	_StrokeBezier(void *p, BPoint *control);
void	_FillBezier(void *p, BPoint *control);
void	_StrokeArc(void *p, BPoint center, BPoint radii, float startTheta, float arcTheta);
void	_FillArc(void *p, BPoint center, BPoint radii, float startTheta, float arcTheta);
void	_StrokeEllipse(void *p, BPoint center, BPoint radii);
void	_FillEllipse(void *p, BPoint center, BPoint radii);
void	_StrokePolygon(void *p, int32 numPoints, BPoint *points, bool isClosed);
void	_FillPolygon(void *p, int32 numPoints, BPoint *points, bool isClosed);
void	_StrokeShape(void *p, BShape *shape);
void	_FillShape(void *p, BShape *shape);
void	_DrawString(void *p, char *string, float deltax, float deltay);
void	_DrawPixels(void *p, BRect src, BRect dest, int32 width, int32 height, int32 bytesPerRow, int32 pixelFormat, int32 flags, void *data);
void	_SetClippingRects(void *p, BRect *rects, uint32 numRects);
void	_ClipToPicture(void *p, BPicture *picture, BPoint point, bool clip_to_inverse_picture);
void	_PushState(void *p);
void	_PopState(void *p);
void	_EnterStateChange(void *p);
void	_ExitStateChange(void *p);
void	_EnterFontState(void *p);
void	_ExitFontState(void *p);
void	_SetOrigin(void *p, BPoint pt);
void	_SetPenLocation(void *p, BPoint pt);
void	_SetDrawingMode(void *p, drawing_mode mode);
void	_SetLineMode(void *p, cap_mode capMode, join_mode joinMode, float miterLimit);
void	_SetPenSize(void *p, float size);
void	_SetForeColor(void *p, rgb_color color);
void	_SetBackColor(void *p, rgb_color color);
void	_SetStipplePattern(void *p, pattern pat);
void	_SetScale(void *p, float scale);
void	_SetFontFamily(void *p, char *family);
void	_SetFontStyle(void *p, char *style);
void	_SetFontSpacing(void *p, int32 spacing);
void	_SetFontSize(void *p, float size);
void	_SetFontRotate(void *p, float rotation);
void	_SetFontEncoding(void *p, int32 encoding);
void	_SetFontFlags(void *p, int32 flags);
void	_SetFontShear(void *p, float shear);
void	_SetFontFace(void *p, int32 flags);

// undefined or undocumented operation handlers...
void	_op0(void *p);
void	_op19(void *p);
void	_op45(void *p);
void	_op47(void *p);
void	_op48(void *p);
void	_op49(void *p);

#endif	// #if PDFWRITER_H
