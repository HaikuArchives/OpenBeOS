// MimeTypeTest.cpp

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>


#include <Application.h>
#include <Bitmap.h>
#include <Mime.h>
#include <MimeTypeTest.h>
#include <Path.h>			// Only needed for entry_ref dumps

#include "Test.StorageKit.h"
#include "TestApp.h"
#include "TestUtils.h"

static const char *testDir				= "/tmp/mimeTestDir";
static const char *mimeDatabaseDir		= "/boot/home/config/settings/beos_mime";
static const char *testType				= "text/StorageKit-Test";
static const char *testApp				= "/boot/beos/apps/SoundRecorder";
static const char *testApp2				= "/boot/beos/apps/CDPlayer";
static const char *fakeTestApp			= "/__this_isn't_likely_to_exist__";
// Descriptions
static const char *testDescr			= "Just a test, nothing more :-)";
static const char *testDescr2			= "Another amazing test string";
static const char *longDescr			= 
"This description is longer than B_MIME_TYPE_LENGTH, which is quite useful for certain things... "
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
"cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
"dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd";
// Signatures
static const char *testSig				= "application/x-vnd.obos.mime-type-test";
static const char *testSig2				= "application/x-vnd.obos.mime-type-test-2";
static const char *longSig				= "application/x-vnd.obos.mime-type-test-long."
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
"cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
"dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd";

// Suite
CppUnit::Test*
MimeTypeTest::Suite() {
	StorageKit::TestSuite *suite = new StorageKit::TestSuite();
	typedef CppUnit::TestCaller<MimeTypeTest> TC;
		
	suite->addTest( new TC("BMimeType::App Hint Test",
						   &MimeTypeTest::AppHintTest) );
	suite->addTest( new TC("BMimeType::Long Description Test",
						   &MimeTypeTest::LongDescriptionTest) );
	suite->addTest( new TC("BMimeType::Short Description Test",
						   &MimeTypeTest::ShortDescriptionTest) );
	suite->addTest( new TC("BMimeType::Preferred App Test",
						   &MimeTypeTest::PreferredAppTest) );
	suite->addTest( new TC("BMimeType::Initialization Test",
						   &MimeTypeTest::InitTest) );
//	suite->addTest( new TC("BMimeType::Validity Test",
//						   &MimeTypeTest::ValidityTest) );
//	suite->addTest( new TC("BMimeType::MIME String Test",
//						   &MimeTypeTest::StringTest) );

	return suite;
}		


// setUp
void
MimeTypeTest::setUp()
{
	BasicTest::setUp();
/*	// Better not to play with fire, so we'll make a copy of the
	// local mime database which we'll use for certain OpenBeOS tests
	execCommand(string("mkdir ") + testDir
				+ " ; copyattr -d -r -- " + mimeDatabaseDir + "/* " + testDir
				); */	
	// Setup our application
	fApplication = new TestApp(testSig);
	if (fApplication->Init() != B_OK) {
		fprintf(stderr, "Failed to initialize application.\n");
		delete fApplication;
		fApplication = NULL;
	}
	
}
	
// tearDown
void
MimeTypeTest::tearDown()
{
//	execCommand(string("rm -rf ") + testDir);

	// Uninistall our test type
	BMimeType mime(testType);
	status_t err = mime.InitCheck();
	if (!err && mime.IsInstalled())
		err = mime.Delete();
	if (err)
		fprintf(stderr, "Failed to unistall test type \"%s\"\n", testType);
	// Terminate the Application
	if (fApplication) {
		fApplication->Terminate();
		delete fApplication;
		fApplication = NULL;
	}
	BasicTest::tearDown();
}

// entry_ref dumping function ; this may be removed at any time

void printRef(entry_ref *ref, char* name = "ref") {
	if (ref) {
		BPath path(ref);
		status_t err = path.InitCheck();
		if (!err) {
			printf("%s == '%s'", name, path.Path());
		} else
			printf("%s == ERROR", name);
		printf(" == (%d, %d, '%s')\n", ref->device, ref->directory, ref->name);
		
	} else
		printf("%s == (NULL)\n", name);
}
		
// App Hint

