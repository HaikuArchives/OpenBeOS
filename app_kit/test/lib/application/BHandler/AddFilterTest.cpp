//------------------------------------------------------------------------------
//	AddFilterTest.cpp
//
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#include <be/app/Looper.h>
#if defined(SYSTEM_TEST)
#include <be/app/MessageFilter.h>
#else
#include "../../../../lib/application/headers/MessageFilter.h"
#endif

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------
#include "AddFilterTest.h"

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

//------------------------------------------------------------------------------
/**
	AddFilter(BMessageFilter* filter)
	@case			filter is NULL
	@param	filter	NULL
	@results		None (i.e., no seg faults, etc.)
	@note			Contrary to documentation, BHandler doesn't seem to care if
					it belongs to a BLooper when a filter gets added.  Also,
					the original implementation does not handle a NULL param
					gracefully, so this test is not enabled against it.
 */
void TAddFilterTest::AddFilter1()
{
#if !defined(SYSTEM_TEST)
	BHandler Handler;
	Handler.AddFilter(NULL);
#endif
}
//------------------------------------------------------------------------------
/**
	AddFilter(BMessageFilter* filter)
	@case			filter is valid, handler has no looper
	@param	filter	Valid BMessageFilter pointer
	@results		None (i.e., no seg faults, etc.)
	@note			Contrary to documentation, BHandler doesn't seem to care if
					it belongs to a BLooper when a filter gets added.
 */
void TAddFilterTest::AddFilter2()
{
	BHandler Handler;
	BMessageFilter* Filter = new BMessageFilter('1234');
	Handler.AddFilter(Filter);
}
//------------------------------------------------------------------------------
/**
	AddFilter(BMessageFilter* filter)
	@case			filter is valid, handler has looper, looper isn't locked
	@param	filter	Valid BMessageFilter pointer
	@results		None (i.e., no seg faults, etc.)
	@note			Contrary to documentation, BHandler doesn't seem to care if
					if belongs to a BLooper when a filter gets added, or
					whether the looper is locked.
 */
void TAddFilterTest::AddFilter3()
{
	BLooper Looper;
	BHandler Handler;
	BMessageFilter* Filter = new BMessageFilter('1234');
	Looper.AddHandler(&Handler);
	Handler.AddFilter(Filter);
}
//------------------------------------------------------------------------------
/**
	AddFilter(BMessageFilter* filter)
	@case			filter is valid, handler has looper, looper is locked
	@param	filter	Valid BMessageFilter pointer
	@results		None (i.e., no seg faults, etc.)
 */
void TAddFilterTest::AddFilter4()
{
	BLooper Looper;
	BHandler Handler;
	BMessageFilter* Filter = new BMessageFilter('1234');
	Looper.Lock();
	Looper.AddHandler(&Handler);
	Handler.AddFilter(Filter);
}
//------------------------------------------------------------------------------
Test* TAddFilterTest::Suite()
{
	TestSuite* SuiteOfTests = new TestSuite("BHandler::AddFilter");

	ADD_TEST(SuiteOfTests, TAddFilterTest, AddFilter1);
	ADD_TEST(SuiteOfTests, TAddFilterTest, AddFilter2);
	ADD_TEST(SuiteOfTests, TAddFilterTest, AddFilter3);
	ADD_TEST(SuiteOfTests, TAddFilterTest, AddFilter4);

	return SuiteOfTests;
}
//------------------------------------------------------------------------------

/*
 * $Log $
 *
 * $Id  $
 *
 */

