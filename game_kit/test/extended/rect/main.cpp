/*****************************************************************************/
// RectTest
//
// This is a simple test harness for a generic implementation of the
// BRect class.
//
//
// This application and all source files used in its construction, except 
// where noted, are licensed under the MIT License, and have been written 
// and are:
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

#include <iostream>
#include <Rect.h>
#include "Rect.h"

// This is our version of the rect class. Let's see
// how it stacks up. We'll also need our point class.
typedef Rect<float> CRect;
typedef Point<float> CPoint;

// Print a given point to stdout. We don't use PrintToStream()
// because it may be faulty.
void print_point( BPoint& bPoint )
{
	cout << "BPoint: (" << bPoint.x << "," << bPoint.y << ")" << endl;
}

// Print a given point to stdout. We don't use PrintToStream()
// because it may be faulty.
void print_point( CPoint& cPoint )
{
	cout << "CPoint: (" << cPoint.x << "," << cPoint.y << ")" << endl;
}

// Return true if the two points have the same x and y values,
// and false if they're different.
bool compare_points( BPoint& bPoint, CPoint& cPoint )
{
	bool result = false;
	
	if( ( bPoint.x == cPoint.x ) &&
		( bPoint.y == cPoint.y ) )
		result = true;
	
	return( result );
}

// Print a given rect to stdout. We don't use PrintToStream()
// because it may be faulty.
void print_rect( BRect& bRect )
{
	cout << "BRect: (" << bRect.left << "," << bRect.top
	     << "," << bRect.right << "," << bRect.bottom
	     << ")" << endl;
}

// Print a given point to stdout. We don't use PrintToStream()
// because it may be faulty.
void print_rect( CRect& cRect )
{
	cout << "CRect: (" << cRect.left << "," << cRect.top
	     << "," << cRect.right << "," << cRect.bottom
	     << ")" << endl;
}

// Return true if the two points have the same x and y values,
// and false if they're different.
bool compare_rects( BRect& bRect, CRect& cRect )
{
	bool result = false;
	
	if( ( bRect.left == cRect.left ) &&
		( bRect.top == cRect.top ) &&
		( bRect.right == cRect.right ) &&
		( bRect.bottom == cRect.bottom ) )
		result = true;
	
	return( result );
}

// Test Construction.
bool test_construction()
{
	// First test - construction.
	BRect bRect1;
	CRect cRect1;
	
	bool test1 = compare_rects( bRect1, cRect1 );
	if( test1 == false )
		cout << "[fault in construction] ";
		
	// Second test - value initialization.
	const float left = 20.3;
	const float top = 20.9;
	const float right = 340.9;
	const float bottom = 670.0;
	BRect bRect2( left, top, right, bottom );
	CRect cRect2( left, top, right, bottom );
	
	bool test2 = compare_rects( bRect2, cRect2 );
	if( test2 == false )
		cout << "[fault in value initialization] ";
	
	// Third test - copy constructor.
	BRect bRect3( bRect2 );
	CRect cRect3( cRect2 );

	bool test3 = compare_rects( bRect3, cRect3 );
	if( test3 == false )
		cout << "[fault in copy constructor] ";

	// Fourth test - point construction.
	BPoint bPointLeftTop( left, top );
	BPoint bPointRightBottom( right, bottom );
	CPoint cPointLeftTop( left, top );
	CPoint cPointRightBottom( right, bottom );
	
	BRect bRect4( bPointLeftTop, bPointRightBottom );
	CRect cRect4( cPointLeftTop, cPointRightBottom );
	
	bool test4 = compare_rects( bRect4, cRect4 );
	if( test4 == false )
		cout << "[fault in point construction] ";
	
	// Were we successful?
	bool result = ( test1 && test2 && test3 && test4 );
	return( result );
}