void MimeTypeTest::AppHintTest() {
	// init a couple of entry_refs to applications
	BEntry entry(testApp);
	entry_ref appRef;
	CHK(entry.InitCheck() == B_OK);
	CHK(entry.GetRef(&appRef) == B_OK);	
	BEntry entry2(testApp2);
	entry_ref appRef2;
	CHK(entry2.InitCheck() == B_OK);
	CHK(entry2.GetRef(&appRef2) == B_OK);	
	// Uninitialized
	nextSubTest();
	{
		BMimeType mime;
		entry_ref ref;
		CHK(mime.InitCheck() == B_NO_INIT);
		CHK(mime.GetAppHint(&ref) != B_OK);		// R5 == B_BAD_VALUE
		CHK(mime.SetAppHint(&ref) != B_OK);		// R5 == B_BAD_VALUE
	}
	// Non-installed type
	nextSubTest();
	{
		entry_ref ref;
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Make sure the type isn't installed
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		CHK(!mime.IsInstalled());
		CHK(mime.GetAppHint(&ref) != B_OK);		// R5 == B_ENTRY_NOT_FOUND
		CHK(!mime.IsInstalled());
		CHK(mime.SetAppHint(&appRef) == B_OK);
		CHK(mime.IsInstalled());
		CHK(mime.GetAppHint(&ref) == B_OK);
		CHK(ref == appRef);
	}
	// NULL params
	nextSubTest();
	{
		entry_ref ref;
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Make sure the type isn't installed
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		CHK(!mime.IsInstalled());
		CHK(mime.GetAppHint(NULL) != B_OK);		// R5 == B_BAD_VALUE
		CHK(!mime.IsInstalled());
		CHK(mime.SetAppHint(NULL) != B_OK);		// Installs, R5 == B_ENTRY_NOT_FOUND
		CHK(mime.IsInstalled());
		CHK(mime.GetAppHint(NULL) != B_OK);		// R5 == B_BAD_VALUE
		CHK(mime.SetAppHint(NULL) != B_OK);		// R5 == B_ENTRY_NOT_FOUND
	}
	// Installed Type
	nextSubTest();
	{
		entry_ref ref;
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		CHK(mime.IsInstalled());
		// Get() with no apphint installed
		CHK(mime.GetAppHint(&ref) == B_ENTRY_NOT_FOUND);
		// Initial Set()/Get()
		CHK(mime.SetAppHint(&appRef) == B_OK);
		CHK(mime.GetAppHint(&ref) == B_OK);
		CHK(ref == appRef);
		// Followup Set()/Get()
		CHK(mime.SetAppHint(&appRef2) == B_OK);
		CHK(mime.GetAppHint(&ref) == B_OK);
		CHK(ref == appRef2);
		CHK(ref != appRef);
	}
	// Installed Type, invalid entry_ref
	nextSubTest();
	{
		entry_ref ref(-1, -1, NULL);
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);		
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		CHK(mime.IsInstalled());		
		CHK(mime.SetAppHint(&appRef) == B_OK);
		CHK(mime.SetAppHint(&ref) != B_OK);	// R5 == B_BAD_VALUE
	}
	// Installed Type, fake/invalid entry_ref
	nextSubTest();
	{
		entry_ref ref(0, 0, "__this_ought_not_exist__");
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);		
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);			
		CHK(mime.IsInstalled());		
		CHK(mime.SetAppHint(&appRef) == B_OK);
		CHK(mime.SetAppHint(&ref) != B_OK);	// R5 == B_ENTRY_NOT_FOUND
	}		
	// Installed Type, abstract entry_ref
	nextSubTest();
	{
		entry_ref fakeRef;
		entry_ref ref;
		BEntry entry(fakeTestApp);
		CHK(entry.InitCheck() == B_OK);
		CHK(!entry.Exists());
		CHK(entry.GetRef(&fakeRef) == B_OK);
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);		
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);			
		CHK(mime.IsInstalled());		
		CHK(mime.SetAppHint(&appRef) == B_OK);
		CHK(mime.SetAppHint(&fakeRef) == B_OK);
		CHK(mime.GetAppHint(&ref) == B_OK);
		CHK(ref == fakeRef);
		CHK(ref != appRef);
	}		
}


// Short Description

void
MimeTypeTest::ShortDescriptionTest() {
	DescriptionTest(&BMimeType::GetShortDescription, &BMimeType::SetShortDescription);
}

// Long Description

void
MimeTypeTest::LongDescriptionTest() {
	DescriptionTest(&BMimeType::GetLongDescription, &BMimeType::SetLongDescription);
}

