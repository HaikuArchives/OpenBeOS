// MimeTypeTest.cpp

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>


#include <Application.h>
#include <Mime.h>
#include <MimeTypeTest.h>

#include "Test.StorageKit.h"
#include "TestUtils.h"

static const char *testDir				= "/tmp/mimeTestDir";
static const char *mimeDatabaseDir		= "/boot/home/config/settings/beos_mime";
static const char *testType				= "text/StorageKit-Test";
static const char *testDescr			= "Just a test, nothing more :-)";
static const char *testDescr2			= "Another amazing test string";
static const char *longDescr			= 
"This description is longer than B_MIME_TYPE_LENGTH, which is quite useful for certain things... "
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
"cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
"dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd";


#define CHK CPPUNIT_ASSERT
#define RES DecodeResult

// Suite
CppUnit::Test*
MimeTypeTest::Suite() {
	StorageKit::TestSuite *suite = new StorageKit::TestSuite();
	typedef CppUnit::TestCaller<MimeTypeTest> TC;
		
	suite->addTest( new TC("BMimeType::Long Description Test",
						   &MimeTypeTest::LongDescriptionTest) );
	suite->addTest( new TC("BMimeType::Short Description Test",
						   &MimeTypeTest::ShortDescriptionTest) );

	return suite;
}		

// MimeType App

class MimeTypeTest::MimeTypeApp : public BApplication {
public:
	MimeTypeApp()
		: BApplication("application/x-vnd.obos.mime-type-test"),
		  fAppThread(B_ERROR)
	{
		Unlock();
	}

	status_t Init()
	{
		status_t error = B_OK;
		fAppThread = spawn_thread(&_AppThreadStart, "mime type app",
								  B_NORMAL_PRIORITY, this);
		if (fAppThread < 0)
			error = fAppThread;
		else {
			error = resume_thread(fAppThread);
			if (error != B_OK)
				kill_thread(fAppThread);
		}
		if (error != B_OK)
			fAppThread = B_ERROR;
		return error;
	}

	void Terminate()
	{
		PostMessage(B_QUIT_REQUESTED, this);
		int32 result;
		wait_for_thread(fAppThread, &result);
	}

	virtual void ReadyToRun()
	{
	}

private:
	static int32 _AppThreadStart(void *data)
	{
		if (MimeTypeApp *app = (MimeTypeApp*)data) {
			app->Lock();
			app->Run();
		}
		return 0;
	}

private:
	thread_id		fAppThread;
};


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
	fApplication = new MimeTypeApp;
	if (fApplication->Init() != B_OK) {
		fprintf(stderr, "Failed to initialized application.\n");
		delete fApplication;
		fApplication = NULL;
	}
	// Install our test type
	{
		BMimeType mime(testType);
		status_t err;
//		printf("%s %s valid\n", testType, (mime.IsValid() ? "is" : "is NOT"));
//		printf("%s %s installed\n", testType, (mime.IsInstalled() ? "is" : "is NOT"));
		err = mime.InitCheck();
		if (!err) 
			err = mime.Install();
		if (err || !mime.IsInstalled())
			fprintf(stderr, "Failed to install test type \"%s\"\n", testType);
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

void
MimeTypeTest::ShortDescriptionTest() {
	DescriptionTest(&BMimeType::GetShortDescription, &BMimeType::SetShortDescription);
}

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
			// Note that a subsequent install followed by a setDescr() call with a
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

