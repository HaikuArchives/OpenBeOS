//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: EntryRefToPath.cpp
//  Description:  Converts an entry ref (a device id, a directory inode
//  			  number, and a filename) into an absolute pathname.
//
//  Usage: EntryRefToPath DevID Dir_INode Filename
//
//  Output: If successful, prints the complete pathname to standard out
//          If not successful, prints a status_t error code in ASCII
//			followed by a newline and instructions on the program
//			usage.
//---------------------------------------------------------------------

#include <iostream>
#include <Be.h>

void PrintUsage() {
	cout << endl;
	cout << "Usage: EntryRefToPath DevID Dir_INode Filename" << endl;
	cout << endl;
	cout << "Output: If successful, prints the complete pathname to standard out." << endl;
	cout << "        If not successful, prints out a status_t error code in ASCII" << endl;
	cout << "        followed by a newline and this message." << endl;
	cout << endl;
}

int main(int argc, char *argv[]) {
	const int B_INVALID_ARGS = -1234;

	// Vars
	dev_t device;
	ino_t dir;
	char *name = NULL;

	try {
	
		// Decode the given arguments
		if (argc != 4) 
			throw (status_t)B_INVALID_ARGS;
	
		// First arg is the device id
		if (sscanf(argv[1], "%ld", &device) != 1)
			throw (status_t)B_INVALID_ARGS;

		// Second arg is the 64-bit directory inode
		if (sscanf(argv[2], "%lld", &dir) != 1)
			throw (status_t)B_INVALID_ARGS;

		// Third arg is the name
		name = new char[strlen(argv[3])];
		if (name == NULL)
			throw (status_t)B_NO_MEMORY;
		strcpy(name, argv[3]);		

//		printf("device = %ld \n", device);
//		printf("dir    = %lld \n", dir);
//		printf("name   = '%s' \n", name);
	
//		for (int i = 0; i < argc; i++) {
//			cout << i << " == '" << argv[i] << "'" << endl;
//		}

		// Convert the entry_ref to a pathname		
		entry_ref ref(device, dir, name);
		BEntry entry(&ref);
		if (entry.InitCheck() != B_OK)
			throw (status_t)entry.InitCheck();
		BPath path(&entry);
		if (path.InitCheck() != B_OK)
			throw (status_t)path.InitCheck();

		// Print out the pathname
		cout << path.Path() << flush;
		
	} catch (status_t i) {
		cout << i << endl;
		if (i == B_INVALID_ARGS)
			PrintUsage();
	}

	delete [] name;
	return 0;
}


