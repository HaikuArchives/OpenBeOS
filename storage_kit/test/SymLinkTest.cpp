// SymLinkTest.cpp

#include <errno.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>

#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <Path.h>
#include <SymLink.h>

#include "Test.StorageKit.h"
#include "SymLinkTest.h"

// Suite
SymLinkTest::Test*
SymLinkTest::Suite()
{
	CppUnit::TestSuite *suite = new CppUnit::TestSuite();
	typedef CppUnit::TestCaller<SymLinkTest> TC;
	
	suite->addTest( new TC("BSymLink::Init Test 1", &SymLinkTest::InitTest1) );
	suite->addTest( new TC("BSymLink::Init Test 2", &SymLinkTest::InitTest2) );
	suite->addTest( new TC("BSymLink::ReadLink Test",
						   &SymLinkTest::ReadLinkTest) );
	suite->addTest( new TC("BSymLink::MakeLinkedPath Test",
						   &SymLinkTest::MakeLinkedPathTest) );
	suite->addTest( new TC("BSymLink::IsAbsolute Test",
						   &SymLinkTest::IsAbsoluteTest) );
	suite->addTest( new TC("BSymLink::Assignment Test",
						   &SymLinkTest::AssignmentTest) );
	
	return suite;
}		

// setUp
void SymLinkTest::setUp()
{
	BasicTest::setUp();
	execCommand(string("touch ") + existingFilename);
	execCommand(string("mkdir ") + existingDirname);
	execCommand(string("mkdir ") + existingSubDirname);
	execCommand(string("ln -s ") + existingDirname + " " + dirLinkname);
	execCommand(string("ln -s ") + existingFilename + " " + fileLinkname);
	execCommand(string("ln -s ") + existingRelDirname + " " + relDirLinkname);
	execCommand(string("ln -s ") + existingRelFilename + " "
				+ relFileLinkname);
	execCommand(string("ln -s ") + nonExistingDirname + " " + badLinkname);
	execCommand(string("ln -s ") + cyclicLinkname1 + " " + cyclicLinkname2);
	execCommand(string("ln -s ") + cyclicLinkname2 + " " + cyclicLinkname1);
}

// tearDown
void SymLinkTest::tearDown()
{
	BasicTest::tearDown();
	// cleanup
	for (int32 i = 0; i < allFilenameCount; i++)
		execCommand(string("rm -rf ") + allFilenames[i]);
}


