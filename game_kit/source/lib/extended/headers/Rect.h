/*****************************************************************************/
// Rect.h
//
// This is our generic implementation of the BRect class. We need the
// genericity to avoid writing a separate class for rectangles with integer
// coordinates.
//
//
// Copyright (c) 2001 OpenBeOS Project
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included 
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
/*****************************************************************************/


#ifndef EXTENDED_GAME_KIT_RECTANGLE
#define EXTENDED_GAME_KIT_RECTANGLE

#include <algorithm>
#include <cmath>
#include <SupportDefs.h>
#include "Point.h"


template
<
	typename StorageType = float
>
class Rect {

public:
	StorageType left;
	StorageType top;
	StorageType right;
	StorageType bottom;

	Rect();
	Rect(const Rect&);
	Rect(StorageType l, StorageType t, StorageType r, StorageType b);
	Rect(Point<StorageType> leftTop, Point<StorageType> rightBottom);

	Rect<StorageType>& operator=(const Rect& from);
	void Set(StorageType l, StorageType t, StorageType r, StorageType b);

	void PrintToStream() const;

	// BPoint selectors
	Point<StorageType> LeftTop() const;
	Point<StorageType> RightBottom() const;
	Point<StorageType> LeftBottom() const;
	Point<StorageType> RightTop() const;

	// BPoint setters
	void SetLeftTop(const Point<StorageType>);
	void SetRightBottom(const Point<StorageType>);
	void SetLeftBottom(const Point<StorageType>);
	void SetRightTop(const Point<StorageType>);

	// Transformation
	void InsetBy(Point<StorageType>);
	void InsetBy(StorageType dx, StorageType dy);
	void OffsetBy(Point<StorageType>);
	void OffsetBy(StorageType dx, StorageType dy);
	void OffsetTo(Point<StorageType>);
	void OffsetTo(StorageType x, StorageType y);

	// Expression transformations
	Rect<StorageType>& InsetBySelf(Point<StorageType>);
	Rect<StorageType>& InsetBySelf(StorageType dx, StorageType dy);
	Rect<StorageType> InsetByCopy(Point<StorageType>);
	Rect<StorageType> InsetByCopy(StorageType dx, StorageType dy);
	Rect<StorageType>& OffsetBySelf(Point<StorageType>);
	Rect<StorageType>& OffsetBySelf(StorageType dx, StorageType dy);
	Rect<StorageType> OffsetByCopy(Point<StorageType>);
	Rect<StorageType> OffsetByCopy(StorageType dx, StorageType dy);
	Rect<StorageType>& OffsetToSelf(Point<StorageType>);
	Rect<StorageType>& OffsetToSelf(StorageType dx, StorageType dy);
	Rect<StorageType> OffsetToCopy(Point<StorageType>);
	Rect<StorageType> OffsetToCopy(StorageType dx, StorageType dy);

	// Comparison
	bool operator==(Rect<StorageType>) const;
	bool operator!=(Rect<StorageType>) const;

	// Intersection and union
	Rect<StorageType> operator&(Rect<StorageType>) const;
	Rect<StorageType> operator|(Rect<StorageType>) const;

	// Utilities
	bool Intersects(Rect<StorageType> r) const;
	bool IsValid() const;
	StorageType Width() const;
	int32 IntegerWidth() const;
	StorageType Height() const;
	int32 IntegerHeight() const;
	bool Contains(Point<StorageType>) const;
	bool Contains(Rect<StorageType>) const;
};


// ------------------------------------------------
// Methods
// ------------------------------------------------

template
<
	typename StorageType
>
void 
Rect<StorageType>::PrintToStream() const
{
	// Display one digit after the decimal point.
	cout << setiosflags( ios::fixed )
		 << setprecision( 1 )
		 << "Rect(l:" << left << ", t:" << top
		 << ", r:" << right << ", b:" << bottom
		 << ")" << endl;
}


template
<
	typename StorageType
>
void 
Rect<StorageType>::InsetBy(Point<StorageType> point)
{
	// Isn't overloading great?
	InsetBy( point.x, point.y );	
}


template
<
	typename StorageType
>
void 
Rect<StorageType>::InsetBy(StorageType dx, StorageType dy)
{
	// Excerpt from Das Buch:
	//
	// InsetBy() insets the sides of the BRect's rectangle by x units 
	// (left and right sides) and y units (top and bottom). 
	// Positive inset values shrink the rectangle; negative values 
	// expand it. Note that both sides of each pair moves the full amount.

	left += dx;
	right -= dx;
	
	top += dy;
	bottom -= dy;
}


