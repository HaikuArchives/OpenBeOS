// PathTest.h

#ifndef __sk_path_test_h__
#define __sk_path_test_h__

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include <StorageDefs.h>
#include <SupportDefs.h>

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
	void InitTest1();
	void InitTest2();
	void AppendTest();
	void LeafTest();
	void ParentTest();
	void ComparisonTest();
	void AssignmentTest();
	void FlattenableTest();
	
	// helper functions

	void nextSubTest();

	int32 fSubTestNumber;
	char fCurrentWorkingDir[B_PATH_NAME_LENGTH];
	bool fValidCWD;
};



#endif	// __sk_path_test_h__