// InitTest1
void
SymLinkTest::InitTest1()
{
	const char *dirLink = dirLinkname;
	const char *dirSuperLink = dirSuperLinkname;
	const char *dirRelLink = dirRelLinkname;
	const char *fileLink = fileLinkname;
	const char *fileSuperLink = fileSuperLinkname;
	const char *fileRelLink = fileRelLinkname;
	const char *existingDir = existingDirname;
	const char *existingSuperDir = existingSuperDirname;
	const char *existingRelDir = existingRelDirname;
	const char *existingFile = existingFilename;
	const char *existingSuperFile = existingSuperFilename;
	const char *existingRelFile = existingRelFilename;
	const char *nonExisting = nonExistingDirname;
	const char *nonExistingSuper = nonExistingSuperDirname;
	const char *nonExistingRel = nonExistingRelDirname;
	// 1. default constructor
	nextSubTest();
	{
		BSymLink link;
		CPPUNIT_ASSERT( link.InitCheck() == B_NO_INIT );
	}

	// 2. BSymLink(const char*)
	nextSubTest();
	{
		BSymLink link(fileLink);
		CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	}
	nextSubTest();
	{
		BSymLink link(nonExisting);
		CPPUNIT_ASSERT( link.InitCheck() == B_ENTRY_NOT_FOUND );
	}
	nextSubTest();
	{
		BSymLink link((const char *)NULL);
		CPPUNIT_ASSERT( equals(link.InitCheck(), B_BAD_VALUE, B_NO_INIT) );
	}
	nextSubTest();
	{
		BSymLink link("");
		CPPUNIT_ASSERT( link.InitCheck() == B_ENTRY_NOT_FOUND );
	}
	nextSubTest();
	{
		BSymLink link(existingFile);
		CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	}
	nextSubTest();
	{
		BSymLink link(existingDir);
		CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	}
	nextSubTest();
	{
		BSymLink link(tooLongEntryname);
		CPPUNIT_ASSERT( link.InitCheck() == B_NAME_TOO_LONG );
	}

	// 3. BSymLink(const BEntry*)
	nextSubTest();
	{
		BEntry entry(dirLink);
		CPPUNIT_ASSERT( entry.InitCheck() == B_OK );
		BSymLink link(&entry);
		CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	}
	nextSubTest();
	{
		BEntry entry(nonExisting);
		CPPUNIT_ASSERT( entry.InitCheck() == B_OK );
		BSymLink link(&entry);
		CPPUNIT_ASSERT( link.InitCheck() == B_ENTRY_NOT_FOUND );
	}
	nextSubTest();
	{
		BSymLink link((BEntry *)NULL);
		CPPUNIT_ASSERT( link.InitCheck() == B_BAD_VALUE );
	}
	nextSubTest();
	{
		BEntry entry;
		BSymLink link(&entry);
		CPPUNIT_ASSERT( equals(link.InitCheck(), B_BAD_ADDRESS, B_BAD_VALUE) );
	}
	nextSubTest();
	{
		BEntry entry(existingFile);
		CPPUNIT_ASSERT( entry.InitCheck() == B_OK );
		BSymLink link(&entry);
		CPPUNIT_ASSERT( link.InitCheck() == B_OK );

	}
	nextSubTest();
	{
		BEntry entry(existingDir);
		CPPUNIT_ASSERT( entry.InitCheck() == B_OK );
		BSymLink link(&entry);
		CPPUNIT_ASSERT( link.InitCheck() == B_OK );

	}
/* BEntry doesn't seem to check the entry name length.
	nextSubTest();
	{
		BEntry entry(tooLongEntryname);
		// R5 returns E2BIG instead of B_NAME_TOO_LONG
printf("entry.InitCheck(): %x\n", entry.InitCheck());
		CPPUNIT_ASSERT( entry.InitCheck() == E2BIG );
		BSymLink link(&entry);
		CPPUNIT_ASSERT( equals(link.InitCheck(), B_BAD_ADDRESS, B_BAD_VALUE) );
	}
*/

	// 4. BSymLink(const entry_ref*)
	nextSubTest();
	{
		BEntry entry(dirLink);
		CPPUNIT_ASSERT( entry.InitCheck() == B_OK );
		entry_ref ref;
		CPPUNIT_ASSERT( entry.GetRef(&ref) == B_OK );
		BSymLink link(&ref);
		CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	}
	nextSubTest();
	{
		BEntry entry(nonExisting);
		CPPUNIT_ASSERT( entry.InitCheck() == B_OK );
		entry_ref ref;
		CPPUNIT_ASSERT( entry.GetRef(&ref) == B_OK );
		BSymLink link(&ref);
		CPPUNIT_ASSERT( link.InitCheck() == B_ENTRY_NOT_FOUND );
	}
	nextSubTest();
	{
		BSymLink link((entry_ref *)NULL);
		CPPUNIT_ASSERT( link.InitCheck() == B_BAD_VALUE );
	}
	nextSubTest();
	{
		BEntry entry(existingFile);
		CPPUNIT_ASSERT( entry.InitCheck() == B_OK );
		entry_ref ref;
		CPPUNIT_ASSERT( entry.GetRef(&ref) == B_OK );
		BSymLink link(&ref);
		CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	}
	nextSubTest();
	{
		BEntry entry(existingDir);
		CPPUNIT_ASSERT( entry.InitCheck() == B_OK );
		entry_ref ref;
		CPPUNIT_ASSERT( entry.GetRef(&ref) == B_OK );
		BSymLink link(&ref);
		CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	}

	// 5. BSymLink(const BDirectory*, const char*)
	nextSubTest();
	{
		BDirectory pathDir(dirSuperLink);
		CPPUNIT_ASSERT( pathDir.InitCheck() == B_OK );
		BSymLink link(&pathDir, dirRelLink);
		CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	}
	nextSubTest();
	{
		BDirectory pathDir(dirSuperLink);
		CPPUNIT_ASSERT( pathDir.InitCheck() == B_OK );
		BSymLink link(&pathDir, dirLink);
		CPPUNIT_ASSERT( link.InitCheck() == B_BAD_VALUE );
	}
	nextSubTest();
	{
		BDirectory pathDir(nonExistingSuper);
		CPPUNIT_ASSERT( pathDir.InitCheck() == B_OK );
		BSymLink link(&pathDir, nonExistingRel);
		CPPUNIT_ASSERT( link.InitCheck() == B_ENTRY_NOT_FOUND );
	}
	nextSubTest();
	{
		BSymLink link((BDirectory *)NULL, (const char *)NULL);
		CPPUNIT_ASSERT( link.InitCheck() == B_BAD_VALUE );
	}
	nextSubTest();
	{
		BSymLink link((BDirectory *)NULL, dirLink);
		CPPUNIT_ASSERT( link.InitCheck() == B_BAD_VALUE );
	}
	nextSubTest();
	{
		BDirectory pathDir(dirSuperLink);
		CPPUNIT_ASSERT( pathDir.InitCheck() == B_OK );
		BSymLink link(&pathDir, (const char *)NULL);
		CPPUNIT_ASSERT( link.InitCheck() == B_BAD_VALUE );
	}
	nextSubTest();
	{
		BDirectory pathDir(dirSuperLink);
		CPPUNIT_ASSERT( pathDir.InitCheck() == B_OK );
		BSymLink link(&pathDir, "");
		CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	}
	nextSubTest();
	{
		BDirectory pathDir(existingSuperFile);
		CPPUNIT_ASSERT( pathDir.InitCheck() == B_OK );
		BSymLink link(&pathDir, existingRelFile);
		CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	}
	nextSubTest();
	{
		BDirectory pathDir(existingSuperDir);
		CPPUNIT_ASSERT( pathDir.InitCheck() == B_OK );
		BSymLink link(&pathDir, existingRelDir);
		CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	}
/* BEntry doesn't seem to check the entry name length.
	nextSubTest();
	{
		BDirectory pathDir(tooLongSuperEntryname);
		CPPUNIT_ASSERT( pathDir.InitCheck() == B_OK );
		BSymLink link(&pathDir, tooLongRelEntryname);
printf("link.InitCheck(): %x\n", link.InitCheck());
		CPPUNIT_ASSERT( link.InitCheck() == B_NAME_TOO_LONG );
	}
*/
	nextSubTest();
	{
		BDirectory pathDir(fileSuperDirname);
		CPPUNIT_ASSERT( pathDir.InitCheck() == B_OK );
		BSymLink link(&pathDir, fileRelDirname);
		CPPUNIT_ASSERT( link.InitCheck() == B_ENTRY_NOT_FOUND );
	}
}