template
<
	typename StorageType
>
void 
Rect<StorageType>::OffsetBy(Point<StorageType> point)
{
	OffsetBy( point.x, point.y );
}


template
<
	typename StorageType
>
void 
Rect<StorageType>::OffsetBy(StorageType dx, StorageType dy)
{
	// Excerpt from Das Buch:
	//
	// OffsetBy() moves the BRect horizontally by x units and 
	// vertically by y units. The rectangle's size doesn't change.
	
	left += dx;
	right += dx;
	
	top += dy;
	bottom += dy;
}


template
<
	typename StorageType
>
void 
Rect<StorageType>::OffsetTo(Point<StorageType> point)
{
	OffsetTo( point.x, point.y );
}


template
<
	typename StorageType
>
void 
Rect<StorageType>::OffsetTo(StorageType x, StorageType y)
{
	StorageType dx = x - left;
	StorageType dy = y - top;
	
	OffsetBy( dx, dy );
}

template
<
	typename StorageType
>
Rect<StorageType>& 
Rect<StorageType>::InsetBySelf(Point<StorageType> point)
{
	// Excerpt from Das Buch:
	//
	// The ...Self() versions of the functions are the same as the 
	// simpler versions, but they conveniently return the modified 
	// BRect.

	InsetBy( point );
	return (*this);
}


template
<
	typename StorageType
>
Rect<StorageType>& 
Rect<StorageType>::InsetBySelf(StorageType dx, StorageType dy)
{
	// Excerpt from Das Buch:
	//
	// The ...Self() versions of the functions are the same as the 
	// simpler versions, but they conveniently return the modified 
	// BRect.

	InsetBy( dx, dy );
	return (*this);
}


template
<
	typename StorageType
>
Rect<StorageType>
Rect<StorageType>::InsetByCopy(Point<StorageType> point)
{
	// Excerpt from Das Buch:
	//
	// The ...Copy() versions copy the BRect, and then modify 
	// and return the copy (without changing the original).
	
	Rect<StorageType> rectCopy( *this );
	rectCopy.InsetBy( point );
	return (rectCopy);
}


template
<
	typename StorageType
>
Rect<StorageType> 
Rect<StorageType>::InsetByCopy(StorageType dx, StorageType dy)
{
	// Excerpt from Das Buch:
	//
	// The ...Copy() versions copy the BRect, and then modify 
	// and return the copy (without changing the original).

	Rect<StorageType> rectCopy( *this );
	rectCopy.InsetBy( dx, dy );
	return (rectCopy);
}


template
<
	typename StorageType
>
Rect<StorageType>& 
Rect<StorageType>::OffsetBySelf(Point<StorageType> point)
{
	// Excerpt from Das Buch:
	//
	// The ...Self() versions of the functions are the same as the 
	// simpler versions, but they conveniently return the modified 
	// BRect.

	OffsetBy( point );
	return (*this);
}


template
<
	typename StorageType
>
Rect<StorageType>& 
Rect<StorageType>::OffsetBySelf(StorageType dx, StorageType dy)
{
	// Excerpt from Das Buch:
	//
	// The ...Self() versions of the functions are the same as the 
	// simpler versions, but they conveniently return the modified 
	// BRect.

	OffsetBy( dx, dy );
	return (*this);
}


template
<
	typename StorageType
>
Rect<StorageType> 
Rect<StorageType>::OffsetByCopy(Point<StorageType> point)
{
	// Excerpt from Das Buch:
	//
	// The ...Copy() versions copy the BRect, and then modify 
	// and return the copy (without changing the original).

	Rect<StorageType> rectCopy( *this );
	rectCopy.OffsetBy( point );
	return (rectCopy);
}


template
<
	typename StorageType
>
Rect<StorageType> 
Rect<StorageType>::OffsetByCopy(StorageType dx, StorageType dy)
{
	// Excerpt from Das Buch:
	//
	// The ...Copy() versions copy the BRect, and then modify 
	// and return the copy (without changing the original).

	Rect<StorageType> rectCopy( *this );
	rectCopy.OffsetBy( dx, dy );
	return (rectCopy);
}


template
<
	typename StorageType
