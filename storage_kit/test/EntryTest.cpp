#include <EntryTest.h>

#include <Directory.h>
#include <Path.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/utsname.h> // Not needed
#include <sys/statvfs.h> // Not needed

//#include <string.h>

//#include <sys/stat.h>	// For struct stat
//#include <fs_attr.h>	// For struct attr_info

//#include <kernel_interface.h>

CppUnit::Test*
EntryTest::Suite() {
	CppUnit::TestSuite *suite = new CppUnit::TestSuite();
		
	suite->addTest( new CppUnit::TestCaller<EntryTest>("BEntry::Traversal Test", &EntryTest::TraversalTest) );
	suite->addTest( new CppUnit::TestCaller<EntryTest>("BEntry::Copy Constructor Test", &EntryTest::CopyConstructorTest) );
	suite->addTest( new CppUnit::TestCaller<EntryTest>("BEntry::Equality Test", &EntryTest::EqualityTest) );
	suite->addTest( new CppUnit::TestCaller<EntryTest>("BEntry::Assignment Test", &EntryTest::AssignmentTest) );
	suite->addTest( new CppUnit::TestCaller<EntryTest>("BEntry::Conversion Test", &EntryTest::ConversionTest) );
//	suite->addTest( new CppUnit::TestCaller<EntryTest>("BEntry::Miscellaneous Test", &EntryTest::MiscTest) );
	suite->addTest( new CppUnit::TestCaller<EntryTest>("BEntry::Existence Test", &EntryTest::ExistenceTest) );
	suite->addTest( new CppUnit::TestCaller<EntryTest>("BEntry::Rename Test", &EntryTest::RenameTest) );
	suite->addTest( new CppUnit::TestCaller<EntryTest>("BEntry::Stat Test", &EntryTest::StatTest) );
		
	return suite;
}		


EntryTest::EntryTest() {
	// Create our tripleLink filename, which is absolute (most of
	// the others are relative)
	CPPUNIT_ASSERT( getcwd(tripleLink, B_PATH_NAME_LENGTH) == tripleLink );
	sprintf(tripleLink, "%s%s%s", tripleLink,
		((tripleLink[strlen(tripleLink)-1] == '/') ? "" : "/"),
		tripleLinkLeaf);
}	

// This function called before *each* test added in Suite()
void
EntryTest::setUp() {
	SaveCWD();
}
	
// This function called after *each* test added in Suite()
void
EntryTest::tearDown()	{
	RestoreCWD();
}