// Test Setting.
bool test_setting()
{
	// First test - operator '='.
	const float left = 87.1345;
	const float top = 36.23;
	const float right = 405.342;
	const float bottom = 536.23;
	BRect bRect1( left, top, right, bottom );
	CRect cRect1( left, top, right, bottom );
	
	BRect bRect2 = bRect1;
	CRect cRect2 = cRect1;
	
	bool test1 = compare_rects( bRect2, cRect2 );
	if( test1 == false )
		cout << "[fault in '=' operator] ";
	
	// Second test - set method.
	bRect1.Set( -left, -top, -right, -bottom );
	cRect1.Set( -left, -top, -right, -bottom );

	bool test2 = compare_rects( bRect1, cRect1 );
	if( test2 == false )
		cout << "[fault in Set() method] ";
	
	// Third test - point selectors.
	BPoint bLeftTop = bRect1.LeftTop();
	BPoint bRightBottom = bRect1.RightBottom();
	CPoint cLeftTop = cRect1.LeftTop();
	CPoint cRightBottom = cRect1.RightBottom();	
	
	bool test3 = compare_points( bLeftTop, cLeftTop ) &&
		compare_points( bRightBottom, cRightBottom );
	if( test3 == false )
		cout << "[failure in point selection] ";
	
	bool result = ( test1 && test2 && test3 );
	return( result );
}

// Test Contain.
bool test_contain()
{
	const BRect bRect( 0.0, 0.0, 100.0, 100.0 );
	const CRect cRect( 0.0, 0.0, 100.0, 100.0 );
	
	// A contained point.	
	const float xval1 = 150.0;
	const float yval1 = 150.0;
	
	// An uncontained point.
	const float xval2 = -150.0;
	const float yval2 = -150.0;
	
	BPoint bPoint;
	CPoint cPoint;

	// First test - point containment.
	bPoint.Set( xval1, yval1 );
	cPoint.Set( xval1, yval1 );

	bool test1 = ( bRect.Contains( bPoint ) == cRect.Contains( cPoint ) );
	if( test1 == false )
		cout << "[failure in containment test 1] ";

	// Second test - point not contained.
	bPoint.Set( xval2, yval2 );
	cPoint.Set( xval2, yval2 );

	bool test2 = ( bRect.Contains( bPoint ) == cRect.Contains( cPoint ) );
	if( test2 == false )
		cout << "[failure in containment test 2] ";
	
	// Third test - point on top left corner.
	bPoint.Set( 0.0, 0.0 );	
	cPoint.Set( 0.0, 0.0 );

	bool test3 = ( bRect.Contains( bPoint ) == cRect.Contains( cPoint ) );
	if( test3 == false )
		cout << "[failure in containment test 3] ";
	
	// Fourth test - point on bottom right corner.
	bPoint.Set( 100.0, 100.0 );
	cPoint.Set( 100.0, 100.0 );

	bool test4 = ( bRect.Contains( bPoint ) == cRect.Contains( cPoint ) );
	if( test4 == false )
		cout << "[failure in containment test 4] ";
	
	// Fifth test - rectangle containment.
	BRect bRectA = bRect;
	CRect cRectA = cRect;
	
	bool test5 = ( bRect.Contains( bRectA ) == cRect.Contains( cRectA ) );
	if( test5 == false )
		cout << "[failure in containment test 5] ";

	// Sixth test - rectangle containment.
	bRectA.OffsetBy( 50.0, 0.0 );
	cRectA.OffsetBy( 50.0, 0.0 );

	bool test6 = ( bRect.Contains( bRectA ) == cRect.Contains( cRectA ) );
	if( test6 == false )
		cout << "[failure in containment test 6] ";


	bool result = ( test1 && test2 && test3 && test4 && test5 && test6 );
	return( result );
}

// Test Comparison
bool test_comparison()
{
	CRect cRect1( 5.0, 10.0, 90.1, 120.3 );
	CRect cRect2( 5.0, 10.0, 90.1, 120.3 );

	// Test one - '==' operator.
	bool test1 = ( cRect1 == cRect2 );
	if( test1 == false )
		cout << "[failure in '==' test] ";

	// Test two - '!-' operator.
	cRect2.left += 10.0;
	
	bool test2 = ( cRect1 != cRect2 );
	if( test2 == false )
		cout << "[failure in '!=' test] ";


	bool result = ( test1 && test2 );
	return( result );
}

