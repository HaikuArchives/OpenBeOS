//------------------------------------------------------------------------------
//	LockLooper.h
//
//------------------------------------------------------------------------------

#ifndef LOCKLOOPER_H
#define LOCKLOOPER_H

// Standard Includes -----------------------------------------------------------

// System Includes -------------------------------------------------------------
#if defined(SYSTEM_TEST)
#include <be/app/Handler.h>
#else
#include "../../../../lib/application/headers/Handler.h"
#endif

// Project Includes ------------------------------------------------------------

// Local Includes --------------------------------------------------------------
#include "common.h"

// Local Defines ---------------------------------------------------------------

// Globals ---------------------------------------------------------------------

class TLockLooperTest : public TestCase
{
	public:
		TLockLooperTest(std::string name) : TestCase(name) {;}

		void LockLooper1();
		void LockLooper2();
		void LockLooper3();
		void LockLooper4();

		static Test* Suite();
};

#endif	//LOCKLOOPER_H

/*
 * $Log $
 *
 * $Id  $
 *
 */

