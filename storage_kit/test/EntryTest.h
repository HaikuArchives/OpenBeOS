#ifndef __sk_entry_test_h__
#define __sk_entry_test_h__

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>

#include <Entry.h>

#include <sys/utsname.h> // Not needed
#include <sys/statvfs.h> // Not needed

//#include <stdio.h>

//#include <sys/stat.h>	// For struct stat
//#include <fs_attr.h>	// For struct attr_info

//#include <kernel_interface.h>

#include "TestUtils.h"

class EntryTest : public CppUnit::TestCase
{
public:
	static Test* Suite() {
		CppUnit::TestSuite *suite = new CppUnit::TestSuite();
		
		suite->addTest( new CppUnit::TestCaller<EntryTest>("BEntry::Init Test", &EntryTest::InitTest) );
		
		return suite;
	}		

	// This function called before *each* test added in Suite()
	void setUp() {}
	
	// This function called after *each* test added in Suite()
	void tearDown()	{}

	bool CreateLink(const char *link, const char *target) {
		if (link == NULL || target == NULL)
			return false;
			
		char str[1024];
		
		// If the link already exists, we want to remove it first. To
		// keep from dealing with error messages, we'll just create
		// the file first if it doesn't already exist, and then delete
		// it. :-)
		int fd = ::open(link, O_CREAT);
		if (fd >= 0)
			::close( fd );
		
		// Remove the old file
		sprintf(str, "rm %s", link);
		system(str);
		
		// Create a new one
		sprintf(str, "ln -s %s %s", target, link);
		system(str);
	
		return true;	
	}
	
	bool RemoveLink(const char *link) {
		if (link == NULL)
			return false;
			
		char str[1024];

		// If the link doesn't exist, we'll get an error message if we
		// try to remove it. So we'll just go ahead and make sure it
		// exists first :-)
		int fd = ::open(link, O_CREAT);
		if (fd >= 0)
			::close( fd );
			
		// Remove the old file
		sprintf(str, "rm %s", link);
		system(str);
		
		return true;
	}

	void InitTest() {
		BEntry entry;
		CPPUNIT_ASSERT( entry.InitCheck() == B_NO_INIT );
		
//		BEntry entry2("/cvsroot/open-beos/storage_kit/Jamfile", true);
		BEntry entry2("/cvsroot/open-beos/storage_kit/EntryTest.Link", true);
		CPPUNIT_ASSERT( entry2.InitCheck() == B_OK );

		CreateLink("EntryTest.Link", "/cvsroot/open-beos/storage_kit/Jamfile");
		BEntry entry3("/cvsroot/open-beos/storage_kit/EntryTest.Link", true);
		CPPUNIT_ASSERT( entry3.InitCheck() == B_OK );

		CreateLink("EntryTest.RelLink", "./EntryTest.Link");
		BEntry eRelLink( "/cvsroot/open-beos/storage_kit/EntryTest.RelLink", true );
		CPPUNIT_ASSERT( eRelLink.InitCheck() == B_OK );

		RemoveLink("EntryTest.RelLink");
		RemoveLink("EntryTest.Link");
	}

	// This is a non-CPPUNIT test called by EntryTest.Private.cpp to test
	// our BEntry implmentation's private SplitPathInTwain() functions
	void SplitPathTest() {
		DoSplitPathTest(NULL);
		DoSplitPathTest("");
		DoSplitPathTest("/");
		DoSplitPathTest("/////");
		DoSplitPathTest("this/is/a/relative/path/leaf");
		DoSplitPathTest("/this/is/an/absolute/path/leaf");
		DoSplitPathTest("this/is/a/relative/path/dir/");
		DoSplitPathTest("/this/is/an/absolute/path/dir/");
		DoSplitPathTest("/leaf_in_root_dir");
		DoSplitPathTest("/dir_in_root_dir/");
		DoSplitPathTest("leaf_only");
		DoSplitPathTest("dir_only/");
		DoSplitPathTest("path////redundant_slashes_are_stupid////");
		
		DIR* dir = opendir("/");
		if (dir != NULL) {
			printf("ttyname() == %s\n", ttyname(dir->fd));
		}
		
		char str[1024];
		printf("getcwd() == %s\n", getcwd(str, 1024));
		printf("getlogin() == %s\n", getlogin());

		struct utsname name;
		uname(&name);
		cout << "sysname == " << name.sysname << endl;
		cout << "nodename == " << name.nodename << endl;
		cout << "release == " << name.release << endl;
		cout << "version == " << name.version << endl;
		cout << "machine == " << name.machine << endl;

		struct statvfs v;
		fstatvfs(dir->fd, &v);
		cout << "f_bsize == " << v.f_bsize << endl;
		cout << "f_frsize == " << v.f_frsize << endl;
		cout << "f_blocks == " << v.f_blocks << endl;
		cout << "f_bfree == " << v.f_bfree << endl;
		cout << "f_bavail == " << v.f_bavail << endl;
		cout << "f_files == " << v.f_files << endl;
		cout << "f_ffree == " << v.f_ffree << endl;
		cout << "f_favail == " << v.f_favail << endl;
		cout << "f_fsid == " << v.f_fsid << endl;
		cout << "f_flag == " << v.f_flag << endl;
		cout << "f_namemax == " << v.f_namemax << endl;
		
/*
struct statvfs {
	unsigned long f_bsize;
	unsigned long f_frsize;
	fsblkcnt_t    f_blocks;
	fsblkcnt_t    f_bfree;
	fsblkcnt_t    f_bavail;
	fsfilcnt_t    f_files;
	fsfilcnt_t    f_ffree;
	fsfilcnt_t    f_favail;
	unsigned long f_fsid;
	unsigned long f_flag;
	unsigned long f_namemax;
};


__extern_c_start

int statvfs(const char *path, struct statvfs *buf);
int fstatvfs(int fildes, struct statvfs *buf);

/*	char sysname[32];
	char nodename[32];
	char release[32];
	char [32];
	char machine[32];
};

__extern_c_start

int uname(struct utsname *name); */


	}
	
	void DoSplitPathTest(const char *fullPath) {
		BEntry entry;
		char *path, *leaf;
		
		cout << "------------------------------------------------------------" << endl;
		if (fullPath == NULL)
			cout << "NULL" << endl;
		else
			cout << "'" << fullPath << "'" << endl;
		cout << "------------------------------------------------------------" << endl;

		entry.SplitPathInTwain(fullPath, path, leaf);
		
		cout << "path == ";
		if (path == NULL)
			cout << "NULL";
		else
			cout << "'" << path << "'";
		cout << endl;		
		cout << "leaf == ";
		if (leaf == NULL)
			cout << "NULL";
		else
			cout << "'" << leaf << "'";
		cout << endl << endl;
		

		delete [] path;
		delete [] leaf;
	}		
		

};


#endif	// __sk_entry_test_h__
