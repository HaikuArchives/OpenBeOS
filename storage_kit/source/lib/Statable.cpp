#include <Statable.h>

#include "kernel_interface.h"

bool
BStatable::IsFile() const
{
	return false;
}

bool
BStatable::IsDirectory() const
{
	return false;
}

bool
BStatable::IsSymLink() const
{
	return false;
}
	
status_t 
BStatable::GetNodeRef(node_ref *ref) const
{
	return B_BAD_VALUE;
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
