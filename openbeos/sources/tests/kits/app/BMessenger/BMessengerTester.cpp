//------------------------------------------------------------------------------
//	BMessengerTester.cpp
//
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#include <be/app/Message.h>
#include <be/kernel/OS.h>

#ifdef SYSTEM_TEST
#include <be/app/Handler.h>
#include <be/app/Looper.h>
#include <be/app/Messenger.h>
#else
#include <Handler.h>
#include <Looper.h>
#include <Messenger.h>
#endif

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------
#include "BMessengerTester.h"

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

//------------------------------------------------------------------------------

/**
	BMessenger()
	@case			
	@results		BMessenger::IsValid() should return false
 */
void TBMessengerTester::BMessenger1()
{
}


Test* TBMessengerTester::Suite()
{
	TestSuite* SuiteOfTests = new TestSuite;

	ADD_TEST(SuiteOfTests, TBMessengerTester, BMessenger1);

	return SuiteOfTests;
}


