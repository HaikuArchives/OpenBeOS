#ifndef __sk_path_test_h__
#define __sk_path_test_h__

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include <Path.h>

class PathTest : public CppUnit::TestCase
{
public:
	static Test* Suite() {
		CppUnit::TestSuite *suite = new CppUnit::TestSuite();
		
		suite->addTest( new CppUnit::TestCaller<PathTest>("BPath::Simple Init Test", &PathTest::SimpleInitTest) );
		
		return suite;
	}		

	// This function called before *each* test added in Suite()
	void setUp() {}
	
	// This function called after *each* test added in Suite()
	void tearDown()	{}

	void SimpleInitTest() {
		BPath path("/");
		CPPUNIT_ASSERT( path.InitCheck() == B_OK ); 
	}
	
};



#endif	// __sk_path_test_h__
