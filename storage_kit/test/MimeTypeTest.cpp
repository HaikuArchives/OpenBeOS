// MimeTypeTest.cpp

#include <stdio.h>
#include <string.h>			// For memcmp()
#include <string>
#include <unistd.h>
#include <vector>


#include <Application.h>
#include <Bitmap.h>
#include <DataIO.h>
#include <Message.h>
#include <Mime.h>
#include <MimeTypeTest.h>
#include <Path.h>			// Only needed for entry_ref dumps
#include <StorageKit.h>
#include <String.h>


#include "Test.StorageKit.h"
#include "TestApp.h"
#include "TestUtils.h"

static const char *testDir				= "/tmp/mimeTestDir";
static const char *mimeDatabaseDir		= "/boot/home/config/settings/beos_mime";
static const char *testType				= "text/StorageKit-Test";
static const char *testTypeApp			= "application/StorageKit-Test";
static const char *testTypeInvalid		= "text/Are spaces valid?";
static const char *testApp				= "/boot/beos/apps/SoundRecorder";
static const char *testApp2				= "/boot/beos/apps/CDPlayer";
static const char *fakeTestApp			= "/__this_isn't_likely_to_exist__";
static const char *typeField			= "type";
static const char *fileExtField			= "extensions";
// Descriptions
static const char *testDescr			= "Just a test, nothing more :-)";
static const char *testDescr2			= "Another amazing test string";
static const char *longDescr			= 
"This description is longer than B_MIME_TYPE_LENGTH, which is quite useful for certain things... "
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
"cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
"dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd";
// Signatures
static const char *testSig				= "application/x-vnd.obos.mime-type-test";
static const char *testSig2				= "application/x-vnd.obos.mime-type-test-2";
static const char *longSig				= "application/x-vnd.obos.mime-type-test-long."
"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
"cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc"
"dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd";

// Handy comparison operators for BBitmaps. The size and color depth
// are compared first, followed by the bitmap data.
bool operator==(BBitmap &bmp1, BBitmap &bmp2) {
	if (bmp1.Bounds() == bmp2.Bounds()) {
//		printf("bmp== 1\n");
		if (bmp1.ColorSpace() == bmp2.ColorSpace()) {
//			printf("bmp== 2\n");
			char *data1 = (char*)bmp1.Bits();
			char *data2 = (char*)bmp2.Bits();
			// NOTE! It's possible that padding bits might not get copied verbatim,
			// which could lead to unexpected failures. If things are acting weird,
			// you might try the commented out code below (keeping in mind that it
			// currently only works for 8-bit color depths).
			for (int i = 0; i < bmp1.BitsLength(); data1++, data2++, i++) {
				if (*data1 != *data2) {
//					printf("i == %d\n", i);
					return false;
				}
			} 
/*			for (int i = 0; i < bmp1.Bounds().IntegerHeight(); i++) {
				for (int j = 0; j < bmp1.Bounds().IntegerWidth(); j++) {
//					printf("(%d, %d)", data1[(i * bmp1.BytesPerRow()) + j], data2[(i * bmp2.BytesPerRow()) + j]);
					if (data1[(i * bmp1.BytesPerRow()) + j] != data2[(i * bmp2.BytesPerRow()) + j]) {
						printf("fail at (%d, %d)\n", j, i);
						return false;
					}
				}
			}*/
			return true;
		} else
			return false;
	} else
		return false;
}

bool operator!=(BBitmap &bmp1, BBitmap &bmp2) {
	return !(bmp1 == bmp2);
}

// Handy comparison operators for BMessages. The BMessages are checked field
// by field, each of which is verified to be identical with respect to: type,
// count, and data (all items).
bool operator==(BMessage &msg1, BMessage &msg2) {
	status_t err = B_OK;
	
	// For now I'm ignoring the what fields...I shall deal with that later :-)
	if (msg1.what != msg2.what)
		return false;

/*
	printf("----------------------------------------------------------------------\n");
	msg1.PrintToStream();
	msg2.PrintToStream();
	printf("----------------------------------------------------------------------\n");
*/
	
	// Check the counts of field names
	int count1, count2;	
	count1 = msg1.CountNames(B_ANY_TYPE);
	count2 = msg2.CountNames(B_ANY_TYPE);
	if (count1 != count2 && (count1 == 0 || count2 == 0))
		return false;
		
	// Iterate over all the names in msg1 and check that the field
	// with the same name exists in msg2, is of the same type, and
	// contains identical data.
	for (int i = 0; i < count1 && !err; i++) {
		char *name;
		type_code typeFound1, typeFound2;
		int32 countFound1, countFound2;
		
		// Check type and count info
		err = msg1.GetInfo(B_ANY_TYPE, i, &name, &typeFound1, &countFound1);			
		if (!err) 
			err = msg2.GetInfo(name, &typeFound2, &countFound2);
		if (!err) 
			err = (typeFound1 == typeFound2 && countFound1 == countFound2 ? B_OK : B_ERROR);
		if (!err) {
			// Check all the data items
			for (int j = 0; j < countFound1; j++) {
				void *data1, *data2;
				ssize_t bytes1, bytes2;
				
				err = msg1.FindData(name, typeFound1, j, (const void**)&data1, &bytes1);
				if (!err) 
					err = msg2.FindData(name, typeFound2, j, (const void**)&data2, &bytes2);
				if (!err) 
					err = (bytes1 == bytes2 && memcmp(data1, data2, bytes1) == 0 ? B_OK : B_ERROR);			
			}
		}
	}
	
	return !err;
}

bool operator!=(BMessage &msg1, BMessage &msg2) {
	return !(msg1 == msg2);
}


// Suite
CppUnit::Test*
MimeTypeTest::Suite() {
	StorageKit::TestSuite *suite = new StorageKit::TestSuite();
	typedef CppUnit::TestCaller<MimeTypeTest> TC;
		
	// Tyler
	suite->addTest( new TC("BMimeType::App Hint Test",
						   &MimeTypeTest::AppHintTest) );
	suite->addTest( new TC("BMimeType::File Extensions Test",
						   &MimeTypeTest::FileExtensionsTest) );
	suite->addTest( new TC("BMimeType::Large Icon Test",
						   &MimeTypeTest::LargeIconTest) );
	suite->addTest( new TC("BMimeType::Mini Icon Test",
						   &MimeTypeTest::MiniIconTest) );
	suite->addTest( new TC("BMimeType::Large Icon For Type Test",
						   &MimeTypeTest::LargeIconForTypeTest) );
	suite->addTest( new TC("BMimeType::Mini Icon For Type Test",
						   &MimeTypeTest::MiniIconForTypeTest) );
	suite->addTest( new TC("BMimeType::Long Description Test",
						   &MimeTypeTest::LongDescriptionTest) );
	suite->addTest( new TC("BMimeType::Short Description Test",
						   &MimeTypeTest::ShortDescriptionTest) );
	suite->addTest( new TC("BMimeType::Preferred App Test",
						   &MimeTypeTest::PreferredAppTest) );

	// Ingo						   
	suite->addTest( new TC("BMimeType::Initialization Test",
						   &MimeTypeTest::InitTest) );
	suite->addTest( new TC("BMimeType::MIME String Test",
						   &MimeTypeTest::StringTest) );

	return suite;
}		

