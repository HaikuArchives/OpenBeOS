// FileTest.cpp

#include <Entry.h>
#include <File.h>

#include "FileTest.h"

// FileTestCaller
//
// a TestCaller that cleans up after the test is finished

template <typename Fixture,
		  typename ExpectedException = class CppUnit::NoExceptionExpected>
//		  typename ExpectedException>
//struct FileTest::FileTestCaller
struct FileTestCaller
	: public CppUnit::TestCaller<Fixture, ExpectedException> {
	FileTestCaller(std::string name, TestMethod test)
		: CppUnit::TestCaller<Fixture, ExpectedException>(name, test) {}
	FileTestCaller(std::string name, TestMethod test, Fixture& fixture)
		: CppUnit::TestCaller<Fixture, ExpectedException>(name, test,
														  fixture) {}
	FileTestCaller(std::string name, TestMethod test, Fixture* fixture)
		: CppUnit::TestCaller<Fixture, ExpectedException>(name, test,
														  fixture) {}

	void setUp()
	{
		FileTest::execCommand("touch ", FileTest::existingFilename);
	}

	void tearDown ()
	{
		// cleanup
		for (int32 i = 0;
			 i < sizeof(FileTest::allFilenames) / sizeof(const char*);
			 i++) {
			FileTest::execCommand("rm -f ", FileTest::allFilenames[i]);
		}
		printf("\n");
	}
};



// FileTest

// constructor
FileTest::FileTest()
	: CppUnit::TestCase(),
	  subTestNumber(0)
{
}

// Suite
FileTest::Test*
FileTest::Suite()
{
	CppUnit::TestSuite *suite = new CppUnit::TestSuite();
	
	suite->addTest( new FileTestCaller<FileTest>("BFile::Init Test 1", &FileTest::InitTest1) );
	suite->addTest( new FileTestCaller<FileTest>("BFile::Init Test 2", &FileTest::InitTest2) );
	suite->addTest( new FileTestCaller<FileTest>("BFile::IsRead-/IsWriteable Test", &FileTest::RWAbleTest) );
	suite->addTest( new FileTestCaller<FileTest>("BFile::Read/Write Test", &FileTest::RWTest) );
	suite->addTest( new FileTestCaller<FileTest>("BFile::Position Test", &FileTest::PositionTest) );
	suite->addTest( new FileTestCaller<FileTest>("BFile::Size Test", &FileTest::SizeTest) );
	suite->addTest( new FileTestCaller<FileTest>("BFile::Assignment Test", &FileTest::AssignmentTest) );
	
	return suite;
}		

// setUp
void FileTest::setUp() {}

// tearDown
void FileTest::tearDown()	{}

// InitTest1
void
FileTest::InitTest1()
{
	// 1. default constructor
	{
		BFile file;
		CPPUNIT_ASSERT( file.InitCheck() == B_NO_INIT );
	}

	// helper class for the testing the different constructors versions
	struct Tester {
		void testAll() const
		{
			for (int32 i = 0; i < initTestCasesCount; i++) {
				printf("[%ld]", i);
				fflush(stdin);
				test(initTestCases[i]);
			}
			printf("\n");
		}

		virtual void test(const InitTestCase& tc) const = 0;

		static void testInit(const InitTestCase& tc, BFile& file)
		{
			CPPUNIT_ASSERT( file.InitCheck() == tc.initCheck );
			if (tc.removeAfterTest)
				execCommand("rm ", tc.filename);
		}
	};

	// 2. BFile(const char *, uint32)
	struct Tester1 : public Tester {
		virtual void test(const InitTestCase& tc) const
		{
			BFile file(tc.filename,
					   tc.rwmode
					   | (tc.createFile * B_CREATE_FILE)
					   | (tc.failIfExists * B_FAIL_IF_EXISTS)
					   | (tc.eraseFile * B_ERASE_FILE));
			testInit(tc, file);
		}
	};

	// 3. BFile(entry_ref *, uint32)
	struct Tester2 : public Tester {
		virtual void test(const InitTestCase& tc) const
		{
			entry_ref ref;
			BEntry entry(tc.filename);
			entry_ref *refToPass = &ref;
			if (tc.filename)
				CPPUNIT_ASSERT( entry.GetRef(&ref) == B_OK );
			else
				refToPass = NULL;
			BFile file(refToPass,
					   tc.rwmode
					   | (tc.createFile * B_CREATE_FILE)
					   | (tc.failIfExists * B_FAIL_IF_EXISTS)
					   | (tc.eraseFile * B_ERASE_FILE));
			testInit(tc, file);
		}
	};

	// 4. BFile(BEntry *, uint32)
	struct Tester3 : public Tester {
		virtual void test(const InitTestCase& tc) const
		{
			entry_ref ref;
			BEntry entry(tc.filename);
			BEntry *entryToPass = &entry;
			if (tc.filename)
				CPPUNIT_ASSERT( entry.InitCheck() == B_OK );
			else
				entryToPass = NULL;
			BFile file(entryToPass,
					   tc.rwmode
					   | (tc.createFile * B_CREATE_FILE)
					   | (tc.failIfExists * B_FAIL_IF_EXISTS)
					   | (tc.eraseFile * B_ERASE_FILE));
			testInit(tc, file);
		}
	};

	// 5. BFile(BEntry *, uint32)
	// Can't be tested until BDirectory is implemented.

	Tester1().testAll();
	Tester2().testAll();
	Tester3().testAll();
}

