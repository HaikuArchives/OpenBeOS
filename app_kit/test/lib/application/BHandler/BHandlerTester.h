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

		void BHandler1();
		void BHandler2();
		void BHandler3();
		void BHandler4();
		void BHandler5();

		void Archive1();
		void Archive2();
		void Archive3();
		void Archive4();

		void Instantiate1();
		void Instantiate2();
		void Instantiate3();

		void SetName1();
		void SetName2();

		void Perform1();

		void IsWatched1();
		void IsWatched2();

		void Looper1();
		void Looper2();

		void SetNextHandler1();
		void SetNextHandler2();
		void SetNextHandler3();
		void SetNextHandler4();
		void SetNextHandler5();
		void SetNextHandler6();
		void SetNextHandler7();
		void SetNextHandler8();
		void SetNextHandler9();
		void SetNextHandler10();
		void SetNextHandler11();

		void NextHandler1();
		void NextHandler2();

		void AddFilter1();
		void AddFilter2();
		void AddFilter3();
		void AddFilter4();

		void RemoveFilter1();
		void RemoveFilter2();
		void RemoveFilter3();
		void RemoveFilter4();
		void RemoveFilter5();
		void RemoveFilter6();
		void RemoveFilter7();

		static Test* Suite();
};

#endif	//BHANDLERTESTER_H

/*
 * $Log $
 *
 * $Id  $
 *
 */