// Fills the bitmap data with the given character
void FillBitmap(BBitmap &bmp, char value) {
	char *data = (char*)bmp.Bits();
	for (int i = 0; i < bmp.BitsLength(); data++, i++) {
//		printf("(%d -> ", *data);
		*data = value;
//		printf("%d)", *data);
	}
//	printf("\n");
}
	
// Dumps the size, colorspace, and first data byte
// of the bitmap to stdout
void DumpBitmap(BBitmap &bmp, char *name = "bmp") {
	printf("%s == (%dx%d, ", name, bmp.Bounds().IntegerWidth()+1,
		bmp.Bounds().IntegerHeight()+1);
	switch (bmp.ColorSpace()) {
		case B_CMAP8:
			printf("B_CMAP8");
			break;
				
		case B_RGBA32:
			printf("B_RGBA32");
			break;
				
		default:
			printf("%x", bmp.ColorSpace());
			break;		
	}
	printf(", %d)\n", *(char*)bmp.Bits());
}
	
// IconHelper and IconForTypeHelper:
// Adapter(?) classes needed to reuse icon tests among {Get,Set}Icon() and {Get,Set}IconForType()
// What originally were meant to encapsulate the variations among calls to the various BMimeType
// icon functions have now exploded into beefy helper classes with a bunch of BBitmap related
// functionality as well. A lot of this stuff doesn't necessarily belong

class IconHelper {
public:
	IconHelper(icon_size which)
		: size(which),
		  bmp1(BitmapBounds(which), B_CMAP8),
		  bmp2(BitmapBounds(which), B_CMAP8),
		  bmpTemp(BitmapBounds(which), B_CMAP8)
	{
		// Initialize our three bitmaps to different "colors"
		FillBitmap(bmp1, 1);
		FillBitmap(bmp2, 2);
		FillBitmap(bmpTemp, 3);
	}
	
	// Returns the proper bitmap bounds for the given icon size
	BRect BitmapBounds(icon_size isize) {
		return isize == B_LARGE_ICON ? BRect(0,0,31,31) : BRect(0,0,15,15);
	}
	
	// Returns the proper bitmap bounds for this helper's icon size
	BRect BitmapBounds() {
		return BitmapBounds(size);
	}	
	
	// Used to call the appropriate GetIcon[ForType] function
	virtual status_t GetIcon(BMimeType &mime, BBitmap *icon) {
		return mime.GetIcon(icon, size);
	}

	// Used to call the appropriate SetIcon[ForType] function
	virtual status_t SetIcon(BMimeType &mime, BBitmap *icon) {
		return mime.SetIcon(icon, size);
	}
	
	BBitmap* TempBitmap() {
		return &bmpTemp;
	}
	
	BBitmap* Bitmap1() {
		return &bmp1;
	}
	
	BBitmap* Bitmap2() {
		return &bmp2;
	}
	
	icon_size Size() {
		return size;
	}
	
protected:	
	BBitmap bmp1;
	BBitmap bmp2;
	BBitmap bmpTemp;
	icon_size size;	
};

class IconForTypeHelper : public IconHelper {
public:
	IconForTypeHelper(const char *fileType, icon_size which)
		: IconHelper(which), fileType(fileType) {}
	virtual status_t GetIcon(BMimeType &mime, BBitmap *icon) {
		return mime.GetIconForType(fileType.c_str(), icon, size);
	}
	virtual status_t SetIcon(BMimeType &mime, BBitmap *icon) {
		return mime.SetIconForType(fileType.c_str(), icon, size);
	}
protected:
	std::string fileType;
};

// setUp
void
MimeTypeTest::setUp()
{
	BasicTest::setUp();
/*	// Better not to play with fire, so we'll make a copy of the
	// local mime database which we'll use for certain OpenBeOS tests
	execCommand(string("mkdir ") + testDir
				+ " ; copyattr -d -r -- " + mimeDatabaseDir + "/* " + testDir
				); */	
	// Setup our application
	fApplication = new TestApp(testSig);
	if (fApplication->Init() != B_OK) {
		fprintf(stderr, "Failed to initialize application.\n");
		delete fApplication;
		fApplication = NULL;
	}
	
}
	
// tearDown
void
MimeTypeTest::tearDown()
{
//	execCommand(string("rm -rf ") + testDir);

	// Uninistall our test type
/*	BMimeType mime(testType);
	status_t err = mime.InitCheck();
	if (!err && mime.IsInstalled())
		err = mime.Delete();
	if (err)
		fprintf(stderr, "Failed to unistall test type \"%s\"\n", testType); */
	// Terminate the Application
	if (fApplication) {
		fApplication->Terminate();
		delete fApplication;
		fApplication = NULL;
	}
	BasicTest::tearDown();
}

// entry_ref dumping function ; this may be removed at any time

void printRef(entry_ref *ref, char* name = "ref") {
	if (ref) {
		BPath path(ref);
		status_t err = path.InitCheck();
		if (!err) {
			printf("%s == '%s'", name, path.Path());
		} else
			printf("%s == ERROR", name);
		printf(" == (%d, %d, '%s')\n", ref->device, ref->directory, ref->name);
		
	} else
		printf("%s == (NULL)\n", name);
}
		
// App Hint

