#include <Statable.h>

#include "kernel_interface.h"
#include <posix/sys/stat.h>
#include <Node.h>

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
	return B_BAD_VALUE;
}

status_t 
BStatable::SetOwner(uid_t owner)
{
	return B_BAD_VALUE;
}
	
status_t
BStatable::GetGroup(gid_t *group) const
{
	return B_BAD_VALUE;
}

status_t
BStatable::SetGroup(gid_t group)
{
	return B_BAD_VALUE;
}
	
status_t
BStatable::GetPermissions(mode_t *perms) const
{
	return B_BAD_VALUE;
}

status_t
BStatable::SetPermissions(mode_t perms)
{
	return B_BAD_VALUE;
}

status_t
BStatable::GetSize(off_t *size) const
{
	return B_BAD_VALUE;
}
	
status_t
BStatable::GetModificationTime(time_t *mtime) const
{
	return B_BAD_VALUE;
}

status_t
BStatable::SetModificationTime(time_t mtime)
{
	return B_BAD_VALUE;
}

status_t
BStatable::GetCreationTime(time_t *ctime) const
{
	return B_BAD_VALUE;
}

status_t
BStatable::SetCreationTime(time_t ctime)
{
	return B_BAD_VALUE;
}

status_t
BStatable::GetAccessTime(time_t *atime) const
{
	return B_BAD_VALUE;
}

status_t
BStatable::SetAccessTime(time_t atime)
{
	return B_BAD_VALUE;
}

status_t
BStatable::GetVolume(BVolume *vol) const
{
	return B_BAD_VALUE;
}