// Test Transformation.
bool test_transformation()
{
	BRect bRect( 5.0, 10.0, 90.1, 120.3 );
	CRect cRect( 5.0, 10.0, 90.1, 120.3 );

	// Test one - inset by.
	bRect.InsetBy( 5.0, 5.0 );
	cRect.InsetBy( 5.0, 5.0 );
	
	bool test1 = compare_rects( bRect, cRect );
	if( test1 == false )
		cout << "[failure on inset by] ";

	// Test two - inset by point.
	BPoint bPoint( 5.0, 5.0 );
	bRect.InsetBy( bPoint );
	CPoint cPoint( 5.0, 5.0 );
	cRect.InsetBy( cPoint );
	
	bool test2 = compare_rects( bRect, cRect );
	if( test2 == false )
		cout << "[failure on inset by point] ";

	// Test three - offset by.
	bRect.OffsetBy( 5.0, -5.0 );
	cRect.OffsetBy( 5.0, -5.0 );
	
	bool test3 = compare_rects( bRect, cRect );
	if( test3 == false )
		cout << "[failure on offset by] ";

	// Test four - offset by point.
	bRect.OffsetBy( bPoint );
	cRect.OffsetBy( cPoint );
	
	bool test4 = compare_rects( bRect, cRect );
	if( test4 == false )
		cout << "[failure on offset by point] ";

	// Test five - offset to.
	bRect.OffsetTo( 25.1, 34.5 );
	cRect.OffsetTo( 25.1, 34.5 );
	
	bool test5 = compare_rects( bRect, cRect );
	if( test5 == false )
		cout << "[failure on offset to] ";

	// Test six - offset to point.
	bRect.OffsetTo( bPoint );
	cRect.OffsetTo( cPoint );

	bool test6 = compare_rects( bRect, cRect );
	if( test6 == false )
		cout << "[failure on offset to point] ";


	bool result = ( test1 && test2 && test3 && test4 && 
		test5 && test6 );
	return( result );
}

// Test Intersection.
bool test_intersection()
{
	// Test one - non-empty intersection.
	const BRect bRectA( 0.0, 0.0, 100.0, 100.0 );
	BRect bRectB( 50.0, 50.0, 150.0, 150.0 );
	BRect bRectAB = bRectA & bRectB;

	const CRect cRectA( 0.0, 0.0, 100.0, 100.0 );
	CRect cRectB( 50.0, 50.0, 150.0, 150.0 );
	CRect cRectAB = cRectA & cRectB;

	bool test1 = compare_rects( bRectAB, cRectAB );
	if( test1 == false )
		cout << "[failure in intersection 1] ";

	// Test two - containment.
	bRectB.Set( 25.0, 25.0, 75.0, 75.0 );
	cRectB.Set( 25.0, 25.0, 75.0, 75.0 );
	bRectAB = bRectA & bRectB;
	cRectAB = cRectA & cRectB;
	
	bool test2 = compare_rects( bRectAB, cRectAB );
	if( test2 == false )
		cout << "[failure in intersection 2] ";

	// Test three - empty intersection.
	bRectB.Set( 1025.0, 1025.0, 2025.0, 2025.0 );
	cRectB.Set( 1025.0, 1025.0, 2025.0, 2025.0 );
	bRectAB = bRectA & bRectB;
	cRectAB = cRectA & cRectB;
	
	bool test3 = compare_rects( bRectAB, cRectAB );
	if( test3 == false )
		cout << "[failure in intersection 3] ";
	
	// Test four - empty intersection.
	const BRect bRectC( 0.0, 0.0, 100.0, 100.0 );
	BRect bRectD( -150.0, 50.0, -50.0, 150.0 );
	BRect bRectCD = bRectC & bRectD;

	const CRect cRectC( 0.0, 0.0, 100.0, 100.0 );
	CRect cRectD( -150.0, 50.0, -50.0, 150.0 );
	CRect cRectCD = cRectC & cRectD;

	bool test4 = compare_rects( bRectCD, cRectCD );
	if( test4 == false )
		cout << "[failure in intersection 4] ";

	// Test five - non-empty intersection.
	bRectD.OffsetBy( 50.0, 0.0 );
	bRectCD = bRectC & bRectD;
	
	cRectD.OffsetBy( 50.0, 0.0 );
	cRectCD = cRectC & cRectD;

	bool test5 = compare_rects( bRectCD, cRectCD );
	if( test5 == false )
		cout << "[failure in intersection 5] ";

	
	bool result = ( test1 && test2 && test3 && test4 && test5 );
	return( result );
}