void MimeTypeTest::AppHintTest() {
	// init a couple of entry_refs to applications
	BEntry entry(testApp);
	entry_ref appRef;
	CHK(entry.InitCheck() == B_OK);
	CHK(entry.GetRef(&appRef) == B_OK);	
	BEntry entry2(testApp2);
	entry_ref appRef2;
	CHK(entry2.InitCheck() == B_OK);
	CHK(entry2.GetRef(&appRef2) == B_OK);	
	// Uninitialized
	nextSubTest();
	{
		BMimeType mime;
		entry_ref ref;
		CHK(mime.InitCheck() == B_NO_INIT);
		CHK(mime.GetAppHint(&ref) != B_OK);		// R5 == B_BAD_VALUE
		CHK(mime.SetAppHint(&ref) != B_OK);		// R5 == B_BAD_VALUE
	}
	// NULL params
	nextSubTest();
	{
		entry_ref ref;
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Make sure the type isn't installed
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		CHK(!mime.IsInstalled());
		CHK(mime.GetAppHint(NULL) != B_OK);		// R5 == B_BAD_VALUE
		CHK(!mime.IsInstalled());
		CHK(mime.SetAppHint(NULL) != B_OK);		// Installs, R5 == B_ENTRY_NOT_FOUND
		CHK(mime.IsInstalled());
		CHK(mime.GetAppHint(NULL) != B_OK);		// R5 == B_BAD_VALUE
		CHK(mime.SetAppHint(NULL) != B_OK);		// R5 == B_ENTRY_NOT_FOUND
	}
	// Non-installed type
	nextSubTest();
	{
		entry_ref ref;
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Make sure the type isn't installed
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		CHK(!mime.IsInstalled());
		CHK(mime.GetAppHint(&ref) != B_OK);		// R5 == B_ENTRY_NOT_FOUND
		CHK(!mime.IsInstalled());
		CHK(mime.SetAppHint(&appRef) == B_OK);
		CHK(mime.IsInstalled());
		CHK(mime.GetAppHint(&ref) == B_OK);
		CHK(ref == appRef);
	}
	// Installed Type
	nextSubTest();
	{
		entry_ref ref;
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		CHK(mime.IsInstalled());
		// Get() with no apphint installed
		CHK(mime.GetAppHint(&ref) == B_ENTRY_NOT_FOUND);
		// Initial Set()/Get()
		CHK(mime.SetAppHint(&appRef) == B_OK);
		CHK(mime.GetAppHint(&ref) == B_OK);
		CHK(ref == appRef);
		// Followup Set()/Get()
		CHK(mime.SetAppHint(&appRef2) == B_OK);
		CHK(mime.GetAppHint(&ref) == B_OK);
		CHK(ref == appRef2);
		CHK(ref != appRef);
	}
	// Installed Type, invalid entry_ref
	nextSubTest();
	{
		entry_ref ref(-1, -1, NULL);
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);		
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		CHK(mime.IsInstalled());		
		CHK(mime.SetAppHint(&appRef) == B_OK);
		CHK(mime.SetAppHint(&ref) != B_OK);	// R5 == B_BAD_VALUE
	}
	// Installed Type, fake/invalid entry_ref
	nextSubTest();
	{
		entry_ref ref(0, 0, "__this_ought_not_exist__");
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);		
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);			
		CHK(mime.IsInstalled());		
		CHK(mime.SetAppHint(&appRef) == B_OK);
		CHK(mime.SetAppHint(&ref) != B_OK);	// R5 == B_ENTRY_NOT_FOUND
	}		
	// Installed Type, abstract entry_ref
	nextSubTest();
	{
		entry_ref fakeRef;
		entry_ref ref;
		BEntry entry(fakeTestApp);
		CHK(entry.InitCheck() == B_OK);
		CHK(!entry.Exists());
		CHK(entry.GetRef(&fakeRef) == B_OK);
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);		
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);			
		CHK(mime.IsInstalled());		
		CHK(mime.SetAppHint(&appRef) == B_OK);
		CHK(mime.SetAppHint(&fakeRef) == B_OK);
		CHK(mime.GetAppHint(&ref) == B_OK);
		CHK(ref == fakeRef);
		CHK(ref != appRef);
	}		
}

// File Extensions

void MimeTypeTest::FileExtensionsTest() {
	// Create some messages to sling around
	const int32 WHAT = 234;	// This is the what value that GFE returns...not sure if it has a name yet
	BMessage msg1(WHAT), msg2(WHAT), msg3(WHAT);

	CHK(msg1.AddString(fileExtField, ".data") == B_OK);
	CHK(msg1.AddString(fileExtField, ".txt") == B_OK);
	CHK(msg1.AddString(fileExtField, ".png") == B_OK);
	CHK(msg1.AddString(fileExtField, ".html") == B_OK);

	CHK(msg2.AddString(fileExtField, ".data") == B_OK);
	CHK(msg2.AddString(fileExtField, ".txt") == B_OK);
	
	CHK(msg3.AddString(fileExtField, ".data") == B_OK);
	CHK(msg3.AddString(fileExtField, ".txt") == B_OK);
	
	CHK(msg1 == msg1);
	CHK(msg2 == msg2);
	CHK(msg3 == msg3);
	CHK(msg1 != msg2);
	CHK(msg1 != msg3);
	CHK(msg2 == msg3);

	// Uninitialized
	nextSubTest();
	{
		BMessage msg(WHAT);
		BMimeType mime;
		
		CHK(mime.InitCheck() == B_NO_INIT);
		CHK(mime.GetFileExtensions(&msg) != B_OK);	// R5 == B_BAD_VALUE
		CHK(mime.SetFileExtensions(&msg) != B_OK);	// R5 == B_BAD_VALUE
	}
	// NULL params
	nextSubTest();
	{
		BMessage msg(WHAT);
		BMimeType mime(testType);
		
		CHK(mime.InitCheck() == B_OK);
		// Make sure the type isn't installed
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		CHK(!mime.IsInstalled());
		// Not installed
		CHK(mime.GetFileExtensions(NULL) != B_OK);	// R5 == B_BAD_VALUE
		CHK(!mime.IsInstalled());
#if !SK_TEST_R5
		CHK(RES(mime.SetFileExtensions(NULL)) == B_OK);	// R5 == CRASH!
		CHK(mime.IsInstalled());
		CHK(RES(mime.GetFileExtensions(&msg)) != B_OK);	// R5 == B_ENTRY_NOT_FOUND
		// Installed
/*		CHK(mime.GetFileExtensions(NULL) != B_OK);	// R5 == B_BAD_VALUE
		CHK(mime.SetFileExtensions(helper.Bitmap1()) == B_OK);
		CHK(mime.GetFileExtensions(bmp) == B_OK);
		CHK(*bmp == *helper.Bitmap1());
		CHK(mime.SetFileExtensions(NULL) == B_OK);
		CHK(mime.GetFileExtensions(bmp) != B_OK);	// R5 == B_ENTRY_NOT_FOUND*/
#endif
	}
	// Set() with empty message
	nextSubTest();
	{
		BMimeType mime(testType);
		BMessage msgEmpty(WHAT);
		BMessage msg(WHAT);
		CHK(msg.AddInt32("stuff", 1234) == B_OK);	// Add an extra attribute to give us something to compare with
		
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);			
		CHK(mime.IsInstalled());
		
		// Set(empty)
		CHK(msg != msgEmpty);
		CHK(mime.SetFileExtensions(&msgEmpty) == B_OK);
		CHK(mime.GetFileExtensions(&msg) == B_OK);
		CHK(msgEmpty.AddString(typeField, testType) == B_OK);	// Add in "type" fields as GFE() does
		CHK(msg == msgEmpty);
	}
	// Set() with extra attributes in message
	nextSubTest();
	{
		BMimeType mime(testType);
		BMessage msg(WHAT);
		BMessage msgExtraSet(msg1);
		CHK(msgExtraSet.AddString("extra", ".extra") == B_OK);
		CHK(msgExtraSet.AddInt32("more_extras", 123) == B_OK);
		CHK(msgExtraSet.AddInt32("more_extras", 456) == B_OK);
		CHK(msgExtraSet.AddInt32("more_extras", 789) == B_OK);
		BMessage msgExtraGet(msgExtraSet);
		
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);			
		CHK(mime.IsInstalled());
		
		// Set(extra)/Get(empty)
		msg1.RemoveName(typeField);		// Clear "type" fields, since SFE() just adds another
		msg2.RemoveName(typeField);	
		CHK(msg != msg1);
		CHK(msg != msgExtraSet);
		CHK(mime.SetFileExtensions(&msgExtraSet) == B_OK);
		CHK(mime.GetFileExtensions(&msg) == B_OK);
		CHK(msg1.AddString(typeField, testType) == B_OK);	// Add in "type" fields as GFE() does
		CHK(msgExtraSet.AddString(typeField, testType) == B_OK);
		CHK(msg == msgExtraSet);
		CHK(msg != msg1);
		
		// Get(extra)
		nextSubTest();
		CHK(mime.GetFileExtensions(&msgExtraGet) == B_OK);
		CHK(msgExtraGet == msgExtraSet);
		CHK(msgExtraGet != msg1);
		
		// Get(extra and then some)
		nextSubTest();
		CHK(msgExtraGet.AddInt32("more_extras", 101112) == B_OK);
		msgExtraGet.RemoveName(typeField);		// Clear "type" fields to be fair, since SFE() just adds another
		CHK(mime.GetFileExtensions(&msgExtraGet) == B_OK);	// Reinitializes result (clearing extra fields)
		CHK(msgExtraGet == msgExtraSet);
		CHK(msgExtraGet != msg1);
		
	}
	// Normal function
	nextSubTest();
	{
		BMessage msg(WHAT);
		BMimeType mime(testType);
		
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);			
		CHK(mime.IsInstalled());

		// Initial Set()/Get()
		msg1.RemoveName(typeField);		// Clear "type" fields, since SFE() just adds another
		msg2.RemoveName(typeField);	
		CHK(msg != msg1);
		CHK(msg != msg2);
		CHK(mime.SetFileExtensions(&msg1) == B_OK);
		CHK(mime.GetFileExtensions(&msg) == B_OK);
		CHK(msg1.AddString(typeField, testType) == B_OK);	// Add in "type" fields as GFE() does
		CHK(msg2.AddString(typeField, testType) == B_OK);
		CHK(msg == msg1);
		CHK(msg != msg2);

		// Followup Set()/Get()
		nextSubTest();
		CHK(msg.MakeEmpty() == B_OK);
		msg1.RemoveName(typeField);		// Clear "type" fields, since SFE() just adds another
		msg2.RemoveName(typeField);	
		CHK(msg != msg1);
		CHK(msg != msg2);
		CHK(mime.SetFileExtensions(&msg2) == B_OK);
		CHK(mime.GetFileExtensions(&msg) == B_OK);
		CHK(msg1.AddString(typeField, testType) == B_OK);	// Add in "type" fields as GFE() does
		CHK(msg2.AddString(typeField, testType) == B_OK);
		CHK(msg != msg1);
		CHK(msg == msg2);

		// Clear
		nextSubTest();
		CHK(msg.MakeEmpty() == B_OK);
		msg1.RemoveName(typeField);		// Clear "type" fields, since SFE() just adds another
		msg2.RemoveName(typeField);	
		CHK(msg != msg1);
		CHK(msg != msg2);