// InitTest2
void
SymLinkTest::InitTest2()
{
	const char *dirLink = dirLinkname;
	const char *dirSuperLink = dirSuperLinkname;
	const char *dirRelLink = dirRelLinkname;
	const char *fileLink = fileLinkname;
	const char *fileSuperLink = fileSuperLinkname;
	const char *fileRelLink = fileRelLinkname;
	const char *existingDir = existingDirname;
	const char *existingSuperDir = existingSuperDirname;
	const char *existingRelDir = existingRelDirname;
	const char *existingFile = existingFilename;
	const char *existingSuperFile = existingSuperFilename;
	const char *existingRelFile = existingRelFilename;
	const char *nonExisting = nonExistingDirname;
	const char *nonExistingSuper = nonExistingSuperDirname;
	const char *nonExistingRel = nonExistingRelDirname;
	BSymLink link;
	// 2. BSymLink(const char*)
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(fileLink) == B_OK );
	CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	//
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(nonExisting) == B_ENTRY_NOT_FOUND );
	CPPUNIT_ASSERT( link.InitCheck() == B_ENTRY_NOT_FOUND );
	//
	nextSubTest();
	CPPUNIT_ASSERT( equals(link.SetTo((const char *)NULL), B_BAD_VALUE,
						   B_NO_INIT) );
	CPPUNIT_ASSERT( equals(link.InitCheck(), B_BAD_VALUE, B_NO_INIT) );
	//
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo("") == B_ENTRY_NOT_FOUND );
	CPPUNIT_ASSERT( link.InitCheck() == B_ENTRY_NOT_FOUND );
	//
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(existingFile) == B_OK );
	CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	//
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(existingDir) == B_OK );
	CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	//
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(tooLongEntryname) == B_NAME_TOO_LONG );
	CPPUNIT_ASSERT( link.InitCheck() == B_NAME_TOO_LONG );

	// 3. BSymLink(const BEntry*)
	nextSubTest();
	BEntry entry(dirLink);
	CPPUNIT_ASSERT( entry.InitCheck() == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&entry) == B_OK );
	CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	//
	nextSubTest();
	CPPUNIT_ASSERT( entry.SetTo(nonExisting) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&entry) == B_ENTRY_NOT_FOUND );
	CPPUNIT_ASSERT( link.InitCheck() == B_ENTRY_NOT_FOUND );
	//
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo((BEntry *)NULL) == B_BAD_VALUE );
	CPPUNIT_ASSERT( link.InitCheck() == B_BAD_VALUE );
	//
	nextSubTest();
	entry.Unset();
	CPPUNIT_ASSERT( entry.InitCheck() == B_NO_INIT );
	CPPUNIT_ASSERT( equals(link.SetTo(&entry), B_BAD_ADDRESS, B_BAD_VALUE) );
	CPPUNIT_ASSERT( equals(link.InitCheck(), B_BAD_ADDRESS, B_BAD_VALUE) );
	//
	nextSubTest();
	CPPUNIT_ASSERT( entry.SetTo(existingFile) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&entry) == B_OK );
	CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	//
	nextSubTest();
	CPPUNIT_ASSERT( entry.SetTo(existingDir) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&entry) == B_OK );
	CPPUNIT_ASSERT( link.InitCheck() == B_OK );