>
Rect<StorageType>& 
Rect<StorageType>::OffsetToSelf(Point<StorageType> point)
{
	// Excerpt from Das Buch:
	//
	// The ...Self() versions of the functions are the same as the 
	// simpler versions, but they conveniently return the modified 
	// BRect.

	OffsetTo( point );
	return (*this);
}


template
<
	typename StorageType
>
Rect<StorageType>& 
Rect<StorageType>::OffsetToSelf(StorageType dx, StorageType dy)
{
	// Excerpt from Das Buch:
	//
	// The ...Self() versions of the functions are the same as the 
	// simpler versions, but they conveniently return the modified 
	// BRect.

	OffsetTo( dx, dy );
	return (*this);
}


template
<
	typename StorageType
>
Rect<StorageType> 
Rect<StorageType>::OffsetToCopy(Point<StorageType> point)
{
	// Excerpt from Das Buch:
	//
	// The ...Copy() versions copy the BRect, and then modify 
	// and return the copy (without changing the original).

	Rect<StorageType> rectCopy( *this );
	rectCopy.OffsetTo( point );
	return (rectCopy);
}


template
<
	typename StorageType
>
Rect<StorageType> 
Rect<StorageType>::OffsetToCopy(StorageType dx, StorageType dy)
{
	// Excerpt from Das Buch:
	//
	// The ...Copy() versions copy the BRect, and then modify 
	// and return the copy (without changing the original).

	Rect<StorageType> rectCopy( *this );
	rectCopy.OffsetTo( dx, dy );
	return (rectCopy);
}


template
<
	typename StorageType
>
bool 
Rect<StorageType>::operator==(Rect<StorageType> rect) const
{
	bool result = false;

	if ( (left == rect.left) &&
		 (right == rect.right) &&
		 (top == rect.top) &&
		 (bottom == rect.bottom) )
		result = true;
	
	return (result);
}


template
<
	typename StorageType
>
bool 
Rect<StorageType>::operator!=(Rect<StorageType> rect) const
{
	bool result = false;
	
	if ( (left != rect.left) ||
		 (right != rect.right) ||
		 (top != rect.top) ||
		 (bottom != rect.bottom) )
		result = true;

	return (result);
}

template
<
	typename StorageType
>
Rect<StorageType> 
Rect<StorageType>::operator&(Rect<StorageType> rect) const
{
	Rect<StorageType> result;

	result.left = std::max( left, rect.left );
	result.top = std::max( top, rect.top );
	result.right = std::min( right, rect.right );
	result.bottom = std::min( bottom, rect.bottom );
			
	return( result );
}


template
<
	typename StorageType
>
Rect<StorageType> 
Rect<StorageType>::operator|(Rect<StorageType> rect) const
{
	Rect<StorageType> result;

	result.left = std::min( left, rect.left );
	result.top = std::min( top, rect.top );
	result.right = std::max( right, rect.right );
	result.bottom = std::max( bottom, rect.bottom );

	return( result );
}


template
<
	typename StorageType
>
bool 
Rect<StorageType>::Intersects(Rect<StorageType> rect) const
{
	bool result = false;

	// An easy way to tell if the input rectangle intersects
	// this rectangle is to see if any of the input rects corners
	// are contained by the boundaries of the rectangles. If there
	// is an intersection, at least one of the vertices of the
	// input must be contained in this rectangle, or at least one
	// of the vertices of this rectangle must be contained in the
	// input rectangle.
	if( Contains( rect.LeftTop() ) ||
		Contains( rect.RightTop() ) ||
		Contains( rect.LeftBottom() ) ||
		Contains( rect.RightBottom() ) ||
		rect.Contains( LeftTop() ) ||
		rect.Contains( RightTop() ) ||
		rect.Contains( LeftBottom() ) ||
		rect.Contains( RightBottom() ) )
		result = true;
	
	return (result);
}


template
<
	typename StorageType
>
bool 
Rect<StorageType>::Contains(Point<StorageType> point) const
{
	bool result = true;
	
	// TO DO:
	// How should we behave if this is an invalid rectange,
	// or do we care? Check the behaviour of the existing
	// BRect class to be sure.
	
	// Check if the point is *outside* the boundary of the
	// rectangle because short-circuit evaluation should let us
	// quit sooner in the case of non-containment than if we
	// had to check all four conditions to be sure of containment.
	if( ( point.x < left ) ||
		( point.x > right ) ||
		( point.y < top ) ||
		( point.y > bottom ) )
		result = false;
	
	return (result);
}


