/*
	$Id$
	
	This file declares the addonTestName string and addonTestFunc
	function for the BLocker tests.  These symbols will be used
	when the addon is loaded.
	
	*/
	

#include "AutolockLockerTest.h"
#include "AutolockLooperTest.h"
#include <Autolock.h>
#include "Autolock.h"
#include "TestAddon.h"
#include "TestSuite.h"


// The addonTestName is used to select the test to run or to prompt
// the user which test suite is currently being run.

const char *addonTestName = "BAutolockTests";


/*
 *  Function:  addonTestFunc()
 *     Descr:  This function is called by the test application to
 *             get a pointer to the test to run.  The BLocker test
 *             is a test suite.  A series of tests are added to
 *             the suite.  Each test appears twice, once for
 *             the Be implementation of BLocker, once for the
 *             OpenBeOS implementation.
 */

Test *addonTestFunc(void)
{
	TestSuite *testSuite = new TestSuite("BAutolock");
	
	testSuite->addTest(AutolockLockerTest<BAutolock, BLocker>::suite());
	testSuite->addTest(AutolockLooperTest<BAutolock, BLooper>::suite());
	
	testSuite->addTest(
		AutolockLockerTest<OpenBeOS::BAutolock, OpenBeOS::BLocker>::suite());
	testSuite->addTest(
		AutolockLooperTest<OpenBeOS::BAutolock, BLooper>::suite());
	
	return(testSuite);
}
