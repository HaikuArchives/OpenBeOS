// ResourcesTest.cpp

#include <stdio.h>
#include <string>
#include <unistd.h>

#include <ResourcesTest.h>

#include <Directory.h>
#include <Entry.h>
#include <Path.h>
#include "Test.StorageKit.h"

// Suite
CppUnit::Test*
ResourcesTest::Suite() {
	CppUnit::TestSuite *suite = new CppUnit::TestSuite();
	typedef CppUnit::TestCaller<ResourcesTest> TC;
		
	suite->addTest( new TC("BResources::Init Test1",
						   &ResourcesTest::InitTest1) );
	return suite;
}		

// setUp
void
ResourcesTest::setUp()
{
	BasicTest::setUp();
}
	
// tearDown
void
ResourcesTest::tearDown()
{
	BasicTest::tearDown();
}

// InitTest1
void
ResourcesTest::InitTest1()
{
}

