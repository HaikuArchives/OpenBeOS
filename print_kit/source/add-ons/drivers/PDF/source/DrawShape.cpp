/*

DrawShape

Copyright (c) 2002 OpenBeOS. 

Author: 
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

#include "DrawShape.h"
#include "Bezier.h"
#include "PDFLinePathBuilder.h"
#include "Log.h"

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
void
DrawShape::CreateBezierPath(BPoint *curve) {
	Bezier bezier(curve, 4);
	const int n = 10; // XXX find a heuristic to calculate this value
	for (int i = 1; i <= n; i ++) {
		fSubPath.AddPoint(bezier.PointAt(i / (float) n));
	}
}


// --------------------------------------------------
void
DrawShape::EndSubPath()
{	
	PDFLinePathBuilder builder(&fSubPath, fWriter);
	builder.CreateLinePath();	
}


// --------------------------------------------------
status_t 
DrawShape::IterateBezierTo(int32 bezierCount, BPoint *control)
{
	for (int32 i = 0; i < bezierCount; i++, control += 3) {
		if (TransformPath()) {
			BPoint p[4] = { fCurrentPoint, control[0], control[1], control[2] };
			CreateBezierPath(p);
		} else {
			PDF_curveto(Pdf(), 
				tx(control[0].x), ty(control[0].y),
				tx(control[1].x), ty(control[1].y),
	    		tx(control[2].x), ty(control[2].y));
	    }
		fCurrentPoint = control[2];
	}
	return B_OK;
}


// --------------------------------------------------
status_t 
DrawShape::IterateClose(void)
{
	LOG((Log(), "IterateClose %s\n", IsDrawing() ? (fStroke ? "stroke" : "fill") : "clip"));
	if (fDrawn) LOG((Log(), ">>> IterateClose called multiple times!"));
	if (TransformPath())
		fSubPath.Close();
	else
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
		} else if (TransformPath()) {
			EndSubPath();
		} else {
			PDF_closepath(Pdf());
		}
	}
}


// --------------------------------------------------
status_t 
DrawShape::IterateLineTo(int32 lineCount, BPoint *linePoints)
{
	LOG((Log(), "IterateLineTo %d\n", (int)lineCount));
	BPoint *p = linePoints;
	for (int32 i = 0; i < lineCount; i++) {
		LOG((Log(), "(%f, %f) ", p->x, p->y));

		if (TransformPath()) {
			fSubPath.AddPoint(*p);
		} else {
			PDF_lineto(Pdf(), tx(p->x), ty(p->y));
		}
		fCurrentPoint = *p;
		p++;
	}
	LOG((Log(), "\n"));
	return B_OK;
}


// --------------------------------------------------
status_t 
DrawShape::IterateMoveTo(BPoint *point)
{
	LOG((Log(), "IterateMoveTo "));
	if (!TransformPath()) {
		PDF_moveto(Pdf(), tx(point->x), ty(point->y)); 
	} else {
		EndSubPath();
		fSubPath.MakeEmpty();
		fSubPath.AddPoint(*point);
	}
	LOG((Log(), "(%f, %f)\n", point->x, point->y));
	fCurrentPoint = *point;
	return B_OK;
}
