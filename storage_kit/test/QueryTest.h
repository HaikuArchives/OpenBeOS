// QueryTest.h

#ifndef __sk_query_test_h__
#define __sk_query_test_h__

#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include <StorageDefs.h>
#include <SupportDefs.h>

#include "BasicTest.h"

class QueryTest : public BasicTest
{
public:
	static CppUnit::Test* Suite();
	
	// This function is called before *each* test added in Suite()
	void setUp();
	
	// This function is called after *each* test added in Suite()
	void tearDown();

	//------------------------------------------------------------
	// Test functions
	//------------------------------------------------------------
	void PredicateTest();
	void ParameterTest();
	void FetchTest();
	void LiveTest();

private:
	class QueryApp;
	class QueryHandler;

private:
	QueryApp	*fApplication;
	bool		fVolumeCreated;
};

#endif	// __sk_query_test_h__
