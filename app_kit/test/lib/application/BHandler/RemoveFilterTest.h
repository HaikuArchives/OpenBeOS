//------------------------------------------------------------------------------
//	RemoveFilterTest.h
//
//------------------------------------------------------------------------------

#ifndef REMOVEFILTERTEST_H
#define REMOVEFILTERTEST_H

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#if defined(SYSTEM_TEST)
#include <be/app/Handler.h>
#else
#include "../../../../lib/application/Handler.h"
#endif

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------
#include "common.h"

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

class TRemoveFilterTest : public TestCase
{
	public:
		TRemoveFilterTest(std::string name) : TestCase(name) {;}

		void RemoveFilter1();
		void RemoveFilter2();
		void RemoveFilter3();
		void RemoveFilter4();
		void RemoveFilter5();
		void RemoveFilter6();
		void RemoveFilter7();

		static Test* Suite();
};

#endif	//REMOVEFILTERTEST_H

/*
 * $Log $
 *
 * $Id  $
 *
 */

