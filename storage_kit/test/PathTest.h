#ifndef __sk_path_test_h__
#define __sk_path_test_h__

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include <Path.h>

class PathTest : public CppUnit::TestCase
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
	void SimpleInitTest();
	
};



#endif	// __sk_path_test_h__
