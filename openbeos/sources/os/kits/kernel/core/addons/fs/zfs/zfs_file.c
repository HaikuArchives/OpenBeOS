/*
** Copyright 2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <kernel.h>
#include <errors.h>

#include <zfs.h>

int zfs_open(fs_cookie fs, fs_vnode v, file_cookie *cookie, stream_type st, int oflags)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

int zfs_close(fs_cookie fs, fs_vnode v, file_cookie cookie)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

int zfs_freecookie(fs_cookie fs, fs_vnode v, file_cookie cookie)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

int zfs_fsync(fs_cookie fs, fs_vnode v)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

ssize_t zfs_read(fs_cookie fs, fs_vnode v, file_cookie cookie, void *buf, off_t pos, ssize_t len)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

ssize_t zfs_write(fs_cookie fs, fs_vnode v, file_cookie cookie, const void *buf, off_t pos, ssize_t len)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

int zfs_seek(fs_cookie fs, fs_vnode v, file_cookie cookie, off_t pos, seek_type st)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

int zfs_ioctl(fs_cookie fs, fs_vnode v, file_cookie cookie, int op, void *buf, size_t len)
{
	return ERR_NOT_IMPLEMENTED_YET;
}