/* BEntry doesn't seem to check the entry name length.
	//
	nextSubTest();
	BEntry entry(tooLongEntryname);
	// R5 returns E2BIG instead of B_NAME_TOO_LONG
	CPPUNIT_ASSERT( entry.SetTo(tooLongEntryname) == E2BIG );
printf("entry.InitCheck(): %x\n", entry.InitCheck());
	CPPUNIT_ASSERT( equals(link.SetTo(&entry), B_BAD_ADDRESS, B_BAD_VALUE) );
	CPPUNIT_ASSERT( equals(link.InitCheck(), B_BAD_ADDRESS, B_BAD_VALUE) );
*/

	// 4. BSymLink(const entry_ref*)
	nextSubTest();
	CPPUNIT_ASSERT( entry.SetTo(dirLink) == B_OK );
	entry_ref ref;
	CPPUNIT_ASSERT( entry.GetRef(&ref) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&ref) == B_OK );
	CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	//
	nextSubTest();
	CPPUNIT_ASSERT( entry.SetTo(nonExisting) == B_OK );
	CPPUNIT_ASSERT( entry.GetRef(&ref) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&ref) == B_ENTRY_NOT_FOUND );
	CPPUNIT_ASSERT( link.InitCheck() == B_ENTRY_NOT_FOUND );
	//
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo((entry_ref *)NULL) == B_BAD_VALUE );
	CPPUNIT_ASSERT( link.InitCheck() == B_BAD_VALUE );
	//
	nextSubTest();
	CPPUNIT_ASSERT( entry.SetTo(existingFile) == B_OK );
	CPPUNIT_ASSERT( entry.GetRef(&ref) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&ref) == B_OK );
	CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	//
	nextSubTest();
	CPPUNIT_ASSERT( entry.SetTo(existingDir) == B_OK );
	CPPUNIT_ASSERT( entry.GetRef(&ref) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&ref) == B_OK );
	CPPUNIT_ASSERT( link.InitCheck() == B_OK );

	// 5. BSymLink(const BDirectory*, const char*)
	nextSubTest();
	BDirectory pathDir(dirSuperLink);
	CPPUNIT_ASSERT( pathDir.InitCheck() == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&pathDir, dirRelLink) == B_OK );
	CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	//
	nextSubTest();
	CPPUNIT_ASSERT( pathDir.SetTo(dirSuperLink) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&pathDir, dirLink) == B_BAD_VALUE );
	CPPUNIT_ASSERT( link.InitCheck() == B_BAD_VALUE );
	//
	nextSubTest();
	CPPUNIT_ASSERT( pathDir.SetTo(nonExistingSuper) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&pathDir, nonExistingRel) == B_ENTRY_NOT_FOUND );
	CPPUNIT_ASSERT( link.InitCheck() == B_ENTRY_NOT_FOUND );
	//
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo((BDirectory *)NULL, (const char *)NULL)
					== B_BAD_VALUE );
	CPPUNIT_ASSERT( link.InitCheck() == B_BAD_VALUE );
	//
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo((BDirectory *)NULL, dirLink) == B_BAD_VALUE );
	CPPUNIT_ASSERT( link.InitCheck() == B_BAD_VALUE );
	//
	nextSubTest();
	CPPUNIT_ASSERT( pathDir.SetTo(dirSuperLink) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&pathDir, (const char *)NULL) == B_BAD_VALUE );
	CPPUNIT_ASSERT( link.InitCheck() == B_BAD_VALUE );
	//
	nextSubTest();
	CPPUNIT_ASSERT( pathDir.SetTo(dirSuperLink) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&pathDir, "") == B_OK );
	CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	//
	nextSubTest();
	CPPUNIT_ASSERT( pathDir.SetTo(existingSuperFile) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&pathDir, existingRelFile) == B_OK );
	CPPUNIT_ASSERT( link.InitCheck() == B_OK );
	//
	nextSubTest();
	CPPUNIT_ASSERT( pathDir.SetTo(existingSuperDir) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&pathDir, existingRelDir) == B_OK );
	CPPUNIT_ASSERT( link.InitCheck() == B_OK );
/* BEntry doesn't seem to check the entry name length.
	//
	nextSubTest();
	CPPUNIT_ASSERT( pathDir.SetTo(tooLongSuperEntryname) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&pathDir, tooLongRelEntryname) == B_OK );
printf("link.InitCheck(): %x\n", link.InitCheck());
	CPPUNIT_ASSERT( link.InitCheck() == B_NAME_TOO_LONG );
*/
	//
	nextSubTest();
	CPPUNIT_ASSERT( pathDir.SetTo(fileSuperDirname) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(&pathDir, fileRelDirname) == B_ENTRY_NOT_FOUND );
	CPPUNIT_ASSERT( link.InitCheck() == B_ENTRY_NOT_FOUND );
}

// ReadLinkTest
void
SymLinkTest::ReadLinkTest()
{
	const char *dirLink = dirLinkname;
	const char *dirSuperLink = dirSuperLinkname;
	const char *dirRelLink = dirRelLinkname;
	const char *fileLink = fileLinkname;
	const char *fileSuperLink = fileSuperLinkname;
	const char *fileRelLink = fileRelLinkname;
	const char *badLink = badLinkname;
	const char *cyclicLink1 = cyclicLinkname1;
	const char *cyclicLink2 = cyclicLinkname2;
	const char *existingDir = existingDirname;
	const char *existingSuperDir = existingSuperDirname;
	const char *existingRelDir = existingRelDirname;
	const char *existingFile = existingFilename;
	const char *existingSuperFile = existingSuperFilename;
	const char *existingRelFile = existingRelFilename;
	const char *nonExisting = nonExistingDirname;
	const char *nonExistingSuper = nonExistingSuperDirname;
	const char *nonExistingRel = nonExistingRelDirname;
	BSymLink link;
	char buffer[B_PATH_NAME_LENGTH + 1];
	// uninitialized
	// R5: returns B_BAD_ADDRESS instead of (as doc'ed) B_FILE_ERROR
	nextSubTest();
	CPPUNIT_ASSERT( link.InitCheck() == B_NO_INIT );
	CPPUNIT_ASSERT( equals(link.ReadLink(buffer, sizeof(buffer)),
						   B_BAD_ADDRESS, B_FILE_ERROR) );
	// existing dir link
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(dirLink) == B_OK );
	CPPUNIT_ASSERT( link.ReadLink(buffer, sizeof(buffer))
					== strlen(existingDir) );
	CPPUNIT_ASSERT( strcmp(buffer, existingDir) == 0 );
	// existing file link
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(fileLink) == B_OK );
	CPPUNIT_ASSERT( link.ReadLink(buffer, sizeof(buffer))
					== strlen(existingFile) );
	CPPUNIT_ASSERT( strcmp(buffer, existingFile) == 0 );
	// existing cyclic link
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(cyclicLink1) == B_OK );
	CPPUNIT_ASSERT( link.ReadLink(buffer, sizeof(buffer))
					== strlen(cyclicLink2) );
	CPPUNIT_ASSERT( strcmp(buffer, cyclicLink2) == 0 );
	// existing link to non-existing entry
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(badLink) == B_OK );
	CPPUNIT_ASSERT( link.ReadLink(buffer, sizeof(buffer))
					== strlen(nonExisting) );
	CPPUNIT_ASSERT( strcmp(buffer, nonExisting) == 0 );
	// non-existing link
	// R5: returns B_BAD_ADDRESS instead of (as doc'ed) B_FILE_ERROR
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(nonExisting) == B_ENTRY_NOT_FOUND );
	CPPUNIT_ASSERT( equals(link.ReadLink(buffer, sizeof(buffer)),
						   B_BAD_ADDRESS, B_FILE_ERROR) );
	// dir
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(existingDir) == B_OK );
	CPPUNIT_ASSERT( link.ReadLink(buffer, sizeof(buffer)) == B_BAD_VALUE );
	// file
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(existingFile) == B_OK );
	CPPUNIT_ASSERT( link.ReadLink(buffer, sizeof(buffer)) == B_BAD_VALUE );
	// small buffer
	// R5: returns the size of the contents, not the number of bytes copied
	// OBOS: ... so do we
	nextSubTest();
	char smallBuffer[2];
	CPPUNIT_ASSERT( link.SetTo(dirLink) == B_OK );
	CPPUNIT_ASSERT( link.ReadLink(smallBuffer, sizeof(smallBuffer))
//					== sizeof(smallBuffer) );
					== strlen(dirLink) );
	CPPUNIT_ASSERT( strncmp(smallBuffer, existingDir, sizeof(smallBuffer)) == 0 );
	// bad args
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(fileLink) == B_OK );
	CPPUNIT_ASSERT( equals(link.ReadLink(NULL, sizeof(buffer)), B_BAD_ADDRESS,
						   B_BAD_VALUE) );
}