// InitTest2
void
FileTest::InitTest2()
{
	// helper class for the testing the different SetTo() versions
	struct Tester {
		void testAll() const
		{
			for (int32 i = 0; i < initTestCasesCount; i++) {
				printf("[%ld]", i);
				fflush(stdin);
				test(initTestCases[i]);
			}
			printf("\n");
		}

		virtual void test(const InitTestCase& tc) const = 0;

		static void testInit(const InitTestCase& tc, BFile& file)
		{
			CPPUNIT_ASSERT( file.InitCheck() == tc.initCheck );
			file.Unset();
			CPPUNIT_ASSERT( file.InitCheck() == B_NO_INIT );
			if (tc.removeAfterTest)
				execCommand("rm ", tc.filename);
		}
	};

	// 2. BFile(const char *, uint32)
	struct Tester1 : public Tester {
		virtual void test(const InitTestCase& tc) const
		{
			BFile file;
			status_t result = file.SetTo(tc.filename,
				tc.rwmode
				| (tc.createFile * B_CREATE_FILE)
				| (tc.failIfExists * B_FAIL_IF_EXISTS)
				| (tc.eraseFile * B_ERASE_FILE));
			CPPUNIT_ASSERT( result == tc.initCheck );
			testInit(tc, file);
		}
	};

	// 3. BFile(entry_ref *, uint32)
	struct Tester2 : public Tester {
		virtual void test(const InitTestCase& tc) const
		{
			entry_ref ref;
			BEntry entry(tc.filename);
			entry_ref *refToPass = &ref;
			if (tc.filename)
				CPPUNIT_ASSERT( entry.GetRef(&ref) == B_OK );
			else
				refToPass = NULL;
			BFile file;
			status_t result = file.SetTo(refToPass,
				tc.rwmode
				| (tc.createFile * B_CREATE_FILE)
				| (tc.failIfExists * B_FAIL_IF_EXISTS)
				| (tc.eraseFile * B_ERASE_FILE));
			CPPUNIT_ASSERT( result == tc.initCheck );
			testInit(tc, file);
		}
	};

	// 4. BFile(BEntry *, uint32)
	struct Tester3 : public Tester {
		virtual void test(const InitTestCase& tc) const
		{
			entry_ref ref;
			BEntry entry(tc.filename);
			BEntry *entryToPass = &entry;
			if (tc.filename)
				CPPUNIT_ASSERT( entry.InitCheck() == B_OK );
			else
				entryToPass = NULL;
			BFile file;
			status_t result = file.SetTo(entryToPass,
				tc.rwmode
				| (tc.createFile * B_CREATE_FILE)
				| (tc.failIfExists * B_FAIL_IF_EXISTS)
				| (tc.eraseFile * B_ERASE_FILE));
			CPPUNIT_ASSERT( result == tc.initCheck );
			testInit(tc, file);
		}
	};

	// 5. BFile(BEntry *, uint32)
	// Can't be tested until BDirectory is implemented.

	Tester1().testAll();
	Tester2().testAll();
	Tester3().testAll();
}

