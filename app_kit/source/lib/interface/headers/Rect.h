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

	BRect();
	BRect(const BRect &r);
	BRect(float l, float t, float r, float b);
	BRect(BPoint lt, BPoint rb);

	BRect	&operator=(const BRect &r);
	void	Set(float l, float t, float r, float b);

	void	PrintToStream() const;

	BPoint	LeftTop() const;
	BPoint	RightBottom() const;
	BPoint	LeftBottom() const;
	BPoint	RightTop() const;

	void	SetLeftTop(const BPoint p);
	void	SetRightBottom(const BPoint p);
	void	SetLeftBottom(const BPoint p);
	void	SetRightTop(const BPoint p);

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

/* comparison */
	bool	operator==(BRect r) const		{ return( left==r.left && right==r.right && top==r.top && bottom==r.bottom ); }
	bool	operator!=(BRect r) const		{ return( left!=r.left || right!=r.right || top!=r.top || bottom!=r.bottom ); }

/* intersection and union */
	BRect	operator&(BRect r) const		{ return( BRect( max(left, r.left), max(top, r.top), min(right, r.right), min(bottom, r.bottom) )); }
	BRect	operator|(BRect r) const		{ return( BRect( min(left, r.left), min(top, r.top), max(right, r.right), max(bottom, r.bottom) )); }

	bool	Intersects(BRect r) const;
	bool	IsValid() const;
	float	Width() const;
	int32	IntegerWidth() const;
	float	Height() const;
	int32	IntegerHeight() const;
	bool	Contains(BPoint p) const;
	bool	Contains(BRect r) const;

};

/*----------------------------------------------------------------*/
/*----- inline definitions ---------------------------------------*/

inline BPoint BRect::LeftTop() const
{
	return(*((const BPoint*)&left));
}

inline BPoint BRect::RightBottom() const
{
	return(*((const BPoint*)&right));
}

inline BPoint BRect::LeftBottom() const
{
	return(BPoint(left, bottom));
}

inline BPoint BRect::RightTop() const
{
	return(BPoint(right, top));
}

inline BRect::BRect()
{
	top = left = 0;
	bottom = right = -1;
}

inline BRect::BRect(float l, float t, float r, float b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

inline BRect::BRect(const BRect &r)
{
	left = r.left;
	top = r.top;
	right = r.right;
	bottom = r.bottom;
}

inline BRect::BRect(BPoint leftTop, BPoint rightBottom)
{
	left = leftTop.x;
	top = leftTop.y;
	right = rightBottom.x;
	bottom = rightBottom.y;
}

inline BRect &BRect::operator=(const BRect& from)
{
	left = from.left;
	top = from.top;
	right = from.right;
	bottom = from.bottom;
	return *this;
}

inline void BRect::Set(float l, float t, float r, float b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

inline bool BRect::IsValid() const
{
	if (left <= right && top <= bottom)
		return true;
	else
		return false;
}

inline int32 BRect::IntegerWidth() const
{
	return((int32)ceil(right - left));
}

inline float BRect::Width() const
{
	return(right - left);
}

inline int32 BRect::IntegerHeight() const
{
	return((int32)ceil(bottom - top));
}

inline float BRect::Height() const
{
	return(bottom - top);
}

/*-------------------------------------------------------------*/
/*-------------------------------------------------------------*/

#ifdef USE_OPENBEOS_NAMESPACE
}	// namespace OpenBeOS
using namespace OpenBeOS;
#endif

#endif /* _RECT_H */