// MakeLinkedPathTest
void
SymLinkTest::MakeLinkedPathTest()
{
	const char *dirLink = dirLinkname;
	const char *dirSuperLink = dirSuperLinkname;
	const char *dirRelLink = dirRelLinkname;
	const char *fileLink = fileLinkname;
	const char *fileSuperLink = fileSuperLinkname;
	const char *fileRelLink = fileRelLinkname;
	const char *relDirLink = relDirLinkname;
	const char *relFileLink = relFileLinkname;
	const char *badLink = badLinkname;
	const char *cyclicLink1 = cyclicLinkname1;
	const char *cyclicLink2 = cyclicLinkname2;
	const char *existingDir = existingDirname;
	const char *existingSuperDir = existingSuperDirname;
	const char *existingRelDir = existingRelDirname;
	const char *existingFile = existingFilename;
	const char *existingSuperFile = existingSuperFilename;
	const char *existingRelFile = existingRelFilename;
	const char *nonExisting = nonExistingDirname;
	const char *nonExistingSuper = nonExistingSuperDirname;
	const char *nonExistingRel = nonExistingRelDirname;
	BSymLink link;
	BPath path;
	// 1. MakeLinkedPath(const char*, BPath*)
	// uninitialized
	nextSubTest();
	CPPUNIT_ASSERT( link.InitCheck() == B_NO_INIT );
	CPPUNIT_ASSERT( equals(link.MakeLinkedPath("/boot", &path), B_BAD_ADDRESS,
						   B_FILE_ERROR) );
	link.Unset();
	path.Unset();
	// existing absolute dir link
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(dirLink) == B_OK );
	CPPUNIT_ASSERT( link.MakeLinkedPath("/boot", &path) == strlen(existingDir) );
	CPPUNIT_ASSERT( path.InitCheck() == B_OK );
	CPPUNIT_ASSERT( string(existingDir) == path.Path() );
	link.Unset();
	path.Unset();
	// existing absolute file link
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(fileLink) == B_OK );
	CPPUNIT_ASSERT( link.MakeLinkedPath("/boot", &path) == strlen(existingFile) );
	CPPUNIT_ASSERT( path.InitCheck() == B_OK );
	CPPUNIT_ASSERT( string(existingFile) == path.Path() );
	link.Unset();
	path.Unset();
	// existing absolute cyclic link
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(cyclicLink1) == B_OK );
	CPPUNIT_ASSERT( link.MakeLinkedPath("/boot", &path) == strlen(cyclicLink2) );
	CPPUNIT_ASSERT( path.InitCheck() == B_OK );
	CPPUNIT_ASSERT( string(cyclicLink2) == path.Path() );
	link.Unset();
	path.Unset();
	// existing relative dir link
	nextSubTest();
	BEntry entry;
	BPath entryPath;
	CPPUNIT_ASSERT( entry.SetTo(existingDir) == B_OK );
	CPPUNIT_ASSERT( entry.GetPath(&entryPath) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(relDirLink) == B_OK );
	CPPUNIT_ASSERT( link.MakeLinkedPath(existingSuperDir, &path)
					== strlen(entryPath.Path()) );
	CPPUNIT_ASSERT( path.InitCheck() == B_OK );
	CPPUNIT_ASSERT( entryPath == path );
	link.Unset();
	path.Unset();
	entry.Unset();
	entryPath.Unset();
	// existing relative file link
	nextSubTest();
	CPPUNIT_ASSERT( entry.SetTo(existingFile) == B_OK );
	CPPUNIT_ASSERT( entry.GetPath(&entryPath) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(relFileLink) == B_OK );
	CPPUNIT_ASSERT( link.MakeLinkedPath(existingSuperFile, &path)
					== strlen(entryPath.Path()) );
	CPPUNIT_ASSERT( path.InitCheck() == B_OK );
	CPPUNIT_ASSERT( entryPath == path );
	link.Unset();
	path.Unset();
	entry.Unset();
	entryPath.Unset();
	// bad args
	// R5: crashs, when passing a NULL path
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(dirLink) == B_OK );
//	CPPUNIT_ASSERT( link.MakeLinkedPath("/boot", NULL) == B_BAD_VALUE );
	CPPUNIT_ASSERT( link.MakeLinkedPath((const char*)NULL, &path)
					== B_BAD_VALUE );
