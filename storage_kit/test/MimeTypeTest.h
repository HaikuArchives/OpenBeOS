// MimeTypeTest.h

#ifndef __sk_mime_type_test_h__
#define __sk_mime_type_test_h__

#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include "BasicTest.h"
#include <Mime.h>

class TestApp;

// Function pointer types for test sharing between {Get,Set}{Short,Long}Description()
typedef status_t (BMimeType::*GetDescriptionFunc)(char* description) const;
typedef status_t (BMimeType::*SetDescriptionFunc)(const char* description);

class IconHelper;
class IconForTypeHelper;

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
	void AppHintTest();
	void FileExtensionsTest();
	void LargeIconTest();
	void MiniIconTest();
	void LargeIconForTypeTest();
	void MiniIconForTypeTest();
	void LongDescriptionTest();
	void ShortDescriptionTest();
	void PreferredAppTest();
	void InstallDeleteTest();

	void InitTest();
	void StringTest();

	//------------------------------------------------------------
	// Helper functions
	//------------------------------------------------------------
	void DescriptionTest(GetDescriptionFunc getDescr, SetDescriptionFunc setDescr);
	void IconTest(IconHelper &helper);
	void IconForTypeTest(IconForTypeHelper &helper);

private:
	TestApp	*fApplication;
};


#endif	// __sk_mime_type_test_h__
