//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Test.Error.cpp
//  Description: Command-line test program for the StorageKit::Error
//  class.
//---------------------------------------------------------------------
#include "Error.h"
#include <stdio.h>

namespace StorageKit {

class TestError : public StorageKit::Error {
public:
	TestError(const char* errorMessage = NULL) : StorageKit::Error(99, errorMessage) { };
};

};


int main() {

	printf("\n");

	try	{
		throw new StorageKit::TestError();
		printf("This text will never see the light of day\n");	
	}
	catch (StorageKit::TestError *e) {
		printf("Caught StorageKit::TestError:\n");
		printf(" e->ErrorCode() == %d\n", e->ErrorCode());
		printf(" e->ErrorMessage() == %s\n", e->ErrorMessage());
		delete e;
	}
	
	printf("\n");

	try	{
		throw new StorageKit::TestError("This is a test!");
		printf("This text will never see the light of day\n");	
	}
	catch (StorageKit::Error *e) {
		printf("Caught StorageKit::Error:\n");
		printf(" e->ErrorCode() == %d\n", e->ErrorCode());
		printf(" e->ErrorMessage() == %s\n", e->ErrorMessage());
		delete e;
	}
	
	printf("\n");

	return 0;	

}