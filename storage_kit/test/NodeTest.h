#ifndef __sk_node_test_h__
#define __sk_node_test_h__

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include <Node.h>

#include <stdio.h>

#include "TestUtils.h"

class NodeTest : public CppUnit::TestCase
{
public:
	static Test* Suite() {
		CppUnit::TestSuite *suite = new CppUnit::TestSuite();
		
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::Init Test", &NodeTest::InitTest) );
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::Attribute Directory Test", &NodeTest::AttrDirTest) );
		
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
		
		BNode node2("./");
		
		char str[B_ATTR_NAME_LENGTH];
		status_t result;
		CPPUNIT_ASSERT( node2.GetNextAttrName(str) == B_OK );

		// Get to the end of the list
		while( (result = node2.GetNextAttrName(str)) == B_OK ) {
//			cout << str << endl;	// Uncomment to list off the attribute names
		}
			
		CPPUNIT_ASSERT( result == B_ENTRY_NOT_FOUND );
			// End of the list
			
		CPPUNIT_ASSERT( node2.RewindAttrs() == B_OK );
			// Rewind

		// The following crashes on R5. Our implementation just returns B_BAD_VALUE
		//CPPUNIT_ASSERT( node2.GetNextAttrName(NULL) != B_OK );
		
	}
	
};



#endif	// __sk_node_test_h__
