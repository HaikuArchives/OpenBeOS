#ifndef	_RECT_H
#define	_RECT_H

#include <math.h>
#include "support/SupportDefs.h"
#include "interface/Point.h"

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

#define min(a,b) ((a)>(b)?(b):(a))
#define max(a,b) ((a)>(b)?(a):(b))

class BRect {
  public:
	float	left;
	float	top;
	float	right;
	float	bottom;

	BRect()									{ left = right = 0;	top = bottom = -1; }		// an invalid rect
	BRect(const BRect &r)					{ left = r.left; right = r.right; top = r.top; bottom = r.bottom; }
	BRect(float l, float t, float r, float b)
											{ left = l; right = r; top = t; bottom = b; }
	BRect(BPoint lt, BPoint rb)				{ left = lt.x; top = lt.y; right = rb.x; bottom = rb.y; }

	BRect	&operator=(const BRect &r)		{ left = r.left; right = r.right; top = r.top; bottom = r.bottom; return *this; }
	void	Set(float l, float t, float r, float b)
											{ left = l; right = r; top = t; bottom = b; }

	BPoint	LeftTop() const					{ return (BPoint(left, top)); }
	BPoint	RightBottom() const				{ return (BPoint(right, bottom)); }
	BPoint	LeftBottom() const				{ return (BPoint(left, bottom)); }
	BPoint	RightTop() const				{ return (BPoint(right, top)); }
	void	SetLeftTop(const BPoint p)		{ left = p.x; top = p.y; }
	void	SetRightBottom(const BPoint p)	{ right = p.x; bottom = p.y; }
	void	SetLeftBottom(const BPoint p)	{ left = p.x; bottom = p.y; }
	void	SetRightTop(const BPoint p)		{ right = p.x; top = p.y; }
	bool	operator==(BRect r) const		{ return( left==r.left && right==r.right && top==r.top && bottom==r.bottom ); }
	bool	operator!=(BRect r) const		{ return( left!=r.left || right!=r.right || top!=r.top || bottom!=r.bottom ); }
	BRect	operator&(BRect r) const		{ return( BRect( max(left, r.left), max(top, r.top), min(right, r.right), min(bottom, r.bottom) )); }
	BRect	operator|(BRect r) const		{ return( BRect( min(left, r.left), min(top, r.top), max(right, r.right), max(bottom, r.bottom) )); }
	bool	IsValid() const					{ return(left <= right && top <= bottom); }
	float	Width() const					{ return(right-left); }
	int32	IntegerWidth() const			{ return((int32)ceil(right-left)); }
	float	Height() const					{ return(bottom-top); }
	int32	IntegerHeight() const			{ return((int32)ceil(bottom-top)); }
	bool	Intersects(BRect r) const;
	bool	Contains(BPoint p) const		{ return( p.x>=left && p.x<=right && p.y>=top && p.y<=bottom); }
	bool	Contains(BRect r) const			{ return( r.left>=left && r.right<=right && r.top>=top && r.bottom<=bottom); } 

/* transformation */
	void	InsetBy(BPoint p)				{ left+=p.x; right-=p.x; top+=p.y; bottom-=p.y; }
	void	InsetBy(float dx, float dy)		{ left+=dx; right-=dx; top+=dy; bottom-=dy; }
	void	OffsetBy(BPoint p)				{ left+=p.x; right+=p.x; top+=p.y; bottom+=p.y; }
	void	OffsetBy(float dx, float dy)	{ left+=dx; right+=dx; top+=dy; bottom+=dy; }
	void	OffsetTo(BPoint p)				{ right=(right-left)+p.x; left=p.x; bottom=(bottom-top)+p.y; top=p.y; }
	void	OffsetTo(float x, float y)		{ right=(right-left)+x; left=x; bottom=(bottom-top)+y; top=y; }

/* expression transformations */
	BRect &	InsetBySelf(BPoint);
	BRect &	InsetBySelf(float dx, float dy);
	BRect	InsetByCopy(BPoint);
	BRect	InsetByCopy(float dx, float dy);
	BRect &	OffsetBySelf(BPoint);
	BRect &	OffsetBySelf(float dx, float dy);
	BRect	OffsetByCopy(BPoint);
	BRect	OffsetByCopy(float dx, float dy);
	BRect &	OffsetToSelf(BPoint);
	BRect &	OffsetToSelf(float dx, float dy);
	BRect	OffsetToCopy(BPoint);
	BRect	OffsetToCopy(float dx, float dy);

	void	PrintToStream() const;
};

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
using namespace OpenBeOS;
#endif

#endif /* _RECT_H */