// RWAbleTest
void
FileTest::RWAbleTest()
{
	nextSubTest();
	{
		BFile file;
		CPPUNIT_ASSERT( file.IsReadable() == false );
		CPPUNIT_ASSERT( file.IsWritable() == false );
	}
	nextSubTest();
	{
		BFile file(existingFilename, B_READ_ONLY);
		CPPUNIT_ASSERT( file.IsReadable() == true );
		CPPUNIT_ASSERT( file.IsWritable() == false );
	}
	nextSubTest();
	{
		BFile file(existingFilename, B_WRITE_ONLY);
		CPPUNIT_ASSERT( file.IsReadable() == false );
		CPPUNIT_ASSERT( file.IsWritable() == true );
	}
	nextSubTest();
	{
		BFile file(existingFilename, B_READ_WRITE);
		CPPUNIT_ASSERT( file.IsReadable() == true );
		CPPUNIT_ASSERT( file.IsWritable() == true );
	}
	nextSubTest();
	{
		BFile file(nonExistingFilename, B_READ_WRITE);
		CPPUNIT_ASSERT( file.IsReadable() == false );
		CPPUNIT_ASSERT( file.IsWritable() == false );
	}
}

// RWTest
void
FileTest::RWTest()
{
	// read/write an uninitialized BFile
	nextSubTest();
	BFile file;
	char buffer[10];
	CPPUNIT_ASSERT( file.Read(buffer, sizeof(buffer)) < 0 );
	CPPUNIT_ASSERT( file.ReadAt(0, buffer, sizeof(buffer)) < 0 );
	CPPUNIT_ASSERT( file.Write(buffer, sizeof(buffer)) < 0 );
	CPPUNIT_ASSERT( file.WriteAt(0, buffer, sizeof(buffer)) < 0 );
	file.Unset();
	// read/write an file opened for writing/reading only
	nextSubTest();
	file.SetTo(existingFilename, B_WRITE_ONLY);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.Read(buffer, sizeof(buffer)) < 0 );
	CPPUNIT_ASSERT( file.ReadAt(0, buffer, sizeof(buffer)) < 0 );
	file.SetTo(existingFilename, B_READ_ONLY);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.Write(buffer, sizeof(buffer)) < 0 );
	CPPUNIT_ASSERT( file.WriteAt(0, buffer, sizeof(buffer)) < 0 );
	file.Unset();
	// read from an empty file
	nextSubTest();
	file.SetTo(existingFilename, B_READ_ONLY);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.Read(buffer, sizeof(buffer)) == 0 );
	CPPUNIT_ASSERT( file.ReadAt(0, buffer, sizeof(buffer)) == 0 );
	file.Unset();
	// read from past an empty file
	nextSubTest();
	file.SetTo(existingFilename, B_READ_ONLY);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.Seek(10, SEEK_SET) == 10 );
	CPPUNIT_ASSERT( file.Read(buffer, sizeof(buffer)) == 0 );
	CPPUNIT_ASSERT( file.ReadAt(10, buffer, sizeof(buffer)) == 0 );
	file.Unset();
	// create a new empty file and write some data into it, then
	// read the file and check the data
	nextSubTest();
	file.SetTo(testFilename1, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	char writeBuffer[256];
	for (int32 i = 0; i < 256; i++)
		writeBuffer[i] = (char)i;
	CPPUNIT_ASSERT( file.Write(writeBuffer, 128) == 128 );
	CPPUNIT_ASSERT( file.Position() == 128 );
	CPPUNIT_ASSERT( file.Write(writeBuffer + 128, 128) == 128 );
	CPPUNIT_ASSERT( file.Position() == 256 );
	file.Unset();
	file.SetTo(testFilename1, B_READ_ONLY);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	char readBuffer[256];
	CPPUNIT_ASSERT( file.Read(readBuffer, 42) == 42 );
	CPPUNIT_ASSERT( file.Position() == 42 );
	CPPUNIT_ASSERT( file.Read(readBuffer + 42, 400) == 214 );
	CPPUNIT_ASSERT( file.Position() == 256 );
	for (int32 i = 0; i < 256; i++)
		CPPUNIT_ASSERT( readBuffer[i] == (char)i );
	file.Unset();
	execCommand("rm -f ", testFilename1);
	// same procedure, just using ReadAt()/WriteAt()
	nextSubTest();
	file.SetTo(testFilename1, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.WriteAt(80, writeBuffer + 80, 50) == 50 );
	CPPUNIT_ASSERT( file.Position() == 0 );
	CPPUNIT_ASSERT( file.WriteAt(0, writeBuffer, 80) == 80 );
	CPPUNIT_ASSERT( file.Position() == 0 );
	CPPUNIT_ASSERT( file.WriteAt(130, writeBuffer + 130, 126) == 126 );
	CPPUNIT_ASSERT( file.Position() == 0 );
	file.Unset();
	file.SetTo(testFilename1, B_READ_ONLY);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	for (int32 i = 0; i < 256; i++)
		readBuffer[i] = 0;
	CPPUNIT_ASSERT( file.ReadAt(42, readBuffer + 42, 84) == 84 );
	CPPUNIT_ASSERT( file.Position() == 0 );
	CPPUNIT_ASSERT( file.ReadAt(0, readBuffer, 42) == 42 );
	CPPUNIT_ASSERT( file.Position() == 0 );
	CPPUNIT_ASSERT( file.ReadAt(126, readBuffer + 126, 130) == 130 );
	CPPUNIT_ASSERT( file.Position() == 0 );
	for (int32 i = 0; i < 256; i++)
		CPPUNIT_ASSERT( readBuffer[i] == (char)i );
	file.Unset();
	execCommand("rm -f ", testFilename1);
	// write past the end of a file
	nextSubTest();
	file.SetTo(testFilename1, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.Seek(128, SEEK_SET) == 128 );
	CPPUNIT_ASSERT( file.Write(writeBuffer, 128) == 128 );
	CPPUNIT_ASSERT( file.Position() == 256 );
	file.Unset();
	// open the file with B_OPEN_AT_END flag, Write() some data to it, close
	// and re-open it to check the file
	nextSubTest();
	file.SetTo(testFilename1, B_WRITE_ONLY | B_OPEN_AT_END);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	for (int32 i = 0; i < 256; i++)
		writeBuffer[i] = (char)7;
	CPPUNIT_ASSERT( file.Write(writeBuffer, 50) == 50 );
	file.Seek(0, SEEK_SET);	// might fail -- don't check the return value
	CPPUNIT_ASSERT( file.Write(writeBuffer, 40) == 40 );
	file.Unset();
	file.SetTo(testFilename1, B_READ_ONLY);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.ReadAt(256, readBuffer, 90) == 90 );
	for (int32 i = 0; i < 90; i++)
		CPPUNIT_ASSERT( readBuffer[i] == 7 );
	file.Unset();
	// open the file with B_OPEN_AT_END flag, WriteAt() some data to it, close
	// and re-open it to check the file
	nextSubTest();
	file.SetTo(testFilename1, B_WRITE_ONLY | B_OPEN_AT_END);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	for (int32 i = 0; i < 256; i++)
		writeBuffer[i] = (char)42;
	CPPUNIT_ASSERT( file.WriteAt(0, writeBuffer, 30) == 30 );
	file.Unset();
	file.SetTo(testFilename1, B_READ_ONLY);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.ReadAt(346, readBuffer, 30) == 30 );
	for (int32 i = 0; i < 30; i++)
		CPPUNIT_ASSERT( readBuffer[i] == 42 );
	file.Unset();
	// open the file with B_OPEN_AT_END flag, ReadAt() some data
	nextSubTest();
	file.SetTo(testFilename1, B_READ_ONLY | B_OPEN_AT_END);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	for (int32 i = 0; i < 256; i++)
		readBuffer[i] = 0;
	CPPUNIT_ASSERT( file.ReadAt(256, readBuffer, 90) == 90 );
	for (int32 i = 0; i < 90; i++)
		CPPUNIT_ASSERT( readBuffer[i] == 7 );
	CPPUNIT_ASSERT( file.ReadAt(346, readBuffer, 30) == 30 );
	for (int32 i = 0; i < 30; i++)
		CPPUNIT_ASSERT( readBuffer[i] == 42 );
	file.Unset();
	// same procedure, just using Seek() and Read()
	nextSubTest();
	file.SetTo(testFilename1, B_READ_ONLY | B_OPEN_AT_END);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	for (int32 i = 0; i < 256; i++)
		readBuffer[i] = 0;
	file.Seek(256, SEEK_SET);	// might fail -- don't check the return value
	CPPUNIT_ASSERT( file.Read(readBuffer, 90) == 90 );
	for (int32 i = 0; i < 90; i++)
		CPPUNIT_ASSERT( readBuffer[i] == 7 );
	CPPUNIT_ASSERT( file.ReadAt(346, readBuffer, 30) == 30 );
	for (int32 i = 0; i < 30; i++)
		CPPUNIT_ASSERT( readBuffer[i] == 42 );
	file.Unset();

	execCommand("rm -f ", testFilename1);
}

