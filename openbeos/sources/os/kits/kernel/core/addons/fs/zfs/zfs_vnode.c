/*
** Copyright 2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <kernel.h>
#include <errors.h>

#include <zfs.h>

int zfs_getvnode(fs_cookie fs, vnode_id id, fs_vnode *v, bool r)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

int zfs_putvnode(fs_cookie fs, fs_vnode v, bool r)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

int zfs_removevnode(fs_cookie fs, fs_vnode v, bool r)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

int zfs_canpage(fs_cookie fs, fs_vnode v)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

ssize_t zfs_readpage(fs_cookie fs, fs_vnode v, iovecs *vecs, off_t pos)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

ssize_t zfs_writepage(fs_cookie fs, fs_vnode v, iovecs *vecs, off_t pos)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

int zfs_create(fs_cookie fs, fs_vnode dir, const char *name, stream_type st, void *create_args, vnode_id *new_vnid)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

int zfs_unlink(fs_cookie fs, fs_vnode dir, const char *name)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

int zfs_rename(fs_cookie fs, fs_vnode olddir, const char *oldname, fs_vnode newdir, const char *newname)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

int zfs_rstat(fs_cookie fs, fs_vnode v, struct file_stat *stat)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

int zfs_wstat(fs_cookie fs, fs_vnode v, struct file_stat *stat, int stat_mask)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

