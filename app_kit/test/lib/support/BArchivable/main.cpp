//------------------------------------------------------------------------------
//	main.cpp
//
//------------------------------------------------------------------------------

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------
#include "common.h"
#include "BArchivableTester.h"
#include "ValidateInstantiationTester.h"
#include "InstantiateObjectTester.h"

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

class TArchivingTests : TestCase
{
	public:
		TArchivingTests(std::string name) : TestCase(name) {;}
		static Test* Suite()
		{
			TestSuite* tests = new TestSuite;
//			tests->addTest(TBArchivableTestCase::Suite());
			tests->addTest(TValidateInstantiationTest::Suite());
			tests->addTest(TInstantiateObjectTester::Suite());
			return tests;
		}
};

int main()
{
	TestSuite* tests = new TestSuite;
	tests->addTest(TArchivingTests::Suite());
	TextTestResult Result;
	tests->run(&Result);
	cout << Result << endl;

	delete tests;

	return !Result.wasSuccessful();
}


/*
 * $Log $
 *
 * $Id  $
 *
 */