// Test Union.
bool test_union()
{
	// Test one.
	const BRect bRectA( 0.0, 0.0, 100.0, 100.0 );
	BRect bRectB( 50.0, 50.0, 150.0, 150.0 );
	BRect bRectAUB = bRectA | bRectB;

	const CRect cRectA( 0.0, 0.0, 100.0, 100.0 );
	CRect cRectB( 50.0, 50.0, 150.0, 150.0 );
	CRect cRectAUB = cRectA | cRectB;

	bool test1 = compare_rects( bRectAUB, cRectAUB );
	if( test1 == false )
		cout << "[failure in union 1] ";

	// Test two.
	bRectB.Set( 25.0, 25.0, 75.0, 75.0 );
	cRectB.Set( 25.0, 25.0, 75.0, 75.0 );
	bRectAUB = bRectA | bRectB;
	cRectAUB = cRectA | cRectB;
	
	bool test2 = compare_rects( bRectAUB, cRectAUB );
	if( test2 == false )
		cout << "[failure in union 2] ";

	// Test three.
	bRectB.Set( 1025.0, 1025.0, 2025.0, 2025.0 );
	cRectB.Set( 1025.0, 1025.0, 2025.0, 2025.0 );
	bRectAUB = bRectA | bRectB;
	cRectAUB = cRectA | cRectB;
	
	bool test3 = compare_rects( bRectAUB, cRectAUB );
	if( test3 == false )
		cout << "[failure in union 3] ";

	// Test four.
	const BRect bRectC( 0.0, 0.0, 100.0, 100.0 );
	BRect bRectD( -150.0, 50.0, -50.0, 150.0 );
	BRect bRectCUD = bRectC | bRectD;

	const CRect cRectC( 0.0, 0.0, 100.0, 100.0 );
	CRect cRectD( -150.0, 50.0, -50.0, 150.0 );
	CRect cRectCUD = cRectC | cRectD;

	bool test4 = compare_rects( bRectCUD, cRectCUD );
	if( test4 == false )
		cout << "[failure in union 4] ";


	bool result = ( test1 && test2 && test3 && test4 );
	return( result );
}

// Test Utility.
bool test_utility()
{
	// Test one - non-empty intersects.
	BRect bRectA( 0.0, 0.0, 100.0, 100.0 );
	BRect bRectB( 50.7, 50.7, 150.987, 150.987 );

	CRect cRectA( 0.0, 0.0, 100.0, 100.0 );
	CRect cRectB( 50.7, 50.7, 150.987, 150.987 );

	bool test1 = ( bRectA.Intersects( bRectB ) == 
		cRectA.Intersects( cRectB ) );
	if( test1 == false )
		cout << "[failure in utility 1] ";

	// Test two - empty intersects.
	bRectB.OffsetBy( 1000.0, 1000.0 );
	cRectB.OffsetBy( 1000.0, 1000.0 );		

	bool test2 = ( bRectA.Intersects( bRectB ) == 
		cRectA.Intersects( cRectB ) );
	if( test2 == false )
		cout << "[failure in utility 2] ";

	// Test three - is valid.
	bRectA.Set( 0,0,0,0 );
	cRectA.Set( 0,0,0,0 );
	
	bool test3 = ( bRectA.IsValid() == cRectA.IsValid() );
	if( test3 == false )
		cout << "[failure in utility 3] ";
	
	// Test four - isn't valid.
	bRectA.Set( 0,0,-1,-1 );
	cRectA.Set( 0,0,-1,-1 );
	
	bool test4 = ( bRectA.IsValid() == cRectA.IsValid() );
	if( test4 == false )
		cout << "[failure in utility 4] ";
	
	// Test five - width and height.
	bool test5 = ( bRectB.Width() == cRectB.Width() ) &&
		( bRectB.Height() == cRectB.Height() );
	if( test5 == false )
		cout << "[failure in utility 5] ";

	// Test six - integer width and integer height.
	bool test6 = ( bRectB.IntegerWidth() == cRectB.IntegerWidth() ) &&
		( bRectB.IntegerHeight() == cRectB.IntegerHeight() );
	if( test6 == false )
		cout << "[failure in utility 6] ";


	bool result = ( test1 && test2 && test3 && test4 && 
		test5 && test6 );
	return( result );
}

