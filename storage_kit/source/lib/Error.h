//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Error.h
//  Description: Base error class thrown by all storage kit kernel
//  interface functions.
//----------------------------------------------------------------------
#ifndef _sk_error_h_
#define _sk_error_h_

#include <sys/types.h>

namespace StorageKit {

//! Basic Storage Kit exception class
/*! Implements the basic functionality of the exception class used internally
	by the Storage Kit. */
class Error {
public:
	//! Creates an Error object with the given error code and message.
	/*! Creates an Error object with the given error code and message. This general
		constructor is provided to handle cases where unexpected error codes are
		encountered. For expected errors, creating a corresponding subclass of Error
		(i.e. EInsufficientMemory) is recommended. */
	Error(const int errorCode, const char* errorMessage = NULL);

	//! Destructor.
	/*! Frees the object's error message string, and then destroys the object. */
	virtual ~Error();

	//! Returns the object's error code.
	/*! Returns the object's error code. */
	const int ErrorCode() const;

	//! Returns the object's error message.
	/*! Returns the object's error message. */
	const char* ErrorMessage() const;

protected:
	//! Prints one or more lines of debugging information to standard output.
	/*! Prints one or more lines of debugging information to standard output. The default
		implementation prints the object's base class name and the object's error message.
		Automatically called in the object's constructor if	kDebug is true. */
	virtual void PrintDebugInfo() const;
	
	//! Class-wide flag to enable or disable debugging messages to standard output.
	/*! Class-wide flag to enable or disable debugging messages to standard output. If
		true, a message is printed each time an Error object is constructed specifying
		the object's type and its associated error message. */
	static const bool kDebug = true;
	
	//! Default error message.
	/*! Default error message. Returned by ErrorMessage() if the error message specified
		to the object's constructor is NULL. */
	static const char kDefaultErrorMessage[] = "Undefined Storage Kit Error";

private:
	//! Copy constructor.
	/*! Copy constructor. For the sake of efficiency, this constructor is private to prevent the use of
		pass by value and pass by reference exceptions. */	
	Error(const Error &ref);

	//! Default constructor.
	/*! Default constructor. This constructor is private to prevent creation without
		the inclusion of an error code and message. */
	Error();

	//! Sets the error message for this Error object.
	/*! Sets the error message for this Error object. Allocates the necessary resources
		for the given the string (freeing any previous resources, if they exist), and
		then makes a copy of it. If errorMessage is NULL, then kDefaultErrorMessage is
		used. */
	void SetErrorMessage(const char* errorMessage);

	//! The error code for this object.
	/*! The error code for this object. */
	int fErrorCode;

	//! Pointer to the error message for this object.
	/*! Pointer to the error message for this object. */
	char *fErrorMessage;		
		
};	// class Error

};	// namespace StorageKit

#endif

