#ifndef __sk_entry_test_h__
#define __sk_entry_test_h__

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include "Test.StorageKit.h"

#include <Entry.h>

#include <Directory.h>
#include <Path.h>

#include <sys/utsname.h> // Not needed
#include <sys/statvfs.h> // Not needed

//#include <stdio.h>

//#include <sys/stat.h>	// For struct stat
//#include <fs_attr.h>	// For struct attr_info

//#include <kernel_interface.h>

#include "TestUtils.h"

class EntryTest : public StorageKit::TestCase
{
public:
	static CppUnit::Test* Suite();

	// Filenames
	static const char mainFile[] = "/boot/beos/apps/Clock";
	static const char link[] = "EntryTest.Link";
	static const char relLink[] = "EntryTest.RelLink";
	static const char fourLink[] = "EntryTest.4Link";
	static const char tripleLinkLeaf[] = "EntryTest.TripleLink";
	char tripleLink[B_PATH_NAME_LENGTH+1];
	static const char sparkleMotion[] = "/boot/I_am_beggining_to_doubt_your_dedication_to_Sparkle_Motion";
	static const char someFile[] = "/boot/SomeFileNameThatWeReallyReallyLikeALot";

	// Constructor
	EntryTest();

	// This function called before *each* test added in Suite()
	void setUp();

	// This function called after *each* test added in Suite()
	void tearDown();

	//-------------------------------------------------------------------------
	// Test functions
	//-------------------------------------------------------------------------
	void TraversalTest();
	void CopyConstructorTest();
	void EqualityTest();
	void AssignmentTest();	
	void ConversionTest();
	void MiscTest();
	void ExistenceTest();
	void RenameTest();
	void StatTest();

	//-------------------------------------------------------------------------
	// Helper functions
	//-------------------------------------------------------------------------	

	// Creates a symbolic link named link that points to target
	bool CreateLink(const char *link, const char *target);
	// Creates the given file
	bool CreateFile(const char *file);
	// Removes the given file if it exsists
	bool RemoveFile(const char *file);

	// n1 and n2 should both be uninitialized. y1a and y1b should be initialized
	// to the same entry, y2 should be initialized to a different entry
	void EqualityTest(BEntry &n1, BEntry &n2, BEntry &y1a, BEntry &y1b, BEntry &y2);

};


#endif	// __sk_entry_test_h__