// Test Print.
bool test_print()
{
	BRect bRect( 0.0, 0.0, 234.2, 345.2 );
	CRect cRect( 0.0, 0.0, 234.2, 345.2 );
	
	cout << endl << endl;
	bRect.PrintToStream();
	cRect.PrintToStream();
	cout << endl;
	
	bRect.Set( 4.5643, 3.4512, 7.897, 8.232 );
	bRect.PrintToStream();
	cRect.Set( 4.5643, 3.4512, 7.897, 8.232 );
	cRect.PrintToStream();
	cout << endl;

	bRect.Set( -44.5643, -390.4512, -44.5643, -390.4512 );
	bRect.PrintToStream();
	cRect.Set( -44.5643, -390.4512, -44.5643, -390.4512 );
	cRect.PrintToStream();
	cout << endl;
	
	const bool result = true;
	return( result );
}

// System entry point.
int main()
{
	int errorCount = 0;
	
	cout << "Beginning Rect test suite..." << endl;

	// Construction test.	
	cout << "* construction test...";
	if( test_construction() )
		cout << "passed." << endl;
	else
	{
		cout << "failed." << endl;
		errorCount++;
	}

	// Setting test.
	cout << "* setting test...";
	if( test_setting() )
		cout << "passed." << endl;
	else
	{
		cout << "failed." << endl;
		errorCount++;
	}

	// Contain test.
	cout << "* contain test...";
	if( test_contain() )
		cout << "passed." << endl;
	else
	{
		cout << "failed." << endl;
		errorCount++;
	}

	// Comparison test.
	cout << "* comparison test...";
	if( test_comparison() )
		cout << "passed." << endl;
	else
	{
		cout << "failed." << endl;
		errorCount++;
	}

	// Transformation test.
	cout << "* transformation test...";
	if( test_transformation() )
		cout << "passed." << endl;
	else
	{
		cout << "failed." << endl;
		errorCount++;
	}

	// Intersection test.
	cout << "* intersection test...";
	if( test_intersection() )
		cout << "passed." << endl;
	else
	{
		cout << "failed." << endl;
		errorCount++;
	}
	
	// Union test.
	cout << "* union test...";
	if( test_union() )
		cout << "passed." << endl;
	else
	{
		cout << "failed." << endl;
		errorCount++;
	}
	
	// Utility test.
	cout << "* utility test...";
	if( test_utility() )
		cout << "passed." << endl;
	else
	{
		cout << "failed." << endl;
		errorCount++;
	}

	// Print test.
	cout << "* print test...";
	if( test_print() )
		cout << "passed." << endl;
	else
	{
		cout << "failed." << endl;
		errorCount++;
	}

	// Print the final verdict.	
	if( errorCount == 0 )
		cout << "All tests passed." << endl;
	else
	{
		cout << "Unsuccessful. " << errorCount 
			 << " error(s) encountered." << endl;
	}
			 
	return( errorCount );
}