// Creates a symbolic link named link that points to target
bool
EntryTest::CreateLink(const char *link, const char *target) {
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
	
// Creates the given file
bool
EntryTest::CreateFile(const char *file) {
	if (file == NULL)
		return false;
			
	int fd = ::open(file, O_CREAT | O_RDONLY);
	if (fd >= 0) {
		:: close(fd);
		return true;
	} else {
		return false;
	}
}
			
// Removes the given file if it exsists
bool
EntryTest::RemoveFile(const char *file) {
	if (file == NULL)
		return false;
			
	char str[1024];

	// If the link doesn't exist, we'll get an error message if we
	// try to remove it. So we'll just go ahead and make sure it
	// exists first :-)
	int fd = ::open(file, O_CREAT);
	if (fd >= 0)
		::close( fd );
			
	// Remove the old file
	sprintf(str, "rm %s", file);
	system(str);
		
	return true;
}

void
EntryTest::TraversalTest() {
	CreateLink(link, mainFile);
	CreateLink(relLink, link);
	CreateLink( tripleLink, relLink );
	CreateLink( fourLink, tripleLink );

	BEntry entry;
	CPPUNIT_ASSERT( entry.InitCheck() == B_NO_INIT );
		
	BEntry mainEntry(mainFile);
	CPPUNIT_ASSERT( mainEntry.InitCheck() == B_OK );
		
	BEntry linkEntry( link, true );
	CPPUNIT_ASSERT( linkEntry.InitCheck() == B_OK );
	CPPUNIT_ASSERT( linkEntry == mainEntry );
		
	BEntry relLinkEntry( relLink, true );
	CPPUNIT_ASSERT( relLinkEntry.InitCheck() == B_OK );
	CPPUNIT_ASSERT( relLinkEntry == mainEntry );
	CPPUNIT_ASSERT( relLinkEntry == linkEntry );
		
	BEntry tripleLinkEntry( tripleLink, true );
	CPPUNIT_ASSERT( tripleLinkEntry.InitCheck() == B_OK );
	CPPUNIT_ASSERT( tripleLinkEntry == mainEntry );
	CPPUNIT_ASSERT( tripleLinkEntry == linkEntry );
	CPPUNIT_ASSERT( tripleLinkEntry == relLinkEntry );
		
	BEntry fourLinkEntry( fourLink, true );
	CPPUNIT_ASSERT( fourLinkEntry.InitCheck() == B_OK );
	CPPUNIT_ASSERT( fourLinkEntry == mainEntry );
	CPPUNIT_ASSERT( fourLinkEntry == linkEntry );
	CPPUNIT_ASSERT( fourLinkEntry == relLinkEntry );
	CPPUNIT_ASSERT( fourLinkEntry == tripleLinkEntry );
				
	RemoveFile(link);
	RemoveFile(relLink);
	RemoveFile(tripleLink);
	RemoveFile(fourLink);		
}
	
void
EntryTest::CopyConstructorTest() {
	CreateLink(link, mainFile);
		
	BEntry entry(mainFile);
	BEntry entry2(entry);
	BEntry linkEntry(link);
	BEntry linkEntry2(linkEntry);
		
	CPPUNIT_ASSERT( entry.InitCheck() == B_OK );
	CPPUNIT_ASSERT( entry2.InitCheck() == B_OK );
	CPPUNIT_ASSERT( linkEntry.InitCheck() == B_OK );
	CPPUNIT_ASSERT( linkEntry2.InitCheck() == B_OK );
		
	CPPUNIT_ASSERT( entry == entry2 );
	CPPUNIT_ASSERT( entry != linkEntry );
	CPPUNIT_ASSERT( entry != linkEntry2 );

	CPPUNIT_ASSERT( entry2 == entry );
	CPPUNIT_ASSERT( entry2 != linkEntry );
	CPPUNIT_ASSERT( entry2 != linkEntry2 );
		
	CPPUNIT_ASSERT( linkEntry == linkEntry2 );
	CPPUNIT_ASSERT( linkEntry != entry );
	CPPUNIT_ASSERT( linkEntry != entry2 );
		
	CPPUNIT_ASSERT( linkEntry2 == linkEntry );
	CPPUNIT_ASSERT( linkEntry2 != entry );
	CPPUNIT_ASSERT( linkEntry2 != entry2 );
		
	// Uncomment the following lines if you want to double-check
	// the paths the various entries are pointing to
/*	cout << BPath(&entry).Path() << endl;
	cout << BPath(&entry2).Path() << endl;
	cout << BPath(&linkEntry).Path() << endl;
	cout << BPath(&linkEntry2).Path() << endl; */
		
	RemoveFile(link);		
}
	
// n1 and n2 should both be uninitialized. y1a and y1b should be initialized
// to the same entry, y2 should be initialized to a different entry
void
EntryTest::EqualityTest(BEntry &n1, BEntry &n2, BEntry &y1a, BEntry &y1b, BEntry &y2) {
	CPPUNIT_ASSERT( n1 == n2 );
	CPPUNIT_ASSERT( !(n1 != n2) );
	CPPUNIT_ASSERT( n1 != y2 );
	CPPUNIT_ASSERT( !(n1 == y2) );

	CPPUNIT_ASSERT( y1a != n2 );
	CPPUNIT_ASSERT( !(y1a == n2) );
	CPPUNIT_ASSERT( y1a == y1b );
	CPPUNIT_ASSERT( !(y1a != y1b) );
	CPPUNIT_ASSERT( y1a != y2 );
	CPPUNIT_ASSERT( !(y1a == y2) );

	CPPUNIT_ASSERT( n1 == n1 );
	CPPUNIT_ASSERT( !(n1 != n1) );
	CPPUNIT_ASSERT( y2 == y2 );
	CPPUNIT_ASSERT( !(y2 != y2) );			
}
	
void
EntryTest::EqualityTest() {
	BEntry n1, n2, y1a("/boot"), y1b("/boot"), y2("/");
	
	EqualityTest(n1, n2, y1a, y1b, y2);		
}

void
EntryTest::AssignmentTest() {	
	BEntry n1, n2, y1a("/boot"), y1b("/boot"), y2("/");

//	n1.Dump("n1");
//	n2.Dump("n2");
//	y1a.Dump("y1a");
//	y1b.Dump("y1b");
//	y2.Dump("y2");

	n1 = n1;		// self n
	y1a = y1b;		// psuedo self y
	y1a = y1a;		// self y
	n2 = y2;		// n = y
	y1b = n1;		// y = n
	y2 = y1a;		// y1 = y2
		
//	n1.Dump("n1");
//	y1b.Dump("n2");
//	y1a.Dump("y1a");
//	y2.Dump("y1b");
//	n2.Dump("y2");
		
		
	EqualityTest(n1, y1b, y1a, y2, n2);
}

void
EntryTest::ConversionTest() {
	BEntry entry;

	entry_ref ref;
	BPath path;
	BEntry entry2;
	BDirectory dir;
		
	char str[B_FILE_NAME_LENGTH];
		
	// Test unitialized BEntry
	CPPUNIT_ASSERT( entry.GetRef(&ref) == B_NO_INIT );
	CPPUNIT_ASSERT( entry.GetPath(&path) == B_NO_INIT );
	CPPUNIT_ASSERT( entry.GetParent(&entry2) == B_NO_INIT );
	CPPUNIT_ASSERT( entry.GetParent(&dir) == B_NO_INIT );
	CPPUNIT_ASSERT( entry.GetName(str) == B_NO_INIT );
		
	entry.SetTo("/boot");
		
	CPPUNIT_ASSERT( entry.InitCheck() == B_OK );
	CPPUNIT_ASSERT( entry.GetRef(&ref) == B_OK );
	CPPUNIT_ASSERT( entry.GetPath(&path) == B_OK );
	CPPUNIT_ASSERT( entry.GetParent(&entry2) == B_OK );
//	CPPUNIT_ASSERT( DecodeResult(entry.GetParent(&dir)) == B_OK );

	entry.SetTo("/");
		
	CPPUNIT_ASSERT( entry.InitCheck() == B_OK );
	CPPUNIT_ASSERT( entry.GetRef(&ref) == B_OK );
	CPPUNIT_ASSERT( DecodeResult(entry.GetPath(&path) == B_OK) );
	CPPUNIT_ASSERT( entry.GetParent(&entry2) == B_ENTRY_NOT_FOUND );
//	CPPUNIT_ASSERT( DecodeResult(entry.GetParent(&dir)) == B_OK );
		
	BEntry entry3;
		
	// Verify result of GetRef()
	entry3.SetTo(&ref);
	CPPUNIT_ASSERT( entry3.InitCheck() == B_OK );
	CPPUNIT_ASSERT( entry == entry3 );
		
	// Verify result of GetPath()
	entry3.SetTo(path.Path());
	CPPUNIT_ASSERT( entry3.InitCheck() == B_OK );
	CPPUNIT_ASSERT( entry == entry3 );
		
	// Verify result of GetParent() calls
//	entry3.SetTo(&dir, entry.Name());
//	CPPUNIT_ASSERT( entry.InitCheck() == B_OK );
//	CPPUNIT_ASSERT( entry == entry3 );

	// Test GetName()
	CPPUNIT_ASSERT( entry.GetName(str) == B_OK );
	CPPUNIT_ASSERT( strcmp(str, ".") == B_OK );
		
}

void
EntryTest::MiscTest() {
	BNode node(mainFile);
	BEntry entry(mainFile);
		
	CPPUNIT_ASSERT(node.Lock() == B_OK);
	CPPUNIT_ASSERT( entry.Exists() );
}
	
void
EntryTest::ExistenceTest() {
	BEntry realEntry("/");
	BEntry abstractEntry("/I_am_beggining_to_doubt_your_dedication_to_Sparkle_Motion");
	CPPUNIT_ASSERT( realEntry.Exists() );
	CPPUNIT_ASSERT( !abstractEntry.Exists() );
}

void
EntryTest::RenameTest() {
	// Verify attempts to rename root
	BEntry root("/");
	CPPUNIT_ASSERT( root.Rename("/", false) == B_FILE_EXISTS );
	CPPUNIT_ASSERT( root.Rename("/", true) == B_NOT_ALLOWED );
		
	// Verify abstract entries
	RemoveFile(sparkleMotion);		// Make sure it doesn't exist
	BEntry abstract(sparkleMotion);
	CPPUNIT_ASSERT( abstract.InitCheck() == B_OK );
	CPPUNIT_ASSERT( !abstract.Exists() );
	CPPUNIT_ASSERT( abstract.Rename("/boot/DoesntMatter") == B_ENTRY_NOT_FOUND );
	CPPUNIT_ASSERT( abstract.Rename("/boot/DontMatter", true) == B_ENTRY_NOT_FOUND );
	CPPUNIT_ASSERT( abstract.Rename("/DoesntMatter") == B_CROSS_DEVICE_LINK );
	CPPUNIT_ASSERT( abstract.Rename("/DontMatter", true) == B_CROSS_DEVICE_LINK );
		
	// Verify concrete entries renamed to absolute filenames
	CreateLink(sparkleMotion, mainFile);
	CPPUNIT_ASSERT( abstract.Exists() );
	CPPUNIT_ASSERT( RemoveFile(someFile) );
	CPPUNIT_ASSERT( abstract.Rename(someFile) == B_OK );
	CreateLink(sparkleMotion, "/boot");
	CPPUNIT_ASSERT( abstract.Rename(sparkleMotion, false) == B_FILE_EXISTS );
	CPPUNIT_ASSERT( abstract.Rename(sparkleMotion, true) == B_OK );
		
	// Verify that the rename properly clobbered the old link by checking that
	// it points to the mainFile and not "/boot"
	entry_ref ref;
	CPPUNIT_ASSERT( abstract.GetRef(&ref) == B_OK );
	BEntry verifier(&ref, true);	// Traverse the link
	BEntry verifiee(mainFile, true);
	CPPUNIT_ASSERT( verifier.InitCheck() == B_OK );
	CPPUNIT_ASSERT( verifiee.InitCheck() == B_OK );
	CPPUNIT_ASSERT( verifier == verifiee );
	CPPUNIT_ASSERT( verifier != abstract );
		
	// Verify concrete entries renamed to relative filenames
	CPPUNIT_ASSERT( abstract.Exists() );
//	cout << "abstract() == " << BPath( &abstract ).Path() << endl;
	RemoveFile("SomeCrazyLinkIsFun");
	CPPUNIT_ASSERT( DecodeResult(abstract.Rename("SomeCrazyLinkIsFun", false)) == B_FILE_EXISTS );
	CPPUNIT_ASSERT( DecodeResult(abstract.Rename("SomeCrazyLinkIsFun", true)) == B_OK );
		

	// Clean up
	RemoveFile(sparkleMotion);	

}
	
void
EntryTest::StatTest() {
	struct stat stat;

	// Unitialized Entry
	BEntry null;
	CPPUNIT_ASSERT( null.GetStat(&stat) == B_NO_INIT );
		
	// Abstract entry
	RemoveFile(sparkleMotion);
	BEntry sparkle(sparkleMotion);
	CPPUNIT_ASSERT( sparkle.InitCheck() == B_OK );
	CPPUNIT_ASSERT( !sparkle.Exists() );
	CPPUNIT_ASSERT( sparkle.GetStat(&stat) == B_ENTRY_NOT_FOUND );		
	
	// Concrete entry
	BEntry boot("/boot");
	CPPUNIT_ASSERT( boot.InitCheck() == B_OK );
	CPPUNIT_ASSERT( boot.GetStat(&stat) == B_OK );

	// Uncomment this test if you want to verify that GetStat() is actually
	// returning the proper info. This bit isn't compatible with the R5
	// Storage Kit, so it'll only link properly with our libstorage.
/*	StorageKit::DirEntry *entry;
	CPPUNIT_ASSERT( StorageKit::find_dir( boot.fDir, boot.fName, entry ) == B_OK );
	CPPUNIT_ASSERT( entry->d_dev == stat.st_dev );
	CPPUNIT_ASSERT( entry->d_ino == stat.st_ino );
*/
}
	
/* These two functions need to be moved somewhere else when I get the chance

// This is a non-CPPUNIT test called by EntryTest.Private.cpp to test
// our BEntry implmentation's private SplitPathInTwain() functions
void
EntryTest::SplitPathTest() {
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
		
}
	
void
EntryTest::DoSplitPathTest(const char *fullPath) {
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
		
*/