// PositionTest
void
FileTest::PositionTest()
{
	// unitialized file
	nextSubTest();
	BFile file;
	CPPUNIT_ASSERT( file.Position() == B_FILE_ERROR );
	CPPUNIT_ASSERT( file.Seek(10, SEEK_SET) == B_FILE_ERROR );
	CPPUNIT_ASSERT( file.Seek(10, SEEK_END) == B_FILE_ERROR );
	CPPUNIT_ASSERT( file.Seek(10, SEEK_CUR) == B_FILE_ERROR );
	// open new file, write some bytes to it and seek a bit around
	nextSubTest();
	file.SetTo(testFilename1, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.Position() == 0 );
	char writeBuffer[256];
	CPPUNIT_ASSERT( file.Write(writeBuffer, 256) == 256 );
	CPPUNIT_ASSERT( file.Position() == 256 );
	CPPUNIT_ASSERT( file.Seek(10, SEEK_SET) == 10 );
	CPPUNIT_ASSERT( file.Position() == 10 );
	CPPUNIT_ASSERT( file.Seek(-20, SEEK_END) == 236 );
	CPPUNIT_ASSERT( file.Position() == 236 );
	CPPUNIT_ASSERT( file.Seek(-70, SEEK_CUR) == 166 );
	CPPUNIT_ASSERT( file.Position() == 166 );
	file.Unset();
	// re-open the file at the end and seek a bit around once more
	// The BeBook is a bit unspecific about the B_OPEN_AT_END flag:
	// It has probably the same meaning as the POSIX flag O_APPEND, which
	// means, that all write()s append their data at the end. The behavior
	// of Seek() and Position() is a bit unclear for this case.
/*
	nextSubTest();
	file.SetTo(testFilename1, B_READ_ONLY | B_OPEN_AT_END);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.Position() == 256 );
	CPPUNIT_ASSERT( file.Seek(10, SEEK_SET) == 10 );
	CPPUNIT_ASSERT( file.Position() == 10 );			// fails with R5
	CPPUNIT_ASSERT( file.Seek(-20, SEEK_END) == 236 );
	CPPUNIT_ASSERT( file.Position() == 236 );			// fails with R5
	CPPUNIT_ASSERT( file.Seek(-70, SEEK_CUR) == 166 );	// fails with R5
	CPPUNIT_ASSERT( file.Position() == 166 );			// fails with R5
*/
	file.Unset();
	execCommand("rm -f ", testFilename1);
}

