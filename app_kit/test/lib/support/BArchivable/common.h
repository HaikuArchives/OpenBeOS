//------------------------------------------------------------------------------
//	common.h
//
//------------------------------------------------------------------------------

#ifndef COMMON_H
#define COMMON_H

// Standard Includes -----------------------------------------------------------
#include <posix/string.h>

// System Includes -------------------------------------------------------------
#ifdef SYSTEM_TEST
#include <be/support/Archivable.h>
#else
#include "../../../../source/lib/support/headers/Archivable.h"
#endif

// Project Includes ------------------------------------------------------------
#include "framework/TestCaller.h"
#include "framework/TestCase.h"
#include "framework/TestResult.h"
#include "framework/TestSuite.h"
#include "textui/TextTestResult.h"

// Local Includes --------------------------------------------------------------

// Local Defines ---------------------------------------------------------------
#define assert_err(condition)	\
    (this->assertImplementation ((condition), std::string((#condition)) +	\
    	strerror(condition),\
        __LINE__, __FILE__))

#define ADD_TEST(suitename, classname, funcname)				\
	(suitename)->addTest(new TestCaller<classname>((#funcname),	\
						 &classname::funcname));

#define CHECK_ERRNO														\
	cout << endl << "errno == \"" << strerror(errno) << "\" (" << errno	\
		 << ") " << "in " << __PRETTY_FUNCTION__ << endl

// Globals ---------------------------------------------------------------------


#endif	//COMMON_H

/*
 * $Log $
 *
 * $Id  $
 *
 */
