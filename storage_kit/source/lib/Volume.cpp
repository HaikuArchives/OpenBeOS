// ----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//
//  File Name: Directory.cpp
// ----------------------------------------------------------------------

#include <Volume.h>

#include <Directory.h>
#include <Bitmap.h>
#include <Node.h>
#include <fs_info.h>
#include <errno.h>
/*
#ifdef USE_OPENBEOS_NAMESPACE
namespace OpenBeOS {
#endif
*/
// ----------------------------------------------------------------------
//	BVolume (public)
// ----------------------------------------------------------------------
//	Default constructor: does nothing and sets InitCheck() to B_NO_INIT.

BVolume::BVolume(void)
{
	Unset();
}


// ----------------------------------------------------------------------
//	BVolume (public)
// ----------------------------------------------------------------------
//	Device constructor: sets the BVolume to point to the volume
//	represented by the argument. See the SetTo() function for
//	status codes.

BVolume::BVolume(
	dev_t			dev)
{
	SetTo(dev);
}


// ----------------------------------------------------------------------
//	BVolume (public)
// ----------------------------------------------------------------------
//	Copy constructor: sets the object to point to the same device as
//	does the argument.

BVolume::BVolume(
	const BVolume&	vol)
{
	fDev = vol.Device();
	fCStatus = vol.InitCheck();
}


// ----------------------------------------------------------------------
//	~BVolume (public, virtual)
// ----------------------------------------------------------------------
//	Destructor: Destroys the BVolume object.

BVolume::~BVolume(void)
{
}


// ----------------------------------------------------------------------
//	InitCheck (public)
// ----------------------------------------------------------------------
//	Returns the status of the last initialization (from either the
//	constructor or SetTo()). 

status_t
BVolume::InitCheck(void) const 
{	
	return fCStatus;
}


// ----------------------------------------------------------------------
//	SetTo (public)
// ----------------------------------------------------------------------
//	Initializes the BVolume object to represent the volume (device)
//	identified by the argument. 

status_t
BVolume::SetTo(
	dev_t			dev) 
{
	fDev = dev;
	
	// Call the kernel function that gets device information
	//	in order to determine the device status:
	fs_info			fsInfo;
	int				err = fs_stat_dev(dev, &fsInfo);
	
	if (err != 0) {
		fCStatus = errno;
	}
	else {
		fCStatus = B_OK;
	}
	
	return fCStatus;
}


// ----------------------------------------------------------------------
//	Unset (public)
// ----------------------------------------------------------------------
//	Uninitializes the BVolume. 

void
BVolume::Unset(void) 
{
	fDev = 0L;
	fCStatus = B_NO_INIT;
}


// ----------------------------------------------------------------------
//	Device (public)
// ----------------------------------------------------------------------
//	Returns the object's dev_t number. 

dev_t
BVolume::Device(void) const 
{
	return fDev;
}


// ----------------------------------------------------------------------
//	GetRootDirectory (public)
// ----------------------------------------------------------------------
//	Initializes dir (which must be allocated) to refer to the volume's
//	"root directory." The root directory stands at the "root" of the
//	volume's file hierarchy.
//
//	NOTE: This isn't necessarily the root of the entire file
//	hierarchy, but only the root of the volume hierarchy.
//
//	This function does not change fDev nor fCStatus.

status_t
BVolume::GetRootDirectory(
	BDirectory*		dir) const
{
	status_t		currentStatus = fCStatus;
	
	if ((dir != NULL) && (currentStatus == B_OK)){

		// Obtain the device information for the current device
		//	and initialize the passed-in BDirectory object with
		//	the device and root node values.
		
		fs_info			fsInfo;
		int				err = fs_stat_dev(fDev, &fsInfo);
		
		if (err != 0) {
			currentStatus = errno;
		}
		else {
			node_ref		nodeRef;
			
			nodeRef.device = fsInfo.dev;
				// NOTE: This should be the same as fDev.
			nodeRef.node = fsInfo.root;
			
			currentStatus = dir->SetTo(&nodeRef);
		}

	}
	
	return currentStatus;
}


// ----------------------------------------------------------------------
//	Capacity (public)
// ----------------------------------------------------------------------
//	Returns the volume's total storage capacity (in bytes).

off_t
BVolume::Capacity(void) const 
{
	off_t		totalBytes = 0;
	
	if (fCStatus == B_OK){

		// Obtain the device information for the current device
		//	and calculate the total storage capacity.
		
		fs_info			fsInfo;
		int				err = fs_stat_dev(fDev, &fsInfo);
		
		if (err == 0) {
			totalBytes = fsInfo.block_size * fsInfo.total_blocks;
		}

	}

	return totalBytes;
}


// ----------------------------------------------------------------------
//	FreeBytes (public)
// ----------------------------------------------------------------------
//	Returns the amount of storage that's currently unused on the
//	volume (in bytes).

off_t
BVolume::FreeBytes(void) const 
{
	off_t		remainingBytes = 0L;
	
	if (fCStatus == B_OK){

		// Obtain the device information for the current device
		//	and calculate the free storage available.
		
		fs_info			fsInfo;
		int				err = fs_stat_dev(fDev, &fsInfo);
		
		if (err == 0) {
			remainingBytes = fsInfo.block_size * fsInfo.free_blocks;
		}

	}

	return remainingBytes;
}


// ----------------------------------------------------------------------
//	GetName (public)
// ----------------------------------------------------------------------
//	Copies the name of the volume into the supplied buffer.

