// MimeTypeTest.h

#ifndef __sk_mime_type_test_h__
#define __sk_mime_type_test_h__

#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include "BasicTest.h"
#include <Mime.h>

// Function pointer types for test sharing between {Get,Set}{Short,Long}Description()
typedef status_t (BMimeType::*GetDescriptionFunc)(char* description) const;
typedef status_t (BMimeType::*SetDescriptionFunc)(const char* description);

class MimeTypeTest : public BasicTest {
public:
	static CppUnit::Test* Suite();
	
	// This function called before *each* test added in Suite()
	void setUp();
	
	// This function called after *each* test added in Suite()
	void tearDown();

	//------------------------------------------------------------
	// Test functions
	//------------------------------------------------------------
	void LongDescriptionTest();
	void ShortDescriptionTest();
	void PreferredAppTest();

	//------------------------------------------------------------
	// Helper functions
	//------------------------------------------------------------
	void DescriptionTest(GetDescriptionFunc getDescr, SetDescriptionFunc setDescr);

private:
	class MimeTypeApp;
	MimeTypeApp	*fApplication;
};


#endif	// __sk_mime_type_test_h__
