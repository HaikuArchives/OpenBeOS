#include "Test.StorageKit.h"

// ##### Include your test headers here #####
#include "DirectoryTest.h"
#include "EntryTest.h"
#include "FileTest.h"
#include "NodeTest.h"
#include "PathTest.h"
#include "SymLinkTest.h"

StorageKitShell shell;

int main(int argc, char *argv[]) {
	// ##### Add your test suites here #####
	shell.AddSuite( "BDirectory", &DirectoryTest::Suite );
	shell.AddSuite( "BEntry", &EntryTest::Suite );
	shell.AddSuite( "BFile", &FileTest::Suite );
	shell.AddSuite( "BNode", &NodeTest::Suite );
	shell.AddSuite( "BPath", &PathTest::Suite );
	shell.AddSuite( "BSymLink", &SymLinkTest::Suite );

	return shell.Run(argc, argv);
}

StorageKitShell::StorageKitShell() : CppUnitShell() {
}

void StorageKitShell::PrintDescription(int argc, char *argv[]) {
	std::string AppName = argv[0];
	cout << endl;
	cout << "This program is the central testing framework for the purpose" << endl;
	cout << "of testing and verifying the behavior of the OpenBeOS Storage" << endl;
	cout << "Kit." << endl;
		
	if (AppName.rfind("Test.StorageKit.R5") != std::string::npos) {
		cout << endl;
		cout << "Judging by its name (Test.StorageKit.R5), this copy was" << endl;
		cout << "probably linked again Be Inc.'s R5 Storage Kit for the sake" << endl;
		cout << "of comparison against our own implementation." << endl;
	}
	else if (AppName.rfind("Test.StorageKit.OpenBeOS") != std::string::npos) {
		cout << endl;
		cout << "Judging by its name (Test.StorageKit.OpenBeOS), this copy" << endl;
		cout << "was probably linked against the OpenBeOS implementation of" << endl;
		cout << "the Storage Kit." << endl;
	}
}
	
