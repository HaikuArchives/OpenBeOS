#ifndef __sk_node_test_h__
#define __sk_node_test_h__

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include <Node.h>

class NodeTest : public CppUnit::TestCase {
public:
	static CppUnit::Test* Suite();

	// This function called before *each* test added in Suite()
	void setUp();
	
	// This function called after *each* test added in Suite()
	void tearDown();

	// Tests
	void InitTest();
	void AttrDirTest();
	void AttrTest();
	void AttrRenameTest();
	void AttrInfoTest();
	void AttrBStringTest(); // Requires an OpenBeOS implementation of BString to test these
	void StatTest(); // Doesn't do very thorough testing (which I'm leaving to Mike and BStatable)
	void SyncTest();
	void DupTest();
	void EqualityTest();
	void AssignmentTest();
	void LockTest(); // Locking isn't implemented for the Posix versions

	// Helper functions
	void EqualityTest(BNode &n1, BNode &n2, BNode &y1a, BNode &y1b, BNode &y2);
};



#endif	// __sk_node_test_h__