// DescriptionTest Helper Function
void
MimeTypeTest::DescriptionTest(GetDescriptionFunc getDescr, SetDescriptionFunc setDescr) {
	char str[B_MIME_TYPE_LENGTH+1];

	// Uninitialized
	nextSubTest();
	{
		BMimeType mime;
		CPPUNIT_ASSERT(mime.InitCheck() == B_NO_INIT);
		CPPUNIT_ASSERT((mime.*getDescr)(str) != B_OK);	// R5 == B_BAD_VALUE
		CPPUNIT_ASSERT((mime.*setDescr)(str) != B_OK);	// R5 == B_BAD_VALUE
	}	
	// Non-installed type
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Make sure the type isn't installed
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		CHK(!mime.IsInstalled());
		CHK((mime.*getDescr)(str) != B_OK);		// R5 == B_ENTRY_NOT_FOUND
		CHK(!mime.IsInstalled());
		CHK((mime.*setDescr)(testDescr) == B_OK);	// R5 == Installs (but doesn't set), B_OK
		CHK(mime.IsInstalled());
		CHK((mime.*getDescr)(str) == B_OK);
		CHK(strcmp(str, testDescr) == 0);
	}
	// Non-installed type, NULL params
	nextSubTest();
	{
#if !SK_TEST_R5	// NOTE: These tests crash for R5::LongDescription calls but not for R5::ShortDescription
				// calls. Considering the general instability exihibited by most R5 calls when passed
				// NULL pointers, however, I wouldn't suggest it, and thus they aren't even tested here.
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Make sure the type isn't installed
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		CHK(!mime.IsInstalled());
		CHK((mime.*getDescr)(NULL) != B_OK);		// R5 == B_BAD_ADDRESS
		CHK(!mime.IsInstalled());
		CHK((mime.*setDescr)(NULL) != B_OK);		// R5 == Installs (but doesn't set), B_ENTRY_NOT_FOUND
			// Note that a subsequent uninstall followed by a SetShortDescription() call with a
			// valid description succeeds and installs, but DOES NOT actually set.
			// I don't know why, but I'd just suggest not passing NULLs to the
			// R5 versions.
		CHK(mime.IsInstalled());
		CHK((mime.*getDescr)(str) == B_ENTRY_NOT_FOUND);
#endif
	}
	// Installed type
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		// Get() with no description installed
		CHK(mime.IsInstalled());
		CHK((mime.*getDescr)(str) == B_ENTRY_NOT_FOUND);	// R5 == B_ENTRY_NOT_FOUND
		// Initial Set()/Get()
		CHK((mime.*setDescr)(testDescr) == B_OK);
		CHK((mime.*getDescr)(str) == B_OK);
		CHK(strcmp(str, testDescr) == 0);
		// Followup Set()/Get()
		CHK((mime.*setDescr)(testDescr2) == B_OK);
		CHK((mime.*getDescr)(str) == B_OK);
		CHK(strcmp(str, testDescr2) == 0);		
	}
	// Installed Type, Description Too Long
	nextSubTest();
	{
		CHK(strlen(longDescr) > (B_MIME_TYPE_LENGTH+1));
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		// Initial Set()/Get()
		CHK((mime.*setDescr)(longDescr) != B_OK);		// R5 == B_BAD_VALUE
		CHK((mime.*getDescr)(str) == B_ENTRY_NOT_FOUND);
		// Followup Set()/Get()
		CHK((mime.*setDescr)(testDescr) == B_OK);
		CHK((mime.*setDescr)(longDescr) != B_OK);		// R5 == B_BAD_VALUE
		CHK((mime.*getDescr)(str) == B_OK);
		CHK(strcmp(str, testDescr) == 0);		
	}

}


// Preferred App

