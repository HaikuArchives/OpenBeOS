#include <PathTest.h>

CppUnit::Test*
PathTest::Suite() {
	CppUnit::TestSuite *suite = new CppUnit::TestSuite();
		
	suite->addTest( new CppUnit::TestCaller<PathTest>("BPath::Simple Init Test", &PathTest::SimpleInitTest) );
		
	return suite;
}		

// This function called before *each* test added in Suite()
void
PathTest::setUp() {}
	
// This function called after *each* test added in Suite()
void
PathTest::tearDown() {}

void
PathTest::SimpleInitTest() {
	BPath path("/");
	CPPUNIT_ASSERT( path.InitCheck() == B_OK ); 
}
	