#if !SK_TEST_R5
		CHK(RES(mime.SetFileExtensions(NULL)) == B_OK);		// R5 == CRASH! despite what the BeBook says
		CHK(RES(mime.GetFileExtensions(&msg)) != B_OK);
		CHK(msg1.AddString(typeField, testType) == B_OK);	// Add in "type" fields as GFE() does
		CHK(msg2.AddString(typeField, testType) == B_OK);
		CHK(msg != msg1);
		CHK(msg != msg2);
#endif
	}
}


// Icon Test Helper Function

void MimeTypeTest::IconTest(IconHelper &helper) {
	BBitmap *bmp = helper.TempBitmap();
	// Unitialized 
	nextSubTest();
	{
		BMimeType mime;
		CHK(mime.InitCheck() == B_NO_INIT);
		CHK(helper.GetIcon(mime, bmp) != B_OK);	// R5 == B_BAD_VALUE
		CHK(helper.SetIcon(mime, bmp) != B_OK);	// R5 == B_BAD_VALUE
	}
	// Non-installed type
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Make sure the type isn't installed
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		CHK(!mime.IsInstalled());
		// Test
		CHK(helper.GetIcon(mime, bmp) != B_OK);	// R5 == B_ENTRY_NOT_FOUND
		CHK(!mime.IsInstalled());
		CHK(helper.SetIcon(mime, helper.Bitmap1()) == B_OK);
		CHK(mime.IsInstalled());
		CHK(helper.GetIcon(mime, bmp) == B_OK);
		CHK(*bmp == *helper.Bitmap1());
	}
	// NULL params
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Make sure the type isn't installed
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		CHK(!mime.IsInstalled());
		// Uninstalled
		CHK(helper.GetIcon(mime, NULL) != B_OK);	// R5 == B_BAD_VALUE
		CHK(!mime.IsInstalled());
		CHK(helper.SetIcon(mime, NULL) != B_OK);	// R5 == Installs, B_ENTRY_NOT_FOUND
		CHK(mime.IsInstalled());
		CHK(helper.GetIcon(mime, bmp) != B_OK);	// R5 == B_ENTRY_NOT_FOUND
		// Installed
		CHK(helper.GetIcon(mime, NULL) != B_OK);	// R5 == B_BAD_VALUE
		CHK(helper.SetIcon(mime, helper.Bitmap1()) == B_OK);
		CHK(helper.GetIcon(mime, bmp) == B_OK);
		CHK(*bmp == *helper.Bitmap1());
		CHK(helper.SetIcon(mime, NULL) == B_OK);
		CHK(helper.GetIcon(mime, bmp) != B_OK);	// R5 == B_ENTRY_NOT_FOUND
	}
	// Invalid Bitmap Size (small -- 10x10)
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		CHK(mime.IsInstalled());
		// Init Test Bitmap
		BBitmap testBmp(BRect(0,0,9,9), B_CMAP8);
		FillBitmap(testBmp, 3);
		// Test Set()
		CHK(testBmp != *helper.Bitmap1());
		CHK(helper.SetIcon(mime, helper.Bitmap1()) == B_OK);
		CHK(helper.GetIcon(mime, bmp) == B_OK);
		CHK(*bmp == *helper.Bitmap1());
		CHK(helper.SetIcon(mime, &testBmp) != B_OK);	// R5 == B_BAD_VALUE
		CHK(helper.GetIcon(mime, bmp) == B_OK);
		CHK(*bmp == *helper.Bitmap1());
		CHK(*bmp != testBmp);		
		// Test Get()
		FillBitmap(testBmp, 3);
		CHK(helper.SetIcon(mime, helper.Bitmap1()) == B_OK);
		CHK(helper.GetIcon(mime, &testBmp) != B_OK);	// R5 == B_BAD_VALUE
	}
	// Invalid Bitmap Size (large -- 100x100)
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		CHK(mime.IsInstalled());
		// Init Test Bitmap
		BBitmap testBmp(BRect(0,0,99,99), B_CMAP8);
		// Test Set()
		FillBitmap(testBmp, 3);
		CHK(testBmp != *helper.Bitmap1());
		CHK(helper.SetIcon(mime, helper.Bitmap1()) == B_OK);
		CHK(helper.GetIcon(mime, bmp) == B_OK);
		CHK(*bmp == *helper.Bitmap1());
		CHK(helper.SetIcon(mime, &testBmp) != B_OK);	// R5 == B_BAD_VALUE
		CHK(helper.GetIcon(mime, bmp) == B_OK);
		CHK(*bmp == *helper.Bitmap1());
		CHK(*bmp != testBmp);
		// Test Get()
		FillBitmap(testBmp, 3);
		CHK(helper.SetIcon(mime, helper.Bitmap1()) == B_OK);
		CHK(helper.GetIcon(mime, &testBmp) != B_OK);	// R5 == B_BAD_VALUE
	}	
	// Invalid Bitmap Color Depth
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		CHK(mime.IsInstalled());
		// Init Test Bitmap
		BBitmap testBmp(helper.BitmapBounds(), B_RGBA32);
		// Test Set()