// SizeTest
void
FileTest::SizeTest()
{
	// unitialized file
	nextSubTest();
	BFile file;
	off_t size;
	CPPUNIT_ASSERT( file.GetSize(&size) != B_OK );
	CPPUNIT_ASSERT( file.SetSize(100) != B_OK );
	// read only file
	nextSubTest();
	file.SetTo(testFilename1, B_READ_ONLY | B_CREATE_FILE);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.GetSize(&size) == B_OK );
	CPPUNIT_ASSERT( size == 0 );
	CPPUNIT_ASSERT( file.SetSize(100) == B_OK );
	CPPUNIT_ASSERT( file.GetSize(&size) == B_OK );
	CPPUNIT_ASSERT( size == 100 );
	file.Unset();
	// shorten existing file
	nextSubTest();
	file.SetTo(testFilename1, B_WRITE_ONLY);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.GetSize(&size) == B_OK );
	CPPUNIT_ASSERT( size == 100 );
	CPPUNIT_ASSERT( file.SetSize(73) == B_OK );
	CPPUNIT_ASSERT( file.GetSize(&size) == B_OK );
	CPPUNIT_ASSERT( size == 73 );
	file.Unset();
	// enlarge existing file
	nextSubTest();
	file.SetTo(testFilename1, B_READ_WRITE);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.GetSize(&size) == B_OK );
	CPPUNIT_ASSERT( size == 73 );
	CPPUNIT_ASSERT( file.SetSize(147) == B_OK );
	CPPUNIT_ASSERT( file.GetSize(&size) == B_OK );
	CPPUNIT_ASSERT( size == 147 );
	file.Unset();
	// erase existing file (read only)
	nextSubTest();
	file.SetTo(testFilename1, B_READ_ONLY | B_ERASE_FILE);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.GetSize(&size) == B_OK );
	CPPUNIT_ASSERT( size == 0 );
	CPPUNIT_ASSERT( file.SetSize(132) == B_OK );
	CPPUNIT_ASSERT( file.GetSize(&size) == B_OK );
	CPPUNIT_ASSERT( size == 132 );
	file.Unset();
	// erase existing file (write only)
	nextSubTest();
	file.SetTo(testFilename1, B_WRITE_ONLY | B_ERASE_FILE);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.GetSize(&size) == B_OK );
	CPPUNIT_ASSERT( size == 0 );
	CPPUNIT_ASSERT( file.SetSize(93) == B_OK );
	CPPUNIT_ASSERT( file.GetSize(&size) == B_OK );
	CPPUNIT_ASSERT( size == 93 );
	file.Unset();
	// erase existing file using SetSize()
	nextSubTest();
	file.SetTo(testFilename1, B_READ_WRITE);
	CPPUNIT_ASSERT( file.InitCheck() == B_OK );
	CPPUNIT_ASSERT( file.GetSize(&size) == B_OK );
	CPPUNIT_ASSERT( size == 93 );
	CPPUNIT_ASSERT( file.SetSize(0) == B_OK );
	CPPUNIT_ASSERT( file.GetSize(&size) == B_OK );
	CPPUNIT_ASSERT( size == 0 );
	file.Unset();
	execCommand("rm -f ", testFilename1);
}

