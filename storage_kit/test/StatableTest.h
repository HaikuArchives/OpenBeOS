#ifndef __sk_statable_test_h__
#define __sk_statable_test_h__

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include <Statable.h>
#include <Entry.h>

#define STATABLE BEntry
// Also test with STATBLE set to BNode

class StatableTest : public CppUnit::TestCase
{
public:
	static Test* Suite() {
		CppUnit::TestSuite *suite = new CppUnit::TestSuite();
		
		suite->addTest( new CppUnit::TestCaller<PathTest>("BStatable::IsFile()", &StatableTest::IsFile) );

		suite->addTest( new CppUnit::TestCaller<PathTest>("BStatable::IsLink()", &StatableTest::IsLink) );

		suite->addTest( new CppUnit::TestCaller<PathTest>("BStatable::IsDirectory()", &StatableTest::IsDirectory) );	  
		suite->addTest( new CppUnut::TestCaller<PathTest>("BStatable::GetStat()", &StatableTest::GetStat ) );
		return suite;
	}

	// This function called before *each* test added in Suite()
	void setUp() {}
	
	// This function called after *each* test added in Suite()
	void tearDown()	{}

	void IsFile() {
		STATABLE folder("/", false);
		STATABLE link("/system/", false);
		STATABLE file("/system/Tracker", false);
		STATABLE f404("/what_ya_bet/some_one_has a file called this", false);

		CPPUNIT_ASSERT( folder.IsFile() == false ); 
		CPPUNIT_ASSERT( link.IsFile() == false );
		CPPUNIT_ASSERT( file.IsFile() == true );
		CPPUNIT_ASSERT( f404.IsFile() == false );
	}

	void IsDirectory() {
		STATABLE folder("/", false);
		STATABLE link("/system/", false);
		STATABLE file("/system/Tracker", false);
		STATABLE f404("/what_ya_bet/some_one_has a file called this", false);

		CPPUNIT_ASSERT( folder.IsDirectory() == true ); 
		CPPUNIT_ASSERT( link.IsDirectory() == false );
		CPPUNIT_ASSERT( file.IsDirectory() == false );
		CPPUNIT_ASSERT( f404.IsDirectory() == false );
	}

	void IsLink() {
		STATABLE folder("/", false);
		STATABLE link("/system/", false);
		STATABLE file("/system/Tracker", false);
		STATABLE f404("/what_ya_bet/some_one_has a file called this", false);

		CPPUNIT_ASSERT( folder.IsSymLink() == false ); 
		CPPUNIT_ASSERT( link.IsSymLink() == true );
		CPPUNIT_ASSERT( file.IsSymLink() == false );
		CPPUNIT_ASSERT( f404.IsSymLink() == false );
	}

	void GetStat() {
  		STATABLE folder("/", false);
		STATABLE link("/system/", false);
		STATABLE file("/system/Tracker", false);
		STATABLE f404("/what_ya_bet/some_one_has a file called this", false);
		
		struct stat folderStat;
		struct stat linkStat;
		struct stat fileStat;
		struct stat f404Stat;
		
		CPPUNIT_ASSERT( folder.GetStat(&folderStat) == B_OK );
		CPPUNIT_ASSERT( link.GetStat(&linkStat) == B_OK );
		CPPUNIT_ASSERT( file.GetStat(&fileStat) == B_OK );
		CPPUNIT_ASSERT( f404.GetStat(&f404Stat) == B_BAD_VALUE );

	}
	
};

#endif	// __sk_statable_test_h__
