// DirectoryTest.h

#ifndef __sk_directory_test_h__
#define __sk_directory_test_h__

#include <SupportDefs.h>

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

class DirectoryTest : public CppUnit::TestCase
{
public:
	DirectoryTest();

	static Test* Suite();

	// This function called before *each* test added in Suite()
	void setUp();
	
	// This function called after *each* test added in Suite()
	void tearDown();

	// test methods

	void InitTest1();

	void InitTest2();

	void GetEntryTest();

	void IsRootTest();

	void FindEntryTest();

	void ContainsTest();

	void GetStatForTest();

	void EntryIterationTest();

	void EntryCreationTest();

	void AssignmentTest();

	void CreateDirectoryTest();

	// helper functions

	void nextSubTest();

	static void execCommand(const char *command, const char *parameter);
	static void execCommand(const char *command, const char *parameter1,
							const char *parameter2);
	
			int32				subTestNumber;
	static	const char *		existingFilename;
	static	const char *		existingSuperFilename;
	static	const char *		existingRelFilename;
	static	const char *		existingDirname;
	static	const char *		existingSuperDirname;
	static	const char *		existingRelDirname;
	static	const char *		existingSubDirname;
	static	const char *		existingRelSubDirname;
	static	const char *		nonExistingDirname;
	static	const char *		nonExistingSuperDirname;
	static	const char *		nonExistingRelDirname;
	static	const char *		testDirname1;
	static	const char *		tooLongEntryname;
	static	const char *		tooLongSuperEntryname;
	static	const char *		tooLongRelEntryname;
	static	const char *		fileDirname;
	static	const char *		fileSuperDirname;
	static	const char *		fileRelDirname;
	static	const char *		dirLinkname;
	static	const char *		dirSuperLinkname;
	static	const char *		dirRelLinkname;
	static	const char *		fileLinkname;
	static	const char *		fileSuperLinkname;
	static	const char *		fileRelLinkname;
	static	const char *		badLinkname;
	static	const char *		cyclicLinkname1;
	static	const char *		cyclicLinkname2;
	static	const char *		allFilenames[];
};

#endif	// __sk_directory_test_h__
