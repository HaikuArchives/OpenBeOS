//------------------------------------------------------------------------------
//	BMessengerTester.h
//
//------------------------------------------------------------------------------

#ifndef B_MESSENGER_TESTER_H
#define B_MESSENGER_TESTER_H

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------
#include "../common.h"

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

class TBMessengerTester : public TestCase
{
	public:
		TBMessengerTester() {;}
		TBMessengerTester(std::string name) : TestCase(name) {;}

		void BMessenger1();

		static Test* Suite();
};

#endif	// B_MESSENGER_TESTER_H

