//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: storage_support.h
//  Description: This file contains the interface for miscellaneous
//  Storage Kit internal support functionality.
//---------------------------------------------------------------------

//! Private Storage Kit Namespace
/*! Private Storage Kit Namespace */
namespace StorageKit {

//! Returns whether the supplied path is absolute.
bool is_absolute_path(const char *path);

//!	\brief splits a path name into directory path and leaf name
bool split_path(const char *fullPath, char *&path, char *&leaf);

//!	\brief splits a path name into directory path and leaf name
bool split_path(const char *fullPath, char **path, char **leaf);

} // namespace StorageKit

