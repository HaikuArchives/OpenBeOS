//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Error.cpp
//  Description: Base error class thrown by all storage kit kernel
//  interface functions.
//----------------------------------------------------------------------
#include "Error.h"
#include <string.h>
#include <stdio.h>

StorageKit::Error::Error(int errorCode, const char* errorMessage)
: fErrorCode(errorCode), fErrorMessage(NULL) {
	SetErrorMessage(errorMessage);
	if (DEBUG)
		PrintDebugInfo();
}

void
StorageKit::Error::SetErrorMessage(const char* errorMessage) {
	if (fErrorMessage != NULL)			// Check for previous message
		delete fErrorMessage;			// Free it if necessary

	char str[1024];
	if (errorMessage == NULL) {
		sprintf(str, "%s -- %d", kDefaultErrorMessage, fErrorCode);
		errorMessage = str;
	}
	
	// Allocate room for and then copy the given string
	int len = strlen(errorMessage);	
	fErrorMessage = new char[len + 1];
		/*! /todo We need to do something intelligent here if
			the allocation fails, possibly have a static
			"Insufficent Memory" exception (not derived from
			SKError) that we could throw. */
	strcpy( fErrorMessage, errorMessage );
}

StorageKit::Error::~Error() {
	delete fErrorMessage;
}

const int
StorageKit::Error::ErrorCode() const {
	return fErrorCode;
}

const char*
StorageKit::Error::ErrorMessage() const {
	return (fErrorMessage != NULL) ? fErrorMessage : kDefaultErrorMessage;
}


void
StorageKit::Error::PrintDebugInfo() const {
	/*! /todo It'd be nice to be able to get the name of the call
		at run time. I'm not sure if this is something RTTI will do
		for us or not. I'll just have to look it up when I get the
		chance. */
	printf("StorageKit::Error -- %s\n", this->ErrorMessage());	
}