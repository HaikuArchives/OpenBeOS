#include "SKError.h"
#include <string.h>
#include <stdio.h>

SKError::SKError(int error, const char* errorMessage)
	: fError(error)
{
	InitErrorMessage(errorMessage);
	if (DO_DEBUG)
		printf("SKError: %s\n", errorMessage);	
}

// Initializes the error message; errorMessage is 
// assumed to be non-null
void SKError::InitErrorMessage(const char* errorMessage)
{
	int len = strlen(errorMessage);
	fErrorMessage = new char[len + 1];
		// TODO: We need to do something intelligent here if
		// the allocation fails, possibly have a static
		// "Insufficent Memory" exception (not derived from
		// SKError) that we could throw.
	strcpy( fErrorMessage, errorMessage );
}

SKError::~SKError()
{
	delete fErrorMessage;
}

const int SKError::Error() const
{
	return fError;
}

const char* SKError::ErrorMessage() const
{
	return fErrorMessage;
}