#if !SK_TEST_R5
		FillBitmap(testBmp, 4);
		CHK(testBmp != *helper.Bitmap1());
		CHK(RES(helper.SetIcon(mime, helper.Bitmap1())) == B_OK);
		CHK(RES(helper.GetIcon(mime, bmp)) == B_OK);
		CHK(*bmp == *helper.Bitmap1());
		CHK(RES(helper.SetIcon(mime, &testBmp)) == B_OK);
			// R5 == B_OK, despite being invalid color depth; however, icon is not actually set,
			// and any subsequent call to GetIcon() will cause the application to crash...
		CHK(RES(helper.GetIcon(mime, bmp)) == B_OK);	// R5 == CRASH!
		CHK(*bmp == *helper.Bitmap1());
		CHK(*bmp != testBmp);
#endif
		// Test Get()
#if SK_TEST_R5
		FillBitmap(testBmp, 3);
		CHK(helper.SetIcon(mime, helper.Bitmap1()) == B_OK);
		CHK(helper.GetIcon(mime, &testBmp) == B_OK);	// R5 == B_OK, but testBmp is not actually modified
		CHK(testBmp != *helper.Bitmap1());
#endif
	}	
	// Normal Function
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		CHK(mime.IsInstalled());
		// Set() then Get()
		FillBitmap(*bmp, 3);
		CHK(*bmp != *helper.Bitmap1());
		CHK(helper.SetIcon(mime, helper.Bitmap1()) == B_OK);
		CHK(helper.GetIcon(mime, bmp) == B_OK);
		CHK(*bmp == *helper.Bitmap1());
		// Set() then Get() again
		FillBitmap(*bmp, 3);
		CHK(helper.SetIcon(mime, helper.Bitmap2()) == B_OK);
		CHK(helper.GetIcon(mime, bmp) == B_OK);
		CHK(*bmp == *helper.Bitmap2());
		CHK(*bmp != *helper.Bitmap1());		
	}	
}

// Icon For Type Helper Functions

void MimeTypeTest::IconForTypeTest(IconForTypeHelper &helper) {
	IconTest(helper);	// First run all the icon tests	
		// Then do some IconForType() specific tests
		
	BBitmap *bmp = helper.TempBitmap();
		
	// Invalid MIME string
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		CHK(mime.IsInstalled());
		// Set() then Get()
		FillBitmap(*bmp, 3);
		CHK(*bmp != *helper.Bitmap1());
		CHK(mime.SetIconForType(testTypeInvalid, helper.Bitmap1(), helper.Size()) != B_OK);	// R5 == B_BAD_VALUE
		CHK(mime.GetIconForType(testTypeInvalid, bmp, helper.Size()) != B_OK);				// R5 == B_BAD_VALUE
		CHK(*bmp != *helper.Bitmap1());
	}
	// NULL MIME string (just like calling respective {Get,Set}Icon() function)
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		CHK(mime.IsInstalled());
		// Set() then Get()
		FillBitmap(*bmp, 3);
		CHK(*bmp != *helper.Bitmap1());
		CHK(mime.SetIconForType(NULL, helper.Bitmap1(), helper.Size()) == B_OK);
		CHK(mime.GetIconForType(NULL, bmp, helper.Size()) == B_OK);
		CHK(*bmp == *helper.Bitmap1());
		// Verify GetIcon() does the same thing
		FillBitmap(*bmp, 3);
		CHK(*bmp != *helper.Bitmap1());
		CHK(mime.GetIcon(bmp, helper.Size()) == B_OK);
		CHK(*bmp == *helper.Bitmap1());
	}	
}

void MimeTypeTest::LargeIconTest() {
	IconHelper helper(B_LARGE_ICON);
	IconTest(helper);
}

void MimeTypeTest::MiniIconTest() {
	IconHelper helper(B_MINI_ICON);
	IconTest(helper);
}

void MimeTypeTest::LargeIconForTypeTest() {
	IconForTypeHelper helper(testType, B_LARGE_ICON);
	IconForTypeTest(helper);
}

void MimeTypeTest::MiniIconForTypeTest() {
	IconForTypeHelper helper(testType, B_MINI_ICON);
	IconForTypeTest(helper);
}


// Short Description

void
MimeTypeTest::ShortDescriptionTest() {
	DescriptionTest(&BMimeType::GetShortDescription, &BMimeType::SetShortDescription);
}

// Long Description

void
MimeTypeTest::LongDescriptionTest() {
	DescriptionTest(&BMimeType::GetLongDescription, &BMimeType::SetLongDescription);
}