template
<
	typename StorageType
>
bool 
Rect<StorageType>::Contains(Rect<StorageType> rect) const
{
	bool result = false;

	// If all four vertices of the input rectangle are
	// contained by the boundaries of this rectangle, then
	// the input must be entirely contained.
	if( ( Contains( rect.LeftTop() ) == true ) &&
		( Contains( rect.RightTop() ) == true ) &&
		( Contains( rect.LeftBottom() ) == true ) &&
		( Contains( rect.RightBottom() ) == true ) )
		result = true;

	return (result);	
}

// ------------------------------------------------
// Inlines
// ------------------------------------------------

template
<
	typename StorageType
>
inline Rect<StorageType>::Rect() :
	left( 0 ), top( 0 ), right( -1 ), bottom( -1 )
{
}


template
<
	typename StorageType
>
inline Rect<StorageType>::Rect(StorageType l, StorageType t, 
	StorageType r, StorageType b) :
	left( l ), top( t ), right( r ), bottom( b )
{
}


template
<
	typename StorageType
>
inline Rect<StorageType>::Rect(const Rect& rect) :
	left( rect.left ), top( rect.top ), right( rect.right ), 
		bottom( rect.bottom )
{
}


template
<
	typename StorageType
>
inline Rect<StorageType>::Rect(Point<StorageType> leftTop, 
	Point<StorageType> rightBottom) :
	left( leftTop.x ), top( leftTop.y ), right( rightBottom.x ),
		bottom( rightBottom.y )
{
}


template
<
	typename StorageType
>
inline Point<StorageType> 
Rect<StorageType>::LeftTop() const
{
	Point<StorageType> result( left, top );
	return (result);
}


template
<
	typename StorageType
>
inline Point<StorageType>
Rect<StorageType>::RightBottom() const
{
	Point<StorageType> result( right, bottom );
	return (result);
}


template
<
	typename StorageType
>
inline Point<StorageType>
Rect<StorageType>::LeftBottom() const
{
	Point<StorageType> result( left, bottom );
	return (result);
}


template
<
	typename StorageType
>
inline Point<StorageType>
Rect<StorageType>::RightTop() const
{
	Point<StorageType> result( right, top );
	return (result);
}


template
<
	typename StorageType
>
inline void Rect<StorageType>::SetLeftTop(const Point<StorageType> leftTop)
{
	left = leftTop.x;
	top = leftTop.y;	
}


template
<
	typename StorageType
>
inline void Rect<StorageType>::SetRightBottom(const Point<StorageType> rightBottom)
{
	right = rightBottom.x;
	bottom = rightBottom.y;
}


template
<
	typename StorageType
>
inline void Rect<StorageType>::SetLeftBottom(const Point<StorageType> leftBottom)
{
	left = leftBottom.x;
	bottom = leftBottom.y;
}


template
<
	typename StorageType
>
inline void Rect<StorageType>::SetRightTop(const Point<StorageType> rightTop)
{
	right = rightTop.x;
	top = rightTop.y;
}


template
<
	typename StorageType
>
inline Rect<StorageType>& 
Rect<StorageType>::operator=(const Rect<StorageType>& from)
{
	left = from.left;
	top = from.top;
	right = from.right;
	bottom = from.bottom;
	
	return (*this);
}


template
<
	typename StorageType
>
inline void Rect<StorageType>::Set(StorageType l, StorageType t, 
	StorageType r, StorageType b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}


template
<
	typename StorageType
>
inline bool Rect<StorageType>::IsValid() const
{
	if (left <= right && top <= bottom)
		return true;
	else
		return false;
}


template
<
	typename StorageType
>
inline int32 Rect<StorageType>::IntegerWidth() const
{
	return ((int32)ceil(right - left));
}


template
<
	typename StorageType
>
inline StorageType Rect<StorageType>::Width() const
{
	return (right - left);
}


template
<
	typename StorageType
>
inline int32 Rect<StorageType>::IntegerHeight() const
{
	return ((int32)ceil(bottom - top));
}


template
<
	typename StorageType
>
inline StorageType Rect<StorageType>::Height() const
{
	return (bottom - top);
}

// ------------------------------------------------
// Type Definitions
// ------------------------------------------------

// typedef Rect<float> BRect;
typedef Rect<int32> XRect;


#endif // EXTENDED_GAME_KIT_RECTANGLE
