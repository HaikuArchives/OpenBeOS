//------------------------------------------------------------------------------
//	BHandlerTester.h
//
//------------------------------------------------------------------------------

#ifndef BHANDLERTESTER_H
#define BHANDLERTESTER_H

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------
#include "common.h"

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

class TBHandlerTester : public TestCase
{
	public:
		TBHandlerTester(std::string name) : TestCase(name) {;}

		void Case1();
		void Case2();
		void Case3();
		void Case4();
		void Case5();
		void Case6();
		void Case7();
		void Case8();
		void Case9();
		void Case10();
		void Case11();
		void Case12();
		void Case13();
		void Case14();
		void Case15();
		void Case16();
		void Case17();

		static Test* Suite();
};

#endif	//BHANDLERTESTER_H

/*
 * $Log $
 *
 * $Id  $
 *
 */

