// ResourcesTest.h

#ifndef __sk_resources_test_h__
#define __sk_resources_test_h__

#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include <StorageDefs.h>
#include <SupportDefs.h>

#include "BasicTest.h"

class ResourcesTest : public BasicTest
{
public:
	static CppUnit::Test* Suite();
	
	// This function called before *each* test added in Suite()
	void setUp();
	
	// This function called after *each* test added in Suite()
	void tearDown();

	//------------------------------------------------------------
	// Test functions
	//------------------------------------------------------------
	void InitTest1();
};


#endif	// __sk_resources_test_h__
