/*****************************************************************************/
// Point.h
//
// This is our generic implementation of the BPoint class. We need the
// genericity to avoid writing a separate class for points with integer
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

#ifndef EXTENDED_GAME_KIT_POINT
#define EXTENDED_GAME_KIT_POINT

#include <iomanip>
#include <SupportDefs.h>

class BRect;

template
<
	typename StorageType = float
>
class Point {

public:
	StorageType x;
	StorageType y;

	Point();
	Point(StorageType X, StorageType Y);
	Point(const Point<StorageType>& point);
		
	Point& operator=(const Point<StorageType>& from);
	void Set(StorageType X, StorageType Y);

#ifdef TESTING
	void ConstrainTo(BRect rect);
#else
	void ConstrainTo(Rect<StorageType> rect);
#endif

	void PrintToStream() const;
			
	Point operator+(const Point<StorageType>&) const;
	Point operator-(const Point<StorageType>&) const;
	Point& operator+=(const Point<StorageType>&);
	Point&	operator-=(const Point<StorageType>&);

	bool operator!=(const Point<StorageType>&) const;
	bool operator==(const Point<StorageType>&) const;
};


// ------------------------------------------------
// Methods
// ------------------------------------------------

template
<
	typename StorageType
>
void
#ifdef TESTING 
Point<StorageType>::ConstrainTo(BRect rect)
#else
Point<StorageType>::ConstrainTo(Rect<StorageType> rect)
#endif
{
	// Ensures that the BPoint lies within rect. If it's already 
	// contained in the rectangle, the BPoint is unchanged; otherwise, 
	// it's moved to the rect's nearest edge. 

#ifdef TESTING
	const BPoint point( x, y );
	const bool isContained = rect.Contains( point );
#else
	const bool isContained = rect.Contains( *this );
#endif
		
	if( isContained == false )
	{
		if (x < rect.left)
			x = rect.left;
		else if (x > rect.right)
			x = rect.right;
		
		if (y < rect.top)
			y = rect.top;
		else if (y > rect.bottom)
			y = rect.bottom;
	}
}


template
<
	typename StorageType
>
void 
Point<StorageType>::PrintToStream() const
{
	// TO DO
	//
	// Can we determine at compile time what the class name
	// is, and output that when PrintToStream() is called?
	// That would be nice. Should be possible using some
	// template magic, but needs to be worked out first.
	//
	
	// Display one digit after the decimal point.
	cout << setiosflags( ios::fixed )
		 << setprecision( 1 )
		 << "Point(x:" << x << ", y:" << y << ")" << endl;
}
			
template
<
	typename StorageType
>
Point<StorageType> 
Point<StorageType>::operator+(const Point<StorageType>& point) const
{
	Point<StorageType> result;
	result.x = x + point.x;
	result.y = y + point.y;

	return (result);
}


template
<
	typename StorageType
>
Point<StorageType> 
Point<StorageType>::operator-(const Point<StorageType>& point) const
{
	Point<StorageType> result;
	result.x = x - point.x;
	result.y = y - point.y;

	return (result);
}


template
<
	typename StorageType
>
Point<StorageType>& 
Point<StorageType>::operator+=(const Point<StorageType>& point)
{
	x += point.x;
	y += point.y;
	return (*this);
}


template
<
	typename StorageType
>
Point<StorageType>& 
Point<StorageType>::operator-=(const Point<StorageType>& point)
{
	x -= point.x;
	y -= point.y;
	return (*this);
}


template
<
	typename StorageType
>
bool 
Point<StorageType>::operator!=(const Point<StorageType>& point) const
{
	bool result = ( (x != point.x) || (y != point.y) );
	return (result);
}


template
<
	typename StorageType
>
bool 
Point<StorageType>::operator==(const Point<StorageType>& point) const
{
	bool result = ( (x == point.x) && (y == point.y) );
	return (result);
}

// ------------------------------------------------
// Inlines
// ------------------------------------------------

template
<
	typename StorageType
>
inline Point<StorageType>::Point()
{
}


template
<
	typename StorageType
>
inline Point<StorageType>::Point(StorageType X, StorageType Y) :
	x( X ), y( Y )
{
}


template
<
	typename StorageType
>
inline Point<StorageType>::Point(const Point<StorageType>& point) :
	x( point.x ), y( point.y )
{
}


template
<
	typename StorageType
>
inline Point<StorageType>& 
Point<StorageType>::operator=(const Point<StorageType>& from)
{
	x = from.x;
	y = from.y;
	return *this;
}

template
<
	typename StorageType
>
inline void Point<StorageType>::Set(StorageType X, StorageType Y)
{
	x = X;
	y = Y;
}

// ------------------------------------------------
// Type Definitions
// ------------------------------------------------

//typedef Point<float> BPoint;
typedef Point<int32> XPoint;


#endif // EXTENDED_GAME_KIT_POINT