// DescriptionTest Helper Function
void
MimeTypeTest::DescriptionTest(GetDescriptionFunc getDescr, SetDescriptionFunc setDescr) {
	char str[B_MIME_TYPE_LENGTH+1];

	// Uninitialized
	nextSubTest();
	{
		BMimeType mime;
		CPPUNIT_ASSERT(mime.InitCheck() == B_NO_INIT);
		CPPUNIT_ASSERT((mime.*getDescr)(str) != B_OK);	// R5 == B_BAD_VALUE
		CPPUNIT_ASSERT((mime.*setDescr)(str) != B_OK);	// R5 == B_BAD_VALUE
	}	
	// Non-installed type
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Make sure the type isn't installed
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		CHK(!mime.IsInstalled());
		CHK((mime.*getDescr)(str) != B_OK);		// R5 == B_ENTRY_NOT_FOUND
		CHK(!mime.IsInstalled());
		CHK((mime.*setDescr)(testDescr) == B_OK);	// R5 == Installs (but doesn't set), B_OK
		CHK(mime.IsInstalled());
		CHK((mime.*getDescr)(str) == B_OK);
		CHK(strcmp(str, testDescr) == 0);
	}
	// Non-installed type, NULL params
	nextSubTest();
	{
#if !SK_TEST_R5	// NOTE: These tests crash for R5::LongDescription calls but not for R5::ShortDescription
				// calls. Considering the general instability exihibited by most R5 calls when passed
				// NULL pointers, however, I wouldn't suggest it, and thus they aren't even tested here.
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Make sure the type isn't installed
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		CHK(!mime.IsInstalled());
		CHK((mime.*getDescr)(NULL) != B_OK);		// R5 == B_BAD_ADDRESS
		CHK(!mime.IsInstalled());
		CHK((mime.*setDescr)(NULL) != B_OK);		// R5 == Installs (but doesn't set), B_ENTRY_NOT_FOUND
			// Note that a subsequent uninstall followed by a SetShortDescription() call with a
			// valid description succeeds and installs, but DOES NOT actually set.
			// I don't know why, but I'd just suggest not passing NULLs to the
			// R5 versions.
		CHK(mime.IsInstalled());
		CHK((mime.*getDescr)(str) == B_ENTRY_NOT_FOUND);
#endif
	}
	// Installed type
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		// Get() with no description installed
		CHK(mime.IsInstalled());
		CHK((mime.*getDescr)(str) == B_ENTRY_NOT_FOUND);	// R5 == B_ENTRY_NOT_FOUND
		// Initial Set()/Get()
		CHK((mime.*setDescr)(testDescr) == B_OK);
		CHK((mime.*getDescr)(str) == B_OK);
		CHK(strcmp(str, testDescr) == 0);
		// Followup Set()/Get()
		CHK((mime.*setDescr)(testDescr2) == B_OK);
		CHK((mime.*getDescr)(str) == B_OK);
		CHK(strcmp(str, testDescr2) == 0);		
	}
	// Installed Type, Description Too Long
	nextSubTest();
	{
		CHK(strlen(longDescr) > (B_MIME_TYPE_LENGTH+1));
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		// Initial Set()/Get()
		CHK((mime.*setDescr)(longDescr) != B_OK);		// R5 == B_BAD_VALUE
		CHK((mime.*getDescr)(str) == B_ENTRY_NOT_FOUND);
		// Followup Set()/Get()
		CHK((mime.*setDescr)(testDescr) == B_OK);
		CHK((mime.*setDescr)(longDescr) != B_OK);		// R5 == B_BAD_VALUE
		CHK((mime.*getDescr)(str) == B_OK);
		CHK(strcmp(str, testDescr) == 0);		
	}

}


// Preferred App

void
MimeTypeTest::PreferredAppTest() {
	char str[B_MIME_TYPE_LENGTH+1];
	sprintf(str, "%s", testSig);

	// Uninitialized
	nextSubTest();
	{
		BMimeType mime;
		CPPUNIT_ASSERT(mime.InitCheck() == B_NO_INIT);
		CPPUNIT_ASSERT(mime.GetPreferredApp(str) != B_OK);	// R5 == B_BAD_VALUE
		CPPUNIT_ASSERT(mime.SetPreferredApp(str) != B_OK);	// R5 == B_BAD_VALUE
	}	
	// Non-installed type
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Make sure the type isn't installed
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		CHK(!mime.IsInstalled());
		CHK(mime.GetPreferredApp(str) != B_OK);		// R5 == B_ENTRY_NOT_FOUND
		CHK(!mime.IsInstalled());
		CHK(mime.SetPreferredApp(testSig) == B_OK);	// R5 == Installs (but doesn't set), B_OK
		CHK(mime.IsInstalled());
		CHK(mime.GetPreferredApp(str) == B_OK);
		CHK(strcmp(str, testSig) == 0);
	}
	// Non-installed type, NULL params
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Make sure the type isn't installed
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		CHK(!mime.IsInstalled());
		CHK(mime.GetPreferredApp(NULL) != B_OK);		// R5 == B_ENTRY_NOT_FOUND
		CHK(!mime.IsInstalled());
		CHK(mime.SetPreferredApp(NULL) != B_OK);		// R5 == Installs (but doesn't set), B_ENTRY_NOT_FOUND
		CHK(mime.IsInstalled());
		CHK(mime.GetPreferredApp(str) == B_ENTRY_NOT_FOUND);
	}
	// Installed type, NULL params
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		CHK(mime.IsInstalled());
		CHK(mime.GetPreferredApp(NULL) != B_OK);		// R5 == B_BAD_ADDRESS
		CHK(mime.SetPreferredApp(NULL) != B_OK);		// R5 == B_ENTRY_NOT_FOUND
		CHK(mime.GetPreferredApp(NULL) != B_OK);		// R5 == B_BAD_ADDRESS
	}
	// Installed type
	nextSubTest();
	{
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		// Get() with no description installed
		CHK(mime.IsInstalled());
		CHK(mime.GetPreferredApp(str) == B_ENTRY_NOT_FOUND);	// R5 == B_ENTRY_NOT_FOUND
		// Initial Set()/Get()
		CHK(mime.SetPreferredApp(testSig) == B_OK);
		CHK(mime.GetPreferredApp(str) == B_OK);
		CHK(strcmp(str, testSig) == 0);
		// Followup Set()/Get()
		CHK(mime.SetPreferredApp(testSig2) == B_OK);
		CHK(mime.GetPreferredApp(str) == B_OK);
		CHK(strcmp(str, testSig2) == 0);		
	}
	// Installed Type, Signature Too Long
	nextSubTest();
	{
		CHK(strlen(longDescr) > (B_MIME_TYPE_LENGTH+1));
		BMimeType mime(testType);
		CHK(mime.InitCheck() == B_OK);
		// Uninstall then reinstall to clear attributes
		if (mime.IsInstalled())
			CHK(mime.Delete() == B_OK);
		if (!mime.IsInstalled())
			CHK(mime.Install() == B_OK);
		// Initial Set()/Get()
		CHK(mime.SetPreferredApp(longSig) != B_OK);		// R5 == B_BAD_VALUE
		CHK(mime.GetPreferredApp(str) == B_ENTRY_NOT_FOUND);
		// Followup Set()/Get()
		CHK(mime.SetPreferredApp(testSig) == B_OK);
		CHK(mime.SetPreferredApp(longSig) != B_OK);		// R5 == B_BAD_VALUE
		CHK(mime.GetPreferredApp(str) == B_OK);
		CHK(strcmp(str, testSig) == 0);		
	}

}