// AssignmentTest
void
FileTest::AssignmentTest()
{
	// copy constructor
	// uninitialized
	nextSubTest();
	{
		BFile file;
		CPPUNIT_ASSERT( file.InitCheck() == B_NO_INIT );
		BFile file2(file);
		// R5 returns B_BAD_VALUE instead of B_NO_INIT
//			CPPUNIT_ASSERT( file2.InitCheck() == B_NO_INIT );
		CPPUNIT_ASSERT( file2.InitCheck() != B_OK );
	}
	// existing file, different open modes
	nextSubTest();
	{
		BFile file(existingFilename, B_READ_ONLY);
		CPPUNIT_ASSERT( file.InitCheck() == B_OK );
		BFile file2(file);
		CPPUNIT_ASSERT( file2.InitCheck() == B_OK );
		CPPUNIT_ASSERT( file2.IsReadable() == true );
		CPPUNIT_ASSERT( file2.IsWritable() == false );
	}
	nextSubTest();
	{
		BFile file(existingFilename, B_WRITE_ONLY);
		CPPUNIT_ASSERT( file.InitCheck() == B_OK );
		BFile file2(file);
		CPPUNIT_ASSERT( file2.InitCheck() == B_OK );
		CPPUNIT_ASSERT( file2.IsReadable() == false );
		CPPUNIT_ASSERT( file2.IsWritable() == true );
	}
	nextSubTest();
	{
		BFile file(existingFilename, B_READ_WRITE);
		CPPUNIT_ASSERT( file.InitCheck() == B_OK );
		BFile file2(file);
		CPPUNIT_ASSERT( file2.InitCheck() == B_OK );
		CPPUNIT_ASSERT( file2.IsReadable() == true );
		CPPUNIT_ASSERT( file2.IsWritable() == true );
	}
	// assignment operator
	// uninitialized
	nextSubTest();
	{
		BFile file;
		BFile file2;
		file2 = file;
		// R5 returns B_BAD_VALUE instead of B_NO_INIT
//			CPPUNIT_ASSERT( file2.InitCheck() == B_NO_INIT );
		CPPUNIT_ASSERT( file2.InitCheck() != B_OK );
	}
	nextSubTest();
	{
		BFile file;
		BFile file2(existingFilename, B_READ_ONLY);
		CPPUNIT_ASSERT( file2.InitCheck() == B_OK );
		file2 = file;
		// R5 returns B_BAD_VALUE instead of B_NO_INIT
//			CPPUNIT_ASSERT( file2.InitCheck() == B_NO_INIT );
		CPPUNIT_ASSERT( file2.InitCheck() != B_OK );
	}
	// existing file, different open modes
	nextSubTest();
	{
		BFile file(existingFilename, B_READ_ONLY);
		CPPUNIT_ASSERT( file.InitCheck() == B_OK );
		BFile file2;
		file2 = file;
		CPPUNIT_ASSERT( file2.InitCheck() == B_OK );
		CPPUNIT_ASSERT( file2.IsReadable() == true );
		CPPUNIT_ASSERT( file2.IsWritable() == false );
	}
	nextSubTest();
	{
		BFile file(existingFilename, B_WRITE_ONLY);
		CPPUNIT_ASSERT( file.InitCheck() == B_OK );
		BFile file2;
		file2 = file;
		CPPUNIT_ASSERT( file2.InitCheck() == B_OK );
		CPPUNIT_ASSERT( file2.IsReadable() == false );
		CPPUNIT_ASSERT( file2.IsWritable() == true );
	}
	nextSubTest();
	{
		BFile file(existingFilename, B_READ_WRITE);
		CPPUNIT_ASSERT( file.InitCheck() == B_OK );
		BFile file2;
		file2 = file;
		CPPUNIT_ASSERT( file2.InitCheck() == B_OK );
		CPPUNIT_ASSERT( file2.IsReadable() == true );
		CPPUNIT_ASSERT( file2.IsWritable() == true );
	}
}

