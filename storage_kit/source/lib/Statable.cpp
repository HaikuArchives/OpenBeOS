#include <Statable.h>

#include "kernel_interface.h"
#include <posix/sys/stat.h>

#include <Node.h>
#include <Volume.h>

bool
BStatable::IsFile() const
{
	struct stat statData;
	if ( GetStat(&statData) == B_OK )
		return S_ISREG(statData.st_mode);
	else 
		return false;
}

bool
BStatable::IsDirectory() const
{
	struct stat statData;
	if ( GetStat(&statData) == B_OK )
		return S_ISDIR(statData.st_mode);
	else 
		return false;
}

bool
BStatable::IsSymLink() const
{
	struct stat statData;
	if ( GetStat(&statData) == B_OK )
		return S_ISLNK(statData.st_mode);
	else 
		return false;
}
	
status_t 
BStatable::GetNodeRef(node_ref *ref) const
{
	struct stat statData;
	status_t error;

	error = GetStat(&statData);
	
	if(error == B_OK) {
		ref->device  = statData.st_dev;
		ref->node = statData.st_ino;
	}

	return error;
}
	
status_t 
BStatable::GetOwner(uid_t *owner) const
{
  struct stat statData;

  error = GetStat(&statData);
  if (error == B_OK) {
	*owner = statData.st_uid;
  }

  return error;
}

status_t 
BStatable::SetOwner(uid_t owner)
{
  
  return B_BAD_VALUE;
}
	
status_t
BStatable::GetGroup(gid_t *group) const
{
  struct stat statData;

  error = GetStat(&statData);
  if (error == B_OK) {
	*group = statData.st_gid;
  }

  return error;
}

status_t
BStatable::SetGroup(gid_t group)
{
	return B_BAD_VALUE;
}
	
status_t
BStatable::GetPermissions(mode_t *perms) const
{
  struct stat statData;

  error = GetStat(&statData);
  if (error == B_OK) {
	*perms = statData.st_mode;
  }

  return error;
}

status_t
BStatable::SetPermissions(mode_t perms)
{
	return B_BAD_VALUE;
}

status_t
BStatable::GetSize(off_t *size) const
{
  struct stat statData;

  error = GetStat(&statData);
  if (error == B_OK) {
	*size = statData.st_size;
  }

  return error;
}
	
status_t
BStatable::GetModificationTime(time_t *mtime) const
{
  struct stat statData;

  error = GetStat(&statData);
  if (error == B_OK) {
	*mtime = statData.st_mtime;
  }

  return error;
}

status_t
BStatable::SetModificationTime(time_t mtime)
{
	return B_BAD_VALUE;
}

status_t
BStatable::GetCreationTime(time_t *ctime) const
{
  struct stat statData;

  error = GetStat(&statData);
  if (error == B_OK) {
	*ctime = statData.st_ctime;
  }

  return error;
}

status_t
BStatable::SetCreationTime(time_t ctime)
{
	return B_BAD_VALUE;
}

status_t
BStatable::GetAccessTime(time_t *atime) const
{
  struct stat statData;

  error = GetStat(&statData);
  if (error == B_OK) {
	*atime = statData.st_atime;
  }

  return error;
}

status_t
BStatable::SetAccessTime(time_t atime)
{
	return B_BAD_VALUE;
}

status_t
BStatable::GetVolume(BVolume *vol) const
{
  struct stat statData;

  error = GetStat(&statData);
  if (error == B_OK) {
	if(vol != null) {
	  vol->SetTo(statData.device);
	} else {
	  vol = new BVolume(statData.device);
	}
  }

  return error;
}
