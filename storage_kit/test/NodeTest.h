#ifndef __sk_node_test_h__
#define __sk_node_test_h__

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include <Node.h>

#include <stdio.h>

#include <sys/stat.h>
#include <kernel_interface.h>

#include "TestUtils.h"

class NodeTest : public CppUnit::TestCase
{
public:
	static Test* Suite() {
		CppUnit::TestSuite *suite = new CppUnit::TestSuite();
		
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::Init Test", &NodeTest::InitTest) );
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::Attribute Directory Test", &NodeTest::AttrDirTest) );
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::Attribute Test", &NodeTest::AttrTest) );
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::Stat Test", &NodeTest::StatTest) );
//		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::Locking Test", &NodeTest::LockTest) );
		
		return suite;
	}		

	// This function called before *each* test added in Suite()
	void setUp() {}
	
	// This function called after *each* test added in Suite()
	void tearDown()	{}

	void InitTest() {
		BNode node;
		CPPUNIT_ASSERT( node.InitCheck() == B_NO_INIT );
		
		BNode node2("./");
		CPPUNIT_ASSERT( node2.InitCheck() == B_OK );
		
		BNode node3("./This.File.Ought.Not.Exist");
		CPPUNIT_ASSERT( node3.InitCheck() == B_ENTRY_NOT_FOUND );
	}

	void AttrDirTest() {
		BNode node;
		CPPUNIT_ASSERT( node.RewindAttrs() == B_BAD_ADDRESS );
		
		node.SetTo("./");
		
		char str[B_ATTR_NAME_LENGTH];
		status_t result;
		CPPUNIT_ASSERT( node.GetNextAttrName(str) == B_OK );

		// Get to the end of the list
		while( (result = node.GetNextAttrName(str)) == B_OK ) {
//			cout << str << endl;	// Uncomment to list off the attribute names
		}
			
		CPPUNIT_ASSERT( result == B_ENTRY_NOT_FOUND );
			// End of the list
			
		CPPUNIT_ASSERT( node.RewindAttrs() == B_OK );
			// Rewind

		// The following crashes on R5. Our implementation just returns B_BAD_VALUE
		//CPPUNIT_ASSERT( node.GetNextAttrName(NULL) != B_OK );
		
	}
	
	void AttrTest() {
		const int len = 1024;
	 	unsigned char data[len];
	 	const char attr[] = "StorageKit::TestAttribute";
	 	const char attrToBeRemoved[] = "StorageKit::ThisAttributeShouldHaveBeenRemoved";
	 	const char str[] = "This is a string all right";
	
		BNode node;
		CPPUNIT_ASSERT( node.ReadAttr(attr, B_STRING_TYPE, 0, data, len) == B_FILE_ERROR );
		
		node.SetTo("./");
		CPPUNIT_ASSERT( node.WriteAttr(attr, B_STRING_TYPE, 0, str, strlen(str)) == strlen(str) );
		CPPUNIT_ASSERT( node.ReadAttr(attr, B_STRING_TYPE, 0, data, len) > 0 );
		CPPUNIT_ASSERT( node.RemoveAttr(attr) == B_OK );
		CPPUNIT_ASSERT( node.ReadAttr(attr, B_STRING_TYPE, 0, data, len) == B_ENTRY_NOT_FOUND );
	}
	
	// Doesn't do very thorough testing yet (I'll leave that to Mike :-)
	void StatTest() {
		StorageKit::Stat s;
	
		BNode node;
		CPPUNIT_ASSERT( node.GetStat(&s) == B_NO_INIT );
		
		node.SetTo("./");
		CPPUNIT_ASSERT( node.GetStat(&s) == B_OK );
	
		CPPUNIT_ASSERT( node.SetOwner( s.st_uid ) == B_OK );
		CPPUNIT_ASSERT( node.SetGroup( s.st_gid ) == B_OK );
		
	
	}

	// Locking isn't really implemented yet...
	void LockTest() {
		BNode node;
		
		BNode node2("./");
		CPPUNIT_ASSERT( DecodeResult(node2.Lock()) == B_OK );
	
	}
	
};



#endif	// __sk_node_test_h__
