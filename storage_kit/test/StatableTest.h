#ifndef __sk_path_test_h__
#define __sk_path_test_h__

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include <Statable.h>
#include <Entry.h>

#define STATABLE BEntry
// Also test with BNode

class StatableTest : public CppUnit::TestCase
{
public:
	static Test* Suite() {
		CppUnit::TestSuite *suite = new CppUnit::TestSuite();
		
		suite->addTest( new CppUnit::TestCaller<PathTest>("BStatable::IsFile"(), &StatableTest::IsFile) );

		suite->addTest( new CppUnit::TestCaller<PathTest>("BStatable::IsLink"(), &StatableTest::IsLink) );

		suite->addTest( new CppUnit::TestCaller<PathTest>("BStatable::IsDirectory"(), &StatableTest::IsDirectory) );	  


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

		CPPUNIT_ASSERT( folder.IsLink() == false ); 
		CPPUNIT_ASSERT( link.IsLink() == true );
		CPPUNIT_ASSERT( file.IsLink == false );
		CPPUNIT_ASSERT( f404.IsLink == false );
	}

	
};



#endif	// __sk_path_test_h__