//	CPPUNIT_ASSERT( link.MakeLinkedPath((const char*)NULL, NULL)
//					== B_BAD_VALUE );
	link.Unset();
	path.Unset();

	// 2. MakeLinkedPath(const BDirectory*, BPath*)
	// uninitialized
	nextSubTest();
	link.Unset();
	CPPUNIT_ASSERT( link.InitCheck() == B_NO_INIT );
	BDirectory dir;
	CPPUNIT_ASSERT( dir.SetTo("/boot") == B_OK);
	CPPUNIT_ASSERT( equals(link.MakeLinkedPath(&dir, &path), B_BAD_ADDRESS,
						   B_FILE_ERROR) );
	link.Unset();
	path.Unset();
	dir.Unset();
	// existing absolute dir link
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(dirLink) == B_OK );
	CPPUNIT_ASSERT( dir.SetTo("/boot") == B_OK);
	CPPUNIT_ASSERT( link.MakeLinkedPath(&dir, &path) == strlen(existingDir) );
	CPPUNIT_ASSERT( path.InitCheck() == B_OK );
	CPPUNIT_ASSERT( string(existingDir) == path.Path() );
	link.Unset();
	path.Unset();
	dir.Unset();
	// existing absolute file link
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(fileLink) == B_OK );
	CPPUNIT_ASSERT( dir.SetTo("/boot") == B_OK);
	CPPUNIT_ASSERT( link.MakeLinkedPath(&dir, &path) == strlen(existingFile) );
	CPPUNIT_ASSERT( path.InitCheck() == B_OK );
	CPPUNIT_ASSERT( string(existingFile) == path.Path() );
	link.Unset();
	path.Unset();
	dir.Unset();
	// existing absolute cyclic link
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(cyclicLink1) == B_OK );
	CPPUNIT_ASSERT( dir.SetTo("/boot") == B_OK);
	CPPUNIT_ASSERT( link.MakeLinkedPath(&dir, &path) == strlen(cyclicLink2) );
	CPPUNIT_ASSERT( path.InitCheck() == B_OK );
	CPPUNIT_ASSERT( string(cyclicLink2) == path.Path() );
	link.Unset();
	path.Unset();
	dir.Unset();
	// existing relative dir link
	nextSubTest();
	CPPUNIT_ASSERT( entry.SetTo(existingDir) == B_OK );
	CPPUNIT_ASSERT( entry.GetPath(&entryPath) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(relDirLink) == B_OK );
	CPPUNIT_ASSERT( dir.SetTo(existingSuperDir) == B_OK);
	CPPUNIT_ASSERT( link.MakeLinkedPath(&dir, &path)
					== strlen(entryPath.Path()) );
	CPPUNIT_ASSERT( path.InitCheck() == B_OK );
	CPPUNIT_ASSERT( entryPath == path );
	link.Unset();
	path.Unset();
	dir.Unset();
	entry.Unset();
	entryPath.Unset();
	// existing relative file link
	nextSubTest();
	CPPUNIT_ASSERT( entry.SetTo(existingFile) == B_OK );
	CPPUNIT_ASSERT( entry.GetPath(&entryPath) == B_OK );
	CPPUNIT_ASSERT( link.SetTo(relFileLink) == B_OK );
	CPPUNIT_ASSERT( dir.SetTo(existingSuperFile) == B_OK);
	CPPUNIT_ASSERT( link.MakeLinkedPath(&dir, &path)
					== strlen(entryPath.Path()) );
	CPPUNIT_ASSERT( path.InitCheck() == B_OK );
	CPPUNIT_ASSERT( entryPath == path );
	link.Unset();
	path.Unset();
	dir.Unset();
	entry.Unset();
	entryPath.Unset();
	// absolute link, uninitialized dir
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(dirLink) == B_OK );
	CPPUNIT_ASSERT( dir.InitCheck() == B_NO_INIT);
	CPPUNIT_ASSERT( link.MakeLinkedPath(&dir, &path) == strlen(existingDir) );
	CPPUNIT_ASSERT( path.InitCheck() == B_OK );
	CPPUNIT_ASSERT( string(existingDir) == path.Path() );
	// absolute link, badly initialized dir
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(dirLink) == B_OK );
	CPPUNIT_ASSERT( dir.SetTo(nonExisting) == B_ENTRY_NOT_FOUND);
	CPPUNIT_ASSERT( link.MakeLinkedPath(&dir, &path) == strlen(existingDir) );
	CPPUNIT_ASSERT( path.InitCheck() == B_OK );
	CPPUNIT_ASSERT( string(existingDir) == path.Path() );
	link.Unset();
	path.Unset();
	dir.Unset();
	// relative link, uninitialized dir
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(relDirLink) == B_OK );
	CPPUNIT_ASSERT( dir.InitCheck() == B_NO_INIT);
	CPPUNIT_ASSERT( equals(link.MakeLinkedPath(&dir, &path), B_NO_INIT,
						   B_BAD_VALUE) );
	link.Unset();
	// relative link, badly initialized dir
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(relDirLink) == B_OK );
	CPPUNIT_ASSERT( dir.SetTo(nonExisting) == B_ENTRY_NOT_FOUND);
	CPPUNIT_ASSERT( equals(link.MakeLinkedPath(&dir, &path), B_NO_INIT,
						   B_BAD_VALUE) );
	link.Unset();
	path.Unset();
	dir.Unset();
	// bad args
	// R5: crashs, when passing a NULL path
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(dirLink) == B_OK );
	CPPUNIT_ASSERT( dir.SetTo("/boot") == B_OK);
