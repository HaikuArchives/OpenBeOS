// SymLinkTest.h

#ifndef __sk_sym_link_test_h__
#define __sk_sym_link_test_h__

#include <SupportDefs.h>

#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include "BasicTest.h"

class SymLinkTest : public BasicTest
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

	void ReadLinkTest();

	void MakeLinkedPathTest();

	void IsAbsoluteTest();

	void AssignmentTest();


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
	static	const char *		relDirLinkname;
	static	const char *		relFileLinkname;
	static	const char *		badLinkname;
	static	const char *		cyclicLinkname1;
	static	const char *		cyclicLinkname2;
	static	const char *		allFilenames[];
	static	const int32 		allFilenameCount;
};

#endif	// __sk_sym_link_test_h__
