// FileTest.h

#ifndef __sk_file_test_h__
#define __sk_file_test_h__

#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include "BasicTest.h"

class FileTest : public BasicTest
{
public:
	static Test* Suite();

	// This function called before *each* test added in Suite()
	void setUp();
	
	// This function called after *each* test added in Suite()
	void tearDown();

	// test methods

	void InitTest1();

	void InitTest2();

	void RWAbleTest();

	void RWTest();

	void PositionTest();

	void SizeTest();

	void AssignmentTest();

	// helper functions

	struct InitTestCase {
		const char *	filename;
		uint32			rwmode;
		uint32			createFile;
		uint32			failIfExists;
		uint32			eraseFile;
		bool			removeAfterTest;
		status_t		initCheck;
	};
	
	static	const char *		existingFilename;
	static	const char *		nonExistingFilename;
	static	const char *		testFilename1;
	static	const char *		allFilenames[];
	static	const int32 		allFilenameCount;

	static	const InitTestCase	initTestCases[];
	static	const int32			initTestCasesCount;
};

#endif	// __sk_file_test_h__
