/*****************************************************************************/
// PointTest
//
// This is a simple test harness for a generic implementation of the
// BPoint class.
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
#include <Point.h>
#include <Rect.h>
#include "Point.h"

// This is our version of the point class. Let's see
// how it stacks up.
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

// Test Construction.
bool test_construction()
{
	// First test - construction.
	//
	// When no values are supplied, the fields of the point
	// remain uninitualized -> no sense in comparing them.
	const bool test1 = true;
		
	// Second test - value initialization.
	const float xval = 1.3;
	const float yval = 4.6;
	BPoint bPoint2( xval, yval );
	CPoint cPoint2( xval, yval );
	
	bool test2 = compare_points( bPoint2, cPoint2 );
	if( test2 == false )
		cout << "[fault in value initialization] ";
	
	// Third test - copy constructor.
	BPoint bPoint3( bPoint2 );
	CPoint cPoint3( cPoint2 );

	bool test3 = compare_points( bPoint3, cPoint3 );
	if( test3 == false )
		cout << "[fault in copy constructor] ";
	
	// Were we successful?
	bool result = ( test1 && test2 && test3 );
	return( result );
}

// Test Setting.
bool test_setting()
{
	// First test - operator '='.
	const float xval = 87.1345;
	const float yval = -36.23;
	BPoint bPoint1( xval, yval );
	CPoint cPoint1( xval, yval );
	
	BPoint bPoint2 = bPoint1;
	CPoint cPoint2 = cPoint1;
	
	bool test1 = compare_points( bPoint2, cPoint2 );
	if( test1 == false )
		cout << "[fault in '=' operator] ";
	
	// Second test - set method.
	bPoint1.Set( -xval, -yval );
	cPoint1.Set( -xval, -yval );

	bool test2 = compare_points( bPoint1, cPoint1 );
	if( test2 == false )
		cout << "[fault in Set() method] ";
	
	bool result = ( test1 && test2 );
	return( result );
}

// Test Constrain.
bool test_constrain()
{
	const BRect rect( 0.0, 0.0, 100.0, 100.0 );
	
	const float xval1 = 150.0;
	const float yval1 = 150.0;
	
	const float xval2 = -150.0;
	const float yval2 = -150.0;
	
	BPoint bPoint;
	CPoint cPoint;

	// First test - constrain point to rect.
	bPoint.Set( xval1, yval1 );
	bPoint.ConstrainTo( rect );
	cPoint.Set( xval1, yval1 );
	cPoint.ConstrainTo( rect );

	bool test1 = compare_points( bPoint, cPoint );
	if( test1 == false )
	{
		cout << endl << "constrain test 1: " << endl;
		print_point( bPoint );
		print_point( cPoint );
	}

	// Second test - constrain point to rect.
	bPoint.Set( xval2, yval2 );
	bPoint.ConstrainTo( rect );
	cPoint.Set( xval2, yval2 );
	cPoint.ConstrainTo( rect );

	bool test2 = compare_points( bPoint, cPoint );
	if( test2 == false )
	{
		cout << endl << "constrain test 2: " << endl;
		print_point( bPoint );
		print_point( cPoint );
	}
	
	// Third test - point already in rect.
	bPoint.Set( 50.0, 50.0 );	
	bPoint.ConstrainTo( rect );
	cPoint.Set( 50.0, 50.0 );
	cPoint.ConstrainTo( rect );

	bool test3 = compare_points( bPoint, cPoint );
	if( test3 == false )
	{
		cout << endl << "point in rect test: " << endl;
		print_point( bPoint );
		print_point( cPoint );
	}
	
	// Fourth test - point on left corner.
	bPoint.Set( 0.0, 0.0 );
	bPoint.ConstrainTo( rect );
	cPoint.Set( 0.0, 0.0 );
	cPoint.ConstrainTo( rect );

	bool test4 = compare_points( bPoint, cPoint );
	if( test4 == false )
	{
		cout << endl << "point on corner test: " << endl;
		print_point( bPoint );
		print_point( cPoint );
	}
	

	bool result = ( test1 && test2 && test3 && test4 );
	return( result );
}

// Test Modification.
bool test_modification()
{
	BPoint bPoint1( 15.45, 23.99 );
	BPoint bPoint2( -12.3, 97.8 );

	CPoint cPoint1( 15.45, 23.99 );
	CPoint cPoint2( -12.3, 97.8 );

	// Test one - '+' operator.
	BPoint bSumPoint = bPoint1 + bPoint2;
	CPoint cSumPoint = cPoint1 + cPoint2;

	bool test1 = compare_points( bSumPoint, cSumPoint );
	if( test1 == false )
	{
		cout << endl;
		print_point( bSumPoint );
		print_point( cSumPoint );
	}

	// Test two - '-' operator.
	BPoint bDiffPoint = bPoint1 - bPoint2;
	CPoint cDiffPoint = cPoint1 - cPoint2;

	bool test2 = compare_points( bDiffPoint, cDiffPoint );
	if( test2 == false )
	{
		cout << endl;
		print_point( bSumPoint );
		print_point( cSumPoint );
	}

	// Test three - in-place modification.
	bSumPoint += bPoint1;
	cSumPoint += cPoint1;
	
	bool test3 = compare_points( bSumPoint, cSumPoint );
	if( test3 == false )
	{
		cout << endl;
		print_point( bSumPoint );
		print_point( cSumPoint );
	}

	// Test four - in-place modification.
	bDiffPoint -= bPoint1;
	cDiffPoint -= cPoint1;
	
	bool test4 = compare_points( bDiffPoint, cDiffPoint );
	if( test4 == false )
	{
		cout << endl;
		print_point( bDiffPoint );
		print_point( cDiffPoint );
	}


	bool result = ( test1 && test2 && test3 && test4 );
	return( result );
}

// Test Print.
bool test_print()
{
	BPoint bPoint( 0.0, 0.0 );
	CPoint cPoint( 0.0, 0.0 );
	
	cout << endl << endl;
	bPoint.PrintToStream();
	cPoint.PrintToStream();
	cout << endl;
	
	bPoint.Set( 4.5643, 3.4512 );
	bPoint.PrintToStream();
	cPoint.Set( 4.5643, 3.4512 );
	cPoint.PrintToStream();
	cout << endl;

	bPoint.Set( -44.5643, -390.4512 );
	bPoint.PrintToStream();
	cPoint.Set( -44.5643, -390.4512 );
	cPoint.PrintToStream();
	cout << endl;
	
	const bool result = true;
	return( result );
}



// System entry point.
int main()
{
	int errorCount = 0;
	
	cout << "Beginning Point test suite..." << endl;

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

	// Constrain test.
	cout << "* constrain test...";
	if( test_constrain() )
		cout << "passed." << endl;
	else
	{
		cout << "failed." << endl;
		errorCount++;
	}

	// Modification test.
	cout << "* modification test...";
	if( test_modification() )
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