status_t
BVolume::GetName(
	char*			name) const 
{
	status_t		currentStatus = fCStatus;
	
	if ((name != NULL) && (currentStatus == B_OK)) {

		// Obtain the device information for the current device
		//	and copies the device name into the buffer.
		
		fs_info			fsInfo;
		int				err = fs_stat_dev(fDev, &fsInfo);
		
		if (err != 0) {
			currentStatus = errno;
		}
		else {
			strcpy(name, fsInfo.volume_name);
			currentStatus = B_OK;
		}

	}
	
	return currentStatus;
}


// ----------------------------------------------------------------------
//	SetName (public)
// ----------------------------------------------------------------------
//	Sets the name of the volume to the supplied string.

status_t
BVolume::SetName(
	const char*		name) 
{
	status_t		currentStatus = B_ERROR;
	
	return currentStatus;
}


// ----------------------------------------------------------------------
//	GetIcon (public)
// ----------------------------------------------------------------------
//	Returns the volume's icon in icon, which specifies the icon to
//	retrieve, either B_MINI_ICON (16x16) or B_LARGE_ICON (32x32).

status_t
BVolume::GetIcon(
	BBitmap*		icon,
	icon_size		which) const
{
	status_t		currentStatus = B_ERROR;
	
	return (currentStatus);
}


// ----------------------------------------------------------------------
//	IsRemovable (public)
// ----------------------------------------------------------------------
//	Tests the volume and returns whether or not it is removable.

bool
BVolume::IsRemovable(void) const
{
	bool		volumeIsRemovable = false;
	
	return (volumeIsRemovable);
}


// ----------------------------------------------------------------------
//	IsReadOnly (public)
// ----------------------------------------------------------------------
//	Tests the volume and returns whether or not it is read-only.

bool
BVolume::IsReadOnly(void) const
{
	bool		volumeIsReadOnly = false;
	
	return (volumeIsReadOnly);
}


// ----------------------------------------------------------------------
//	IsPersistent (public)
// ----------------------------------------------------------------------
//	Tests the volume and returns whether or not it is persistent.

bool
BVolume::IsPersistent(void) const
{
	bool		volumeIsPersistent = false;
	
	return (volumeIsPersistent);
}


// ----------------------------------------------------------------------
//	IsShared (public)
// ----------------------------------------------------------------------
//	Tests the volume and returns whether or not it is shared.

bool
BVolume::IsShared(void) const
{
	bool		volumeIsShared = false;
	
	return (volumeIsShared);
}


// ----------------------------------------------------------------------
//	KnowsMime (public)
// ----------------------------------------------------------------------
//	Tests the volume and returns whether or not it uses MIME types.

bool
BVolume::KnowsMime(void) const
{
	bool		volumeKnowsMime = false;
	
	return (volumeKnowsMime);
}


// ----------------------------------------------------------------------
//	KnowsAttr (public)
// ----------------------------------------------------------------------
//	Tests the volume and returns whether or not its files
//	accept attributes.

bool
BVolume::KnowsAttr(void) const
{
	bool		volumeKnowsAttr = false;
	
	return (volumeKnowsAttr);
}


// ----------------------------------------------------------------------
//	KnowsQuery (public)
// ----------------------------------------------------------------------
//	Tests the volume and returns whether or not it can respond
//	to queries.

bool
BVolume::KnowsQuery(void) const
{
	bool		volumeKnowsQuery = false;
	
	return (volumeKnowsQuery);
}


// ----------------------------------------------------------------------
//	operator == (public)
// ----------------------------------------------------------------------
//	Two BVolume objects are said to be equal if they refer to the
//	same volume, or if they're both uninitialized.
//	Returns whether or not the volumes are equal.

bool
BVolume::operator==(
	const BVolume&		vol) const
{
	bool		areEqual = false;
	
	return (areEqual);
}


// ----------------------------------------------------------------------
//	operator != (public)
// ----------------------------------------------------------------------
//	Two BVolume objects are said to be equal if they refer to the
//	same volume, or if they're both uninitialized.
//	Returns whether or not the volumes are not equal.

bool
BVolume::operator!=(
	const BVolume&		vol) const
{
	bool		areNotEqual = false;
	
	return (areNotEqual);
}


// ----------------------------------------------------------------------
//	operator = (public)
// ----------------------------------------------------------------------
//	In the expression:
//
//		BVolume a = b;
//
//	BVolume a is initialized to refer to the same volume as b.
//	To gauge the success of the assignment, you should call InitCheck()
//	immediately afterwards.
//
//	Assigning a BVolume to itself is safe. 
//	Assigning from an uninitialized BVolume is "successful":
//	The assigned-to BVolume will also be uninitialized (B_NO_INIT).

BVolume&
BVolume::operator=(
	const BVolume&		vol)
{
	return (*this);
}

// FBC
void BVolume::_TurnUpTheVolume1() {}
void BVolume::_TurnUpTheVolume2() {}
void BVolume::_TurnUpTheVolume3() {}
void BVolume::_TurnUpTheVolume4() {}
void BVolume::_TurnUpTheVolume5() {}
void BVolume::_TurnUpTheVolume6() {}
void BVolume::_TurnUpTheVolume7() {}
void BVolume::_TurnUpTheVolume8() {}

/*
#ifdef USE_OPENBEOS_NAMESPACE
}
#endif
*/