// nextSubTest
void
FileTest::nextSubTest()
{
	printf("[%ld]", subTestNumber++);
}

// Calls system() with the concatenated string of command and parameter.
// Probably one of the exec*() functions serves the same purpose, but
// I don't have the reference at hand.
// execCommand
void
FileTest::execCommand(const char *command, const char *parameter)
{
	if (command && parameter) {
		char *cmdLine = new char[strlen(command) + strlen(parameter) + 1];
		strcpy(cmdLine, command);
		strcat(cmdLine, parameter);
		system(cmdLine);
		delete[] cmdLine;
	}
}


// some filenames to be used in tests
const char *FileTest::existingFilename		= "/tmp/existing-file";
const char *FileTest::nonExistingFilename	= "/tmp/non-existing-file";
const char *FileTest::testFilename1			= "/tmp/test-file1";
const char *FileTest::allFilenames[]		=  {
	FileTest::existingFilename,
	FileTest::nonExistingFilename,
	FileTest::testFilename1,
};

// test cases for the init tests
const FileTest::InitTestCase FileTest::initTestCases[] = {
	{ existingFilename	 , B_READ_ONLY , 0, 0, 0, false, B_OK				},
	{ existingFilename	 , B_WRITE_ONLY, 0, 0, 0, false, B_OK				},
	{ existingFilename	 , B_READ_WRITE, 0, 0, 0, false, B_OK				},
	{ existingFilename	 , B_READ_ONLY , 1, 0, 0, false, B_OK				},
	{ existingFilename	 , B_WRITE_ONLY, 1, 0, 0, false, B_OK				},
	{ existingFilename	 , B_READ_WRITE, 1, 0, 0, false, B_OK				},
	{ existingFilename	 , B_READ_ONLY , 0, 1, 0, false, B_OK				},
	{ existingFilename	 , B_WRITE_ONLY, 0, 1, 0, false, B_OK				},
	{ existingFilename	 , B_READ_WRITE, 0, 1, 0, false, B_OK				},
	{ existingFilename	 , B_READ_ONLY , 0, 0, 1, false, B_OK				},
	{ existingFilename	 , B_WRITE_ONLY, 0, 0, 1, false, B_OK				},
	{ existingFilename	 , B_READ_WRITE, 0, 0, 1, false, B_OK				},
	{ existingFilename	 , B_READ_ONLY , 1, 1, 0, false, B_FILE_EXISTS		},
	{ existingFilename	 , B_WRITE_ONLY, 1, 1, 0, false, B_FILE_EXISTS		},
	{ existingFilename	 , B_READ_WRITE, 1, 1, 0, false, B_FILE_EXISTS		},
	{ existingFilename	 , B_READ_ONLY , 1, 0, 1, false, B_OK				},
	{ existingFilename	 , B_WRITE_ONLY, 1, 0, 1, false, B_OK				},
	{ existingFilename	 , B_READ_WRITE, 1, 0, 1, false, B_OK				},
	{ existingFilename	 , B_READ_ONLY , 1, 1, 1, false, B_FILE_EXISTS		},
	{ existingFilename	 , B_WRITE_ONLY, 1, 1, 1, false, B_FILE_EXISTS		},
	{ existingFilename	 , B_READ_WRITE, 1, 1, 1, false, B_FILE_EXISTS		},
	{ nonExistingFilename, B_READ_ONLY , 0, 0, 0, false, B_ENTRY_NOT_FOUND	},
	{ nonExistingFilename, B_WRITE_ONLY, 0, 0, 0, false, B_ENTRY_NOT_FOUND	},
	{ nonExistingFilename, B_READ_WRITE, 0, 0, 0, false, B_ENTRY_NOT_FOUND	},
	{ nonExistingFilename, B_READ_ONLY , 1, 0, 0, true , B_OK				},
	{ nonExistingFilename, B_WRITE_ONLY, 1, 0, 0, true , B_OK				},
	{ nonExistingFilename, B_READ_WRITE, 1, 0, 0, true , B_OK				},
	{ nonExistingFilename, B_READ_ONLY , 0, 1, 0, false, B_ENTRY_NOT_FOUND	},
	{ nonExistingFilename, B_WRITE_ONLY, 0, 1, 0, false, B_ENTRY_NOT_FOUND	},
	{ nonExistingFilename, B_READ_WRITE, 0, 1, 0, false, B_ENTRY_NOT_FOUND	},
	{ nonExistingFilename, B_READ_ONLY , 0, 0, 1, false, B_ENTRY_NOT_FOUND	},
	{ nonExistingFilename, B_WRITE_ONLY, 0, 0, 1, false, B_ENTRY_NOT_FOUND	},
	{ nonExistingFilename, B_READ_WRITE, 0, 0, 1, false, B_ENTRY_NOT_FOUND	},
	{ nonExistingFilename, B_READ_ONLY , 1, 1, 0, true , B_OK				},
	{ nonExistingFilename, B_WRITE_ONLY, 1, 1, 0, true , B_OK				},
	{ nonExistingFilename, B_READ_WRITE, 1, 1, 0, true , B_OK				},
	{ nonExistingFilename, B_READ_ONLY , 1, 0, 1, true , B_OK				},
	{ nonExistingFilename, B_WRITE_ONLY, 1, 0, 1, true , B_OK				},
	{ nonExistingFilename, B_READ_WRITE, 1, 0, 1, true , B_OK				},
	{ nonExistingFilename, B_READ_ONLY , 1, 1, 1, true , B_OK				},
	{ nonExistingFilename, B_WRITE_ONLY, 1, 1, 1, true , B_OK				},
	{ nonExistingFilename, B_READ_WRITE, 1, 1, 1, true , B_OK				},
	{ NULL,				   B_READ_ONLY , 1, 1, 1, false, B_BAD_VALUE		},
};
const int32 FileTest::initTestCasesCount
	= sizeof(FileTest::initTestCases) / sizeof(FileTest::InitTestCase);
