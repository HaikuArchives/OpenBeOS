#ifndef __sk_node_test_h__
#define __sk_node_test_h__

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include <Node.h>

#include <stdio.h>

#include <sys/stat.h>	// For struct stat
#include <fs_attr.h>	// For struct attr_info

#include <kernel_interface.h>

#include "TestUtils.h"

class NodeTest : public CppUnit::TestCase
{
public:
	static Test* Suite() {
		CppUnit::TestSuite *suite = new CppUnit::TestSuite();
		
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::Init Test", &NodeTest::InitTest) );
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::Attribute Directory Test", &NodeTest::AttrDirTest) );
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::Attribute Read/Write/Remove Test", &NodeTest::AttrTest) );
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::Attribute Rename Test (this fails with R5 libraries)", &NodeTest::AttrRenameTest) );
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::Attribute Info Test", &NodeTest::AttrInfoTest) );
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::Stat Test", &NodeTest::StatTest) );
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::Sync Test", &NodeTest::SyncTest) );
		suite->addTest( new CppUnit::TestCaller<NodeTest>("BNode::Dup Test", &NodeTest::DupTest) );
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
//			cout << str << endl;	// <<< Uncomment to list off the attribute names
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
	

	void AttrRenameTest() {
		const char attr1[] = "StorageKit::SomeAttribute";
		const char attr2[] = "StorageKit::AnotherAttribute";
		const char str[] = "This is my testing string and it rules your world.";
		const int strLen = strlen(str) + 1;
		const int dataLen = 1024;
		char data[dataLen];
		ssize_t bytes;
		
		BNode node;
		CPPUNIT_ASSERT( node.RenameAttr(attr1, attr2) == B_FILE_ERROR );
		
		node.SetTo("./");

		// Test the case of the first attribute not existing
		node.RemoveAttr(attr1);		
		CPPUNIT_ASSERT( node.RenameAttr(attr1, attr2) == B_BAD_VALUE );
		
		// Write an attribute, read it to verify it, rename it, read the
		// new attribute, read the old (which fails), and then remove the new.
		CPPUNIT_ASSERT( node.WriteAttr(attr1, B_STRING_TYPE, 0, str, strLen) == strLen );
		CPPUNIT_ASSERT( node.ReadAttr(attr1, B_STRING_TYPE, 0, data, dataLen) == strLen );
		CPPUNIT_ASSERT( strcmp(data, str) == 0 );		
		CPPUNIT_ASSERT( node.RenameAttr(attr1, attr2) == B_OK ); // <<< This fails with R5::BNode
		CPPUNIT_ASSERT( node.ReadAttr(attr1, B_STRING_TYPE, 0, data, dataLen) == B_ENTRY_NOT_FOUND );
		CPPUNIT_ASSERT( node.ReadAttr(attr2, B_STRING_TYPE, 0, data, dataLen) == strLen );
		CPPUNIT_ASSERT( strcmp(data, str) == 0 );
		CPPUNIT_ASSERT( node.RemoveAttr(attr2) == B_OK );
	}
	
	void AttrInfoTest() {
		const char attr[] = "StorageKit::SomeAttribute";
		const char str[] = "This is the greatest string ever.";
		const int len = strlen(str) + 1;
		
		BNode node("./");
		CPPUNIT_ASSERT( node.WriteAttr(attr, B_STRING_TYPE, 0, str, len) == len );
		
		attr_info info;
		CPPUNIT_ASSERT( node.GetAttrInfo(attr, &info) == B_OK );
		CPPUNIT_ASSERT( info.type == B_STRING_TYPE );
		CPPUNIT_ASSERT( info.size == len );
		
		node.RemoveAttr(attr);
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

	// This doesn't actually verify synching is occuring; just
	// checks for a B_OK return value.
	void SyncTest() {
		const char attr[] = "StorageKit::SomeAttribute";
		const char str[] = "This string rules your world.";
		const int len = strlen(str) + 1;
	
		BNode node("./");
		CPPUNIT_ASSERT( node.WriteAttr(attr, B_STRING_TYPE, 0, str, len) == len );
		CPPUNIT_ASSERT( node.Sync() == B_OK );
		CPPUNIT_ASSERT( node.RemoveAttr(attr) == B_OK );	
	}
	
	void DupTest() {
		BNode node("./");
		int fd = node.Dup();
		CPPUNIT_ASSERT( fd != -1 );	// Should this be ">= 0" maybe?
		::close(fd);
	}
	
	// Locking isn't really implemented yet...
	void LockTest() {
		BNode node;
		
		BNode node2("./");
		CPPUNIT_ASSERT( DecodeResult(node2.Lock()) == B_OK );
	
	}
	
};



#endif	// __sk_node_test_h__