// init_long_types
static
void
init_long_types(char *notTooLongType, char *tooLongType)
{
	const int notTooLongLength = B_MIME_TYPE_LENGTH;
	const int tooLongLength = notTooLongLength + 1;
	strcpy(notTooLongType, "image/");
	memset(notTooLongType + strlen(notTooLongType), 'a',
		   notTooLongLength - strlen(notTooLongType));
	notTooLongType[notTooLongLength] = '\0';
	strcpy(tooLongType, "image/");
	memset(tooLongType + strlen(tooLongType), 'a',
		   tooLongLength - strlen(tooLongType));
	tooLongType[tooLongLength] = '\0';
}

// InitTest
void
MimeTypeTest::InitTest()
{
	// tests:
	// * constructors
	// * SetTo(), SetType()
	// * Unset()
	// * InitCheck()
	// (* Type())

	// We test only a few types here. Exhausting testing is done in
	// ValidityTest().
	const char *validType	= "image/gif";
	const char *validType2	= "application/octet-stream";
	const char *invalidType	= "invalid type";
	char notTooLongType[B_MIME_TYPE_LENGTH + 3];
	char tooLongType[B_MIME_TYPE_LENGTH + 3];
	init_long_types(notTooLongType, tooLongType);

	// default constructor
	nextSubTest();
	{
		BMimeType type;
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}

	// BMimeType(const char *)
	// valid type
	nextSubTest();
	{
		BMimeType type(validType);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == validType);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// invalid type
	nextSubTest();
	{
		BMimeType type(invalidType);
		CHK(type.InitCheck() == B_BAD_VALUE);
		CHK(type.Type() == NULL);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// long, but not too long type
	nextSubTest();
	{
		BMimeType type(notTooLongType);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == notTooLongType);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// too long type
	nextSubTest();
	{
		BMimeType type(tooLongType);
		CHK(type.InitCheck() == B_BAD_VALUE);
		CHK(type.Type() == NULL);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	
	// SetTo()
	// valid type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetTo(validType) == B_OK);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == validType);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// invalid type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetTo(invalidType) == B_BAD_VALUE);
		CHK(type.InitCheck() == B_BAD_VALUE);
		CHK(type.Type() == NULL);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// long, but not too long type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetTo(notTooLongType) == B_OK);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == notTooLongType);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// too long type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetTo(tooLongType) == B_BAD_VALUE);
		CHK(type.InitCheck() == B_BAD_VALUE);
		CHK(type.Type() == NULL);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}

	// SetType()
	// valid type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetType(validType) == B_OK);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == validType);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// invalid type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetType(invalidType) == B_BAD_VALUE);
		CHK(type.InitCheck() == B_BAD_VALUE);
		CHK(type.Type() == NULL);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// long, but not too long type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetType(notTooLongType) == B_OK);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == notTooLongType);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	// too long type
	nextSubTest();
	{
		BMimeType type;
		CHK(type.SetType(tooLongType) == B_BAD_VALUE);
		CHK(type.InitCheck() == B_BAD_VALUE);
		CHK(type.Type() == NULL);
		type.Unset();
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
	
	// reinitialization
	nextSubTest();
	{
		BMimeType type(validType);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == validType);
		CHK(type.SetTo(validType2) == B_OK);
		CHK(type.InitCheck() == B_OK);
		CHK(string(type.Type()) == validType2);
	}
	// bad args
	nextSubTest();
	{
		BMimeType type(NULL);
		CHK(type.Type() == NULL);
		CHK(type.InitCheck() == B_NO_INIT);
		CHK(type.Type() == NULL);
		CHK(type.SetTo(NULL) == B_NO_INIT);
		CHK(type.Type() == NULL);
		CHK(type.SetType(NULL) == B_NO_INIT);
		CHK(type.Type() == NULL);
	}
}

// StringTest
void
MimeTypeTest::StringTest()
{
	// tests:
	// * IsValid() (static/non static)
	// * Type()
	// * IsSupertypeOnly()
	// * GetSupertype()
	// * Contains()
	// * operator==()

	char notTooLongType[B_MIME_TYPE_LENGTH + 3];
	char tooLongType[B_MIME_TYPE_LENGTH + 3];
	init_long_types(notTooLongType, tooLongType);
	struct mime_type_test {
		const char	*type;
		bool		super_type;
		status_t	error;
	};
	mime_type_test tests[] = {
		// valid types
		{ "application",					true,	B_OK, },
		{ "application/octet-stream",		false,	B_OK, },
		{ "audio",							true,	B_OK, },
		{ "audio/x-aiff",					false,	B_OK, },
		{ "image",							true,	B_OK, },
		{ "image/gif",						false,	B_OK, },
		{ "message",						true,	B_OK, },
		{ "message/rfc822",					false,	B_OK, },
		{ "multipart",						true,	B_OK, },
		{ "multipart/mixed",				false,	B_OK, },
		{ "text",							true,	B_OK, },
		{ "text/plain",						false,	B_OK, },
		{ "video",							true,	B_OK, },
		{ "video/x-msvideo",				false,	B_OK, },
		{ "unknown",						true,	B_OK, },
		{ "unknown/mime-type",				false,	B_OK, },
		{ "$%&./`'~*+#|!^",					false,	B_OK, },
		// invalid types
		{ "",								false,	B_BAD_VALUE, },
		{ "application/",					false,	B_BAD_VALUE, },
		{ "audio/",							false,	B_BAD_VALUE, },
		{ "image/",							false,	B_BAD_VALUE, },
		{ "message/",						false,	B_BAD_VALUE, },
		{ "multipart/",						false,	B_BAD_VALUE, },
		{ "text/",							false,	B_BAD_VALUE, },
		{ "video/",							false,	B_BAD_VALUE, },
		{ "unknown/",						false,	B_BAD_VALUE, },
		{ "/gif",							false,	B_BAD_VALUE, },
		{ "image/very/nice",				false,	B_BAD_VALUE, },
		{ "tex t/plain",					false,	B_BAD_VALUE, },
		{ "text/pla in",					false,	B_BAD_VALUE, },
		{ "tex\tt/plain",					false,	B_BAD_VALUE, },
		{ "text/pla\tin",					false,	B_BAD_VALUE, },
		{ "tex\nt/plain",					false,	B_BAD_VALUE, },
		{ "text/pla\nin",					false,	B_BAD_VALUE, },
		{ "tex<t/plain",					false,	B_BAD_VALUE, },
		{ "text/pla<in",					false,	B_BAD_VALUE, },
		{ "tex>t/plain",					false,	B_BAD_VALUE, },
		{ "text/pla>in",					false,	B_BAD_VALUE, },
		{ "tex@t/plain",					false,	B_BAD_VALUE, },
		{ "text/pla@in",					false,	B_BAD_VALUE, },
		{ "tex,t/plain",					false,	B_BAD_VALUE, },
		{ "text/pla,in",					false,	B_BAD_VALUE, },
		{ "tex;t/plain",					false,	B_BAD_VALUE, },
		{ "text/pla;in",					false,	B_BAD_VALUE, },
		{ "tex:t/plain",					false,	B_BAD_VALUE, },
		{ "text/pla:in",					false,	B_BAD_VALUE, },
		{ "tex\"t/plain",					false,	B_BAD_VALUE, },
		{ "text/pla\"in",					false,	B_BAD_VALUE, },
		{ "tex(t/plain",					false,	B_BAD_VALUE, },
		{ "text/pla(in",					false,	B_BAD_VALUE, },
		{ "tex)t/plain",					false,	B_BAD_VALUE, },
		{ "text/pla)in",					false,	B_BAD_VALUE, },
		{ "tex[t/plain",					false,	B_BAD_VALUE, },
		{ "text/pla[in",					false,	B_BAD_VALUE, },
		{ "tex]t/pla]in",					false,	B_BAD_VALUE, },
		{ "tex?t/plain",					false,	B_BAD_VALUE, },
		{ "text/pla?in",					false,	B_BAD_VALUE, },
		{ "tex=t/plain",					false,	B_BAD_VALUE, },
		{ "text/pla=in",					false,	B_BAD_VALUE, },
		{ "tex\\t/plain",					false,	B_BAD_VALUE, },
		{ "text/pla\\in",					false,	B_BAD_VALUE, },
		// (not) too long types
		{ notTooLongType,					false,	B_OK, },
		{ tooLongType,						false,	B_BAD_VALUE, },
	};
	int32 testCount = sizeof(tests) / sizeof(mime_type_test);
	// test loop
	for (int32 i = 0; i < testCount; i++) {
		nextSubTest();
		mime_type_test &test = tests[i];
		BMimeType type(test.type);
		CHK(type.InitCheck() == test.error);
		bool valid = (test.error == B_OK);
		bool validSuper = (valid && test.super_type);
		// Type()
		if (valid)
			CHK(string(type.Type()) == test.type);
		else
			CHK(type.Type() == NULL);
		// IsValid(), IsSuperTypeOnly()
		CHK(type.IsValid() == valid);
		CHK(type.IsSupertypeOnly() == validSuper);
		CHK(BMimeType::IsValid(test.type) == valid);
		// GetSupertype()
		if (valid && !validSuper) {
			BMimeType super;
			CHK(type.GetSupertype(&super) == B_OK);
			CHK(super.InitCheck() == B_OK);
			CHK(super.Contains(&type) == true);
			BString typeString(test.type);
			BString superString(typeString.String(),
								typeString.FindFirst('/'));
			CHK(superString == super.Type());
		} else {
			BMimeType super;
			CHK(type.GetSupertype(&super) == B_BAD_VALUE);
		}
		// Contains(), ==
		for (int32 k = 0; k < testCount; k++) {
			mime_type_test &test2 = tests[k];
			BMimeType type2(test2.type);
			CHK(type2.InitCheck() == test2.error);
			bool valid2 = (test2.error == B_OK);
			bool validSuper2 = (valid && test2.super_type);
			bool equal = (!strcmp(test.type, test2.type));
			// ==
			if (valid || valid2) {
				CHK((type == type2) == equal);
				CHK((type == test2.type) == equal);
			} else {
				CHK((type == type2) == false);
				CHK((type == test2.type) == false);
			}
			// Contains()
			if (valid || valid2) {
				if (equal)
					CHK(type.Contains(&type2) == true);
				else if (validSuper && valid2 && !validSuper2) {
					BMimeType super2;
					CHK(type2.GetSupertype(&super2) == B_OK);
					bool contains = string(super2.Type()) == type.Type();
					CHK(type.Contains(&type2) == contains);
				} else
					CHK(type.Contains(&type2) == false);
			} else
				CHK(type.Contains(&type2) == false);
		}
	}
	// bad args
	nextSubTest();
	{
		BMimeType type("image/gif");
// R5: crashes when passing NULL
#if !SK_TEST_R5
		CHK(BMimeType::IsValid(NULL) == false);
		CHK(type.GetSupertype(NULL) == B_BAD_VALUE);
		CHK(type.Contains(NULL) == false);
#endif
		CHK((type == NULL) == false);
	}
}


/* Ingo's functions:

	// initialization
+	BMimeType();
+	BMimeType(const char *mimeType);
(	virtual ~BMimeType();)

+	status_t SetTo(const char *mimeType);
+	status_t SetType(const char *mimeType);
+	void Unset();
+	status_t InitCheck() const;

	// string access
+	const char *Type() const;
+	bool IsValid() const;
+	static bool IsValid(const char *mimeType);
+	bool IsSupertypeOnly() const;
+	status_t GetSupertype(BMimeType *superType) const;
+	bool Contains(const BMimeType *type) const;
+	bool operator==(const BMimeType &type) const;
+	bool operator==(const char *type) const;

	// MIME database monitoring
	static status_t StartWatching(BMessenger target);
	static status_t StopWatching(BMessenger target);

	// C functions
	int update_mime_info(const char *path, int recursive, int synchronous,
						 int force);
	status_t create_app_meta_mime(const char *path, int recursive,
								  int synchronous, int force);
	status_t get_device_icon(const char *dev, void *icon, int32 size);
*/


/* Tyler's functions:

	// MIME database access
	status_t Install();
	status_t Delete();
+	status_t GetIcon(BBitmap *icon, icon_size size) const;
+	status_t GetPreferredApp(char *signature, app_verb verb = B_OPEN) const;
	status_t GetAttrInfo(BMessage *info) const;
+	status_t GetFileExtensions(BMessage *extensions) const;
+	status_t GetShortDescription(char *description) const;
+	status_t GetLongDescription(char *description) const;
	status_t GetSupportingApps(BMessage *signatures) const;

+	status_t SetIcon(const BBitmap *icon, icon_size size);
+	status_t SetPreferredApp(const char *signature, app_verb verb = B_OPEN);
	status_t SetAttrInfo(const BMessage *info);
	status_t SetFileExtensions(const BMessage *extensions);
+	status_t SetShortDescription(const char *description);
+	status_t SetLongDescription(const char *description);

	static status_t GetInstalledSupertypes(BMessage *super_types);
	static status_t GetInstalledTypes(BMessage *types);
	static status_t GetInstalledTypes(const char *super_type,
									  BMessage *subtypes);
	static status_t GetWildcardApps(BMessage *wild_ones);

+	status_t GetAppHint(entry_ref *ref) const;
+	status_t SetAppHint(const entry_ref *ref);

+	status_t GetIconForType(const char *type, BBitmap *icon,
							icon_size which) const;
+	status_t SetIconForType(const char *type, const BBitmap *icon,
							icon_size which);
*/


/* unassigned functions:

	// sniffer rule manipulation
	status_t GetSnifferRule(BString *result) const;
	status_t SetSnifferRule(const char *);
	static status_t CheckSnifferRule(const char *rule, BString *parseError);

	// sniffing
	status_t GuessMimeType(const entry_ref *file, BMimeType *result);
	static status_t GuessMimeType(const void *buffer, int32 length,
								  BMimeType *result);
	static status_t GuessMimeType(const char *filename, BMimeType *result);
*/