void
MimeTypeTest::PreferredAppTest() {
	char str[B_MIME_TYPE_LENGTH+1];
	sprintf(str, "%s", testSig);

	// Uninitialized
	nextSubTest();
	{
		BMimeType mime;
		CPPUNIT_ASSERT(mime.InitCheck() == B_NO_INIT);
		CPPUNIT_ASSERT(mime.GetPreferredApp(str) != B_OK);	// R5 == B_BAD_VALUE
		CPPUNIT_ASSERT(mime.SetPreferredApp(str) != B_OK);	// R5 == B_BAD_VALUE
	}	
	// Non-installed type
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Make sure the type isn't installed
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		CHK(!mime.IsInstalled());
		CHK(mime.GetPreferredApp(str) != B_OK);		// R5 == B_ENTRY_NOT_FOUND
		CHK(!mime.IsInstalled());
		CHK(mime.SetPreferredApp(testSig) == B_OK);	// R5 == Installs (but doesn't set), B_OK
		CHK(mime.IsInstalled());
		CHK(mime.GetPreferredApp(str) == B_OK);
		CHK(strcmp(str, testSig) == 0);
	}
	// Non-installed type, NULL params
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Make sure the type isn't installed
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		CHK(!mime.IsInstalled());
		CHK(mime.GetPreferredApp(NULL) != B_OK);		// R5 == B_ENTRY_NOT_FOUND
		CHK(!mime.IsInstalled());
		CHK(mime.SetPreferredApp(NULL) != B_OK);		// R5 == Installs (but doesn't set), B_ENTRY_NOT_FOUND
		CHK(mime.IsInstalled());
		CHK(mime.GetPreferredApp(str) == B_ENTRY_NOT_FOUND);
	}
	// Installed type, NULL params
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		CHK(mime.IsInstalled());
		CHK(mime.GetPreferredApp(NULL) != B_OK);		// R5 == B_BAD_ADDRESS
		CHK(mime.SetPreferredApp(NULL) != B_OK);		// R5 == B_ENTRY_NOT_FOUND
		CHK(mime.GetPreferredApp(NULL) != B_OK);		// R5 == B_BAD_ADDRESS
	}
	// Installed type
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		// Get() with no description installed
		CHK(mime.IsInstalled());
		CHK(mime.GetPreferredApp(str) == B_ENTRY_NOT_FOUND);	// R5 == B_ENTRY_NOT_FOUND
		// Initial Set()/Get()
		CHK(mime.SetPreferredApp(testSig) == B_OK);
		CHK(mime.GetPreferredApp(str) == B_OK);
		CHK(strcmp(str, testSig) == 0);
		// Followup Set()/Get()
		CHK(mime.SetPreferredApp(testSig2) == B_OK);
		CHK(mime.GetPreferredApp(str) == B_OK);
		CHK(strcmp(str, testSig2) == 0);		
	}
	// Installed Type, Signature Too Long
	nextSubTest();
	{
		CHK(strlen(longDescr) > (B_MIME_TYPE_LENGTH+1));
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		// Initial Set()/Get()
		CHK(mime.SetPreferredApp(longSig) != B_OK);		// R5 == B_BAD_VALUE
		CHK(mime.GetPreferredApp(str) == B_ENTRY_NOT_FOUND);
		// Followup Set()/Get()
		CHK(mime.SetPreferredApp(testSig) == B_OK);
		CHK(mime.SetPreferredApp(longSig) != B_OK);		// R5 == B_BAD_VALUE
		CHK(mime.GetPreferredApp(str) == B_OK);
		CHK(strcmp(str, testSig) == 0);		
	}

}

