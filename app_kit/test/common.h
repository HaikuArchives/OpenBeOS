//------------------------------------------------------------------------------
//	common.h
//
//------------------------------------------------------------------------------

#ifndef COMMON_H
#define COMMON_H

// Standard Includes -----------------------------------------------------------
#include <posix/string.h>
#include <errno.h>

// System Includes -------------------------------------------------------------

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
		 << ") in " << __PRETTY_FUNCTION__ << endl

#define CHECK_STATUS(status__)											\
	cout << endl << "status_t == \"" << strerror((status__)) << "\" ("	\
		 << (status__) << ") in " << __PRETTY_FUNCTION__ << endl

// Globals ---------------------------------------------------------------------


#endif	//COMMON_H

/*
 * $Log $
 *
 * $Id  $
 *
 */