//	CPPUNIT_ASSERT( link.MakeLinkedPath(&dir, NULL) == B_BAD_VALUE );
	CPPUNIT_ASSERT( link.MakeLinkedPath((const BDirectory*)NULL, &path)
					== B_BAD_VALUE );
//	CPPUNIT_ASSERT( link.MakeLinkedPath((const BDirectory*)NULL, NULL)
//					== B_BAD_VALUE );
	link.Unset();
	path.Unset();
	dir.Unset();
}

// IsAbsoluteTest
void
SymLinkTest::IsAbsoluteTest()
{
	const char *dirLink = dirLinkname;
	const char *dirSuperLink = dirSuperLinkname;
	const char *dirRelLink = dirRelLinkname;
	const char *fileLink = fileLinkname;
	const char *fileSuperLink = fileSuperLinkname;
	const char *fileRelLink = fileRelLinkname;
	const char *relDirLink = relDirLinkname;
	const char *relFileLink = relFileLinkname;
	const char *badLink = badLinkname;
	const char *cyclicLink1 = cyclicLinkname1;
	const char *cyclicLink2 = cyclicLinkname2;
	const char *existingDir = existingDirname;
	const char *existingSuperDir = existingSuperDirname;
	const char *existingRelDir = existingRelDirname;
	const char *existingFile = existingFilename;
	const char *existingSuperFile = existingSuperFilename;
	const char *existingRelFile = existingRelFilename;
	const char *nonExisting = nonExistingDirname;
	const char *nonExistingSuper = nonExistingSuperDirname;
	const char *nonExistingRel = nonExistingRelDirname;
	BSymLink link;
	// uninitialized
	nextSubTest();
	CPPUNIT_ASSERT( link.InitCheck() == B_NO_INIT );
	CPPUNIT_ASSERT( link.IsAbsolute() == false );
	link.Unset();
	// existing absolute dir link
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(dirLink) == B_OK );
	CPPUNIT_ASSERT( link.IsAbsolute() == true );
	link.Unset();
	// existing relative file link
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(relFileLink) == B_OK );
	CPPUNIT_ASSERT( link.IsAbsolute() == false );
	link.Unset();
	// non-existing link
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(nonExisting) == B_ENTRY_NOT_FOUND );
	CPPUNIT_ASSERT( link.IsAbsolute() == false );
	link.Unset();
	// dir
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(existingDir) == B_OK );
	CPPUNIT_ASSERT( link.IsAbsolute() == false );
	link.Unset();
	// file
	nextSubTest();
	CPPUNIT_ASSERT( link.SetTo(existingFile) == B_OK );
	CPPUNIT_ASSERT( link.IsAbsolute() == false );
	link.Unset();
}

