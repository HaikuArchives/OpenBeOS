//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: File.h
//  Description: A wrapper class for common operations on files.
//---------------------------------------------------------------------
#ifndef __sk_file_h__
#define __sk_file_h__

#include <DataIO.h>
#include <Node.h>

#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif

/*!
	\class BFile
	\brief BFile is a wrapper class for common operations on files providing
	access to the file's content data and its attributes.
	
	A BFile represents a file in some file system. It implements the
	BPositionIO interface and thus the methods to read from and write to the
	file, and is derived of BNode to provide access to the file's attributes.

	\author <a href='mailto:bonefish@users.sf.net'>Ingo Weinhold</a>
	
	\version 0.0.0
*/
class BFile : public BNode, public BPositionIO {
public:
	//! Creates an uninitialized BFile.
	BFile();

	//! Creates a copy of the supplied BFile.
	BFile(const BFile &file);

	/*! \brief Creates a BFile and initializes it to the file referred to by
		the supplied entry_ref and according to the specified open mode.
	*/
	BFile(const entry_ref *ref, uint32 openMode);

	/*! \brief Creates a BFile and initializes it to the file referred to by
		the supplied BEntry and according to the specified open mode.
	*/
	BFile(const BEntry *entry, uint32 openMode);

	/*! \brief Creates a BFile and initializes it to the file referred to by
		the supplied path name and according to the specified open mode.
	*/
	BFile(const char *path, uint32 openMode);

	/*! \brief Creates a BFile and initializes it to the file referred to by
		the supplied path name relative to the specified BDirectory and
		according to the specified open mode.
	*/
	BFile(BDirectory *dir, const char *path, uint32 openMode);

	//! Frees all allocated resources.
	virtual ~BFile();

	/*! \brief Re-initializes the BFile to the file referred to by the
		supplied entry_ref and according to the specified open mode. */
	status_t SetTo(const entry_ref *ref, uint32 openMode);

	/*! \brief Re-initializes the BFile to the file referred to by the
		supplied BEntry and according to the specified open mode. */
	status_t SetTo(const BEntry *entry, uint32 openMode);

	/*! \brief Re-initializes the BFile to the file referred to by the
		supplied path name and according to the specified open mode. */
	status_t SetTo(const char *path, uint32 openMode);

	/*! \brief Re-initializes the BFile to the file referred to by the
		supplied path name relative to the specified BDirectory and according
		to the specified open mode. */
	status_t SetTo(const BDirectory *dir, const char *path, uint32 openMode);

	//! Uninitilizes the BFile.
//	void Unset();

	//! Returns whether the file is readable.
	bool IsReadable() const;

	//! Returns whether the file is writable.
	bool IsWritable() const;

	//! Reads a number of bytes from the file into a buffer.
	virtual ssize_t Read(void *buffer, size_t size);

	/*! \brief Reads a number of bytes from a certain position within the file
		into a buffer. */
	virtual ssize_t ReadAt(off_t location, void *buffer, size_t size);

	//! Writes a number of bytes from a buffer into the file.
	virtual ssize_t Write(const void *buffer, size_t size);

	/*! \brief Writes a number of bytes from a buffer at a certain position
		into the file. */
	virtual ssize_t WriteAt(off_t location, const void *buffer, size_t size);

	//! Seeks to another read/write position within the file.
	virtual off_t Seek(off_t offset, uint32 seekMode);

	//! Returns the current read/write position within the file.
	virtual off_t Position() const;

	//! Sets the size of the file.
	virtual status_t SetSize(off_t size);

//	virtual status_t GetSize(off_t *size) const;

	//! Assigns another BFile to this BFile.
	BFile &operator=(const BFile &file);

private:
	virtual void _ReservedFile1();
	virtual void _ReservedFile2();
	virtual void _ReservedFile3();
	virtual void _ReservedFile4();
	virtual void _ReservedFile5();
	virtual void _ReservedFile6();

	uint32 _reservedData[8];

private:
	//! Closes the file's file descriptor.
	virtual void close_fd();

	//! Returns the file descriptor.
	int get_fd() const;

	//! Sets the BNode's status.
	void set_status(status_t newStatus);

private:
	//! The file's open mode.
	uint32 fMode;
};

#ifdef USE_OPENBEOS_NAMESPACE
};		// namespace OpenBeOS
#endif

#endif	// __sk_file_h__
