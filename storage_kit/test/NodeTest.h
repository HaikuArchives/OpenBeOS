#ifndef __sk_node_test_h__
#define __sk_node_test_h__

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include <Node.h>

class NodeTest : public CppUnit::TestCase
{
public:
	static Test* Suite() {
		CppUnit::TestSuite *suite = new CppUnit::TestSuite();
		
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::RemoveAttr()", &NodeTest::RemoveAttr) );
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::ReadAttr()", &NodeTest::ReadAttr) );
		
		return suite;
	}		

	// This function called before *each* test added in Suite()
	void setUp() {}
	
	// This function called after *each* test added in Suite()
	void tearDown()	{}

	void RemoveAttr() {
		BNode node;
		CPPUNIT_ASSERT( node.RemoveAttr("ThisWillFail") == B_ERROR );
	}

	void ReadAttr() {
		BNode node;
		CPPUNIT_ASSERT( node.ReadAttr("ThisWillFailToo", 0, 0, NULL, 0) == 0 );
	}
	
};



#endif	// __sk_node_test_h__