// AssignmentTest
void
SymLinkTest::AssignmentTest()
{
	const char *dirLink = dirLinkname;
	const char *dirSuperLink = dirSuperLinkname;
	const char *dirRelLink = dirRelLinkname;
	const char *fileLink = fileLinkname;
	const char *fileSuperLink = fileSuperLinkname;
	const char *fileRelLink = fileRelLinkname;
	const char *relDirLink = relDirLinkname;
	const char *relFileLink = relFileLinkname;
	const char *badLink = badLinkname;
	const char *cyclicLink1 = cyclicLinkname1;
	const char *cyclicLink2 = cyclicLinkname2;
	const char *existingDir = existingDirname;
	const char *existingSuperDir = existingSuperDirname;
	const char *existingRelDir = existingRelDirname;
	const char *existingFile = existingFilename;
	const char *existingSuperFile = existingSuperFilename;
	const char *existingRelFile = existingRelFilename;
	const char *nonExisting = nonExistingDirname;
	const char *nonExistingSuper = nonExistingSuperDirname;
	const char *nonExistingRel = nonExistingRelDirname;
	// 1. copy constructor
	// uninitialized
	nextSubTest();
	{
		BSymLink link;
		CPPUNIT_ASSERT( link.InitCheck() == B_NO_INIT );
		BSymLink link2(link);
		// R5 returns B_BAD_VALUE instead of B_NO_INIT
		CPPUNIT_ASSERT( equals(link2.InitCheck(), B_BAD_VALUE, B_NO_INIT) );
	}
	// existing dir link
	nextSubTest();
	{
		BSymLink link(dirLink);
		CPPUNIT_ASSERT( link.InitCheck() == B_OK );
		BSymLink link2(link);
		CPPUNIT_ASSERT( link2.InitCheck() == B_OK );
	}
	// existing file link
	nextSubTest();
	{
		BSymLink link(fileLink);
		CPPUNIT_ASSERT( link.InitCheck() == B_OK );
		BSymLink link2(link);
		CPPUNIT_ASSERT( link2.InitCheck() == B_OK );
	}

	// 2. assignment operator
	// uninitialized
	nextSubTest();
	{
		BSymLink link;
		BSymLink link2;
		link2 = link;
		// R5 returns B_BAD_VALUE instead of B_NO_INIT
		CPPUNIT_ASSERT( equals(link2.InitCheck(), B_BAD_VALUE, B_NO_INIT) );
	}
	nextSubTest();
	{
		BSymLink link;
		BSymLink link2(dirLink);
		link2 = link;
		// R5 returns B_BAD_VALUE instead of B_NO_INIT
		CPPUNIT_ASSERT( equals(link2.InitCheck(), B_BAD_VALUE, B_NO_INIT) );
	}
	// existing dir link
	nextSubTest();
	{
		BSymLink link(dirLink);
		BSymLink link2;
		link2 = link;
		CPPUNIT_ASSERT( link2.InitCheck() == B_OK );
	}
	// existing file link
	nextSubTest();
	{
		BSymLink link(fileLink);
		BSymLink link2;
		link2 = link;
		CPPUNIT_ASSERT( link2.InitCheck() == B_OK );
	}
}




// some filenames to be used in tests
const char *SymLinkTest::existingFilename		= "/tmp/existing-file";
const char *SymLinkTest::existingSuperFilename	= "/tmp";
const char *SymLinkTest::existingRelFilename	= "existing-file";
const char *SymLinkTest::existingDirname		= "/tmp/existing-dir";
const char *SymLinkTest::existingSuperDirname	= "/tmp";
const char *SymLinkTest::existingRelDirname		= "existing-dir";
const char *SymLinkTest::existingSubDirname
	= "/tmp/existing-dir/existing-subdir";
const char *SymLinkTest::existingRelSubDirname	= "existing-subdir";
const char *SymLinkTest::nonExistingDirname		= "/tmp/non-existing-dir";
const char *SymLinkTest::nonExistingSuperDirname	= "/tmp";
const char *SymLinkTest::nonExistingRelDirname	= "non-existing-dir";
const char *SymLinkTest::testDirname1			= "/tmp/test-dir1";
const char *SymLinkTest::tooLongEntryname		=
	"/tmp/This is an awfully long name for an entry. It is that kind of entry "
	"that just can't exist due to its long name. In fact its path name is not "
	"too long -- a path name can contain 1024 characters -- but the name of "
	"the entry itself is restricted to 256 characters, which this entry's "
	"name does exceed.";
const char *SymLinkTest::tooLongSuperEntryname	= "/tmp";
const char *SymLinkTest::tooLongRelEntryname	=
	"This is an awfully long name for an entry. It is that kind of entry "
	"that just can't exist due to its long name. In fact its path name is not "
	"too long -- a path name can contain 1024 characters -- but the name of "
	"the entry itself is restricted to 256 characters, which this entry's "
	"name does exceed.";
const char *SymLinkTest::fileDirname			= "/tmp/test-file1/some-dir";
const char *SymLinkTest::fileSuperDirname		= "/tmp";
const char *SymLinkTest::fileRelDirname			= "test-file1/some-dir";
const char *SymLinkTest::dirLinkname			= "/tmp/link-to-dir1";
const char *SymLinkTest::dirSuperLinkname		= "/tmp";
const char *SymLinkTest::dirRelLinkname			= "link-to-dir1";
const char *SymLinkTest::fileLinkname			= "/tmp/link-to-file1";
const char *SymLinkTest::fileSuperLinkname		= "/tmp";
const char *SymLinkTest::fileRelLinkname		= "link-to-file1";
const char *SymLinkTest::relDirLinkname			= "/tmp/rel-link-to-dir1";
const char *SymLinkTest::relFileLinkname		= "/tmp/rel-link-to-file1";
const char *SymLinkTest::badLinkname			= "/tmp/link-to-void";
const char *SymLinkTest::cyclicLinkname1		= "/tmp/cyclic-link1";
const char *SymLinkTest::cyclicLinkname2		= "/tmp/cyclic-link2";

const char *SymLinkTest::allFilenames[]			=  {
	SymLinkTest::existingFilename,
	SymLinkTest::existingDirname,
	SymLinkTest::nonExistingDirname,
	SymLinkTest::testDirname1,
	SymLinkTest::dirLinkname,
	SymLinkTest::fileLinkname,
	SymLinkTest::relDirLinkname,
	SymLinkTest::relFileLinkname,
	SymLinkTest::badLinkname,
	SymLinkTest::cyclicLinkname1,
	SymLinkTest::cyclicLinkname2,
};
const int32 SymLinkTest::allFilenameCount
	= sizeof(allFilenames) / sizeof(const char*);