// InitTest
void
MimeTypeTest::InitTest()
{
	// tests:
	// * constructors
	// * SetTo(), SetType()
	// * Unset()
	// * InitCheck()
	// (* Type())

	// We test only a few types here. Exhausting testing is done in
	// ValidityTest().
	const int notTooLongLength = B_MIME_TYPE_LENGTH;
	const int tooLongLength = notTooLongLength + 1;
	const char *validType	= "image/gif";
	const char *validType2	= "application/octet-stream";
	const char *invalidType	= "invalid type";
	char notTooLongType[notTooLongLength + 1];
	char tooLongType[tooLongLength + 1];
	strcpy(notTooLongType, "image/");
	memset(notTooLongType + strlen(notTooLongType), 'a',
		   notTooLongLength - strlen(notTooLongType));
	notTooLongType[notTooLongLength] = '\0';
	strcpy(tooLongType, "image/");
	memset(tooLongType + strlen(tooLongType), 'a',
		   tooLongLength - strlen(tooLongType));
	tooLongType[tooLongLength] = '\0';

	// default constructor
	nextSubTest();
	{
		BMimeType type;
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}

	// BMimeType(const char *)
	// valid type
	nextSubTest();
	{
		BMimeType type(validType);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == validType);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// invalid type
	nextSubTest();
	{
		BMimeType type(invalidType);
		CHK(type.InitCheck() == B_BAD_VALUE);
		CHK(type.Type() == NULL);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// long, but not too long type
	nextSubTest();
	{
		BMimeType type(notTooLongType);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == notTooLongType);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// too long type
	nextSubTest();
	{
		BMimeType type(tooLongType);
		CHK(type.InitCheck() == B_BAD_VALUE);
		CHK(type.Type() == NULL);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	
	// SetTo()
	// valid type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetTo(validType) == B_OK);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == validType);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// invalid type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetTo(invalidType) == B_BAD_VALUE);
		CHK(type.InitCheck() == B_BAD_VALUE);
		CHK(type.Type() == NULL);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// long, but not too long type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetTo(notTooLongType) == B_OK);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == notTooLongType);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// too long type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetTo(tooLongType) == B_BAD_VALUE);
		CHK(type.InitCheck() == B_BAD_VALUE);
		CHK(type.Type() == NULL);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}

	// SetType()
	// valid type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetType(validType) == B_OK);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == validType);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// invalid type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetType(invalidType) == B_BAD_VALUE);
		CHK(type.InitCheck() == B_BAD_VALUE);
		CHK(type.Type() == NULL);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// long, but not too long type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetType(notTooLongType) == B_OK);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == notTooLongType);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// too long type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetType(tooLongType) == B_BAD_VALUE);
		CHK(type.InitCheck() == B_BAD_VALUE);
		CHK(type.Type() == NULL);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	
	// reinitialization
	nextSubTest();
	{
		BMimeType type(validType);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == validType);
		CHK(type.SetTo(validType2) == B_OK);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == validType2);
	}
	// bad args
	nextSubTest();
	{
		BMimeType type(NULL);
		CHK(type.Type() == NULL);
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
		CHK(type.SetTo(NULL) == B_NO_INIT);
		CHK(type.Type() == NULL);
		CHK(type.SetType(NULL) == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
}

// ValidityTest
void
MimeTypeTest::ValidityTest()
{
	// tests:
	// * IsValid() (static/non static)
}

// StringTest
void
MimeTypeTest::StringTest()
{
	// tests:
	// * Type()
	// * IsSupertypeOnly()
	// * GetSupertype()
	// * Contains()
	// * operator==()
}

/* Ingo's functions:

	// initialization
+	BMimeType();
+	BMimeType(const char *mimeType);
(	virtual ~BMimeType();)

+	status_t SetTo(const char *mimeType);
+	status_t SetType(const char *mimeType);
+	void Unset();
+	status_t InitCheck() const;

	// string access
	const char *Type() const;
	bool IsValid() const;
	static bool IsValid(const char *mimeType);
	bool IsSupertypeOnly() const;
	status_t GetSupertype(BMimeType *superType) const;
	bool Contains(const BMimeType *type) const;
	bool operator==(const BMimeType &type) const;
	bool operator==(const char *type) const;

	// MIME database monitoring
	static status_t StartWatching(BMessenger target);
	static status_t StopWatching(BMessenger target);

	// C functions
	int update_mime_info(const char *path, int recursive, int synchronous,
						 int force);
	status_t create_app_meta_mime(const char *path, int recursive,
								  int synchronous, int force);
	status_t get_device_icon(const char *dev, void *icon, int32 size);
*/


/* Tyler's functions:

	// MIME database access
	status_t Install();
	status_t Delete();
	status_t GetIcon(BBitmap *icon, icon_size size) const;
+	status_t GetPreferredApp(char *signature, app_verb verb = B_OPEN) const;
	status_t GetAttrInfo(BMessage *info) const;
	status_t GetFileExtensions(BMessage *extensions) const;
+	status_t GetShortDescription(char *description) const;
+	status_t GetLongDescription(char *description) const;
	status_t GetSupportingApps(BMessage *signatures) const;

	status_t SetIcon(const BBitmap *icon, icon_size size);
+	status_t SetPreferredApp(const char *signature, app_verb verb = B_OPEN);
	status_t SetAttrInfo(const BMessage *info);
	status_t SetFileExtensions(const BMessage *extensions);
+	status_t SetShortDescription(const char *description);
+	status_t SetLongDescription(const char *description);

	static status_t GetInstalledSupertypes(BMessage *super_types);
	static status_t GetInstalledTypes(BMessage *types);
	static status_t GetInstalledTypes(const char *super_type,
									  BMessage *subtypes);
	static status_t GetWildcardApps(BMessage *wild_ones);

	status_t GetAppHint(entry_ref *ref) const;
	status_t SetAppHint(const entry_ref *ref);

	status_t GetIconForType(const char *type, BBitmap *icon,
							icon_size which) const;
	status_t SetIconForType(const char *type, const BBitmap *icon,
							icon_size which);
*/


/* unassigned functions:

	// sniffer rule manipulation
	status_t GetSnifferRule(BString *result) const;
	status_t SetSnifferRule(const char *);
	static status_t CheckSnifferRule(const char *rule, BString *parseError);

	// sniffing
	status_t GuessMimeType(const entry_ref *file, BMimeType *result);
	static status_t GuessMimeType(const void *buffer, int32 length,
								  BMimeType *result);
	static status_t GuessMimeType(const char *filename, BMimeType *result);
*/
