//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: FindDirectory.cpp
//---------------------------------------------------------------------
#include <FindDirectory.h>


#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

enum {
	NOT_IMPLEMENTED	= B_ERROR,
};

// find_directory
//!	Returns a path of a directory specified by a directory_which constant.
/*!	\param which the directory_which constant specifying the directory
	\param volume the volume on which the directory is located
	\param createIt \c true, if the directory shall be created, if it doesn't
		   already exist, \c false otherwise.
	\param pathString a pointer to a buffer into which the directory path
		   shall be written.
	\param length the size of the buffer
	\return \c B_OK if everything went fine, an error code otherwise.
	\todo Implement!
*/
status_t
find_directory(directory_which which, dev_t volume, bool createIt,
			   char *pathString, int32 length)
{
	return NOT_IMPLEMENTED;
}

// find_directory
//!	Returns a path of a directory specified by a directory_which constant.
/*!	\param which the directory_which constant specifying the directory
	\param path a BPath object to be initialized to the directory's path
	\param createIt \c true, if the directory shall be created, if it doesn't
		   already exist, \c false otherwise.
	\param volume the volume on which the directory is located
	\return \c B_OK if everything went fine, an error code otherwise.
	\todo Implement!
*/
status_t
find_directory(directory_which which, BPath *path, bool createIt,
			   BVolume *volume)
{
	return NOT_IMPLEMENTED;
}


#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif
