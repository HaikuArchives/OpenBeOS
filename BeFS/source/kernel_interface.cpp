/* kernel_interface - file system interface to BeOS' vnode layer
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include "Debug.h"
#include "cpp.h"
#include "Volume.h"
#include "Inode.h"
#include "Index.h"
#include "BPlusTree.h"
#include "Query.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <malloc.h>
//#include "dmalloc.h"

// BeOS vnode layer stuff
#ifndef _IMPEXP_KERNEL
#	define _IMPEXP_KERNEL
#endif
extern "C" {
//	#include <KernelExport.h>
	#include <fsproto.h>
	#include <lock.h>
	#include <cache.h>
}
#include <fs_index.h>

#ifdef USER
#	define dprintf printf
#endif


/*  Start of fundamental (read-only) required functions */
extern "C" {
	static int bfs_mount(nspace_id nsid, const char *device, ulong flags,
						void *parms, size_t len, void **data, vnode_id *vnid);
	static int bfs_unmount(void *_ns);

	static int bfs_walk(void *_ns, void *_base, const char *file,
						char **newpath, vnode_id *vnid);
	
	static int bfs_read_vnode(void *_ns, vnode_id vnid, char r, void **node);
	static int bfs_release_vnode(void *_ns, void *_node, char r);

	static int bfs_ioctl(void *ns, void *node, void *cookie, int cmd, void *buf,size_t len);
	static int bfs_setflags(void *ns, void *node, void *cookie, int flags);

	static int bfs_rstat(void *_ns, void *_node, struct stat *st);
	static int bfs_open(void *_ns, void *_node, int omode, void **cookie);
	static int bfs_read(void *_ns, void *_node, void *cookie, off_t pos,
						void *buf, size_t *len);
	// bfs_free_cookie - free cookie for file created in open.
	static int bfs_free_cookie(void *ns, void *node, void *cookie);
	static int bfs_close(void *ns, void *node, void *cookie);
	
	// bfs_access - 		checks permissions for access.
	static int bfs_access(void *_ns, void *_node, int mode);

	// directory functions	
	static int bfs_open_dir(void* _ns, void* _node, void** cookie);
	static int bfs_read_dir(void *_ns, void *_node, void *cookie,
						long *num, struct dirent *dirent, size_t bufferSize);
	static int bfs_rewind_dir(void *_ns, void *_node, void *cookie);
	static int bfs_close_dir(void *_ns, void *_node, void *cookie);
	static int bfs_free_dir_cookie(void *_ns, void *_node, void *cookie);
	
	// bfs_rfsstat - Fill in bfs_info struct for device.
	static int bfs_rfsstat(void *_ns, struct fs_info *);
	
	// bfs_read_link - Read in the name of a symbolic link.
	static int bfs_read_link(void *_ns, void *_node, char *buffer, size_t *bufferSize);

	// attribute support
	static int bfs_open_attrdir(void *ns, void *node, void **cookie);
	static int bfs_close_attrdir(void *ns, void *node, void *cookie);
	static int bfs_free_attrdir_cookie(void *ns, void *node, void *cookie);
	static int bfs_rewind_attrdir(void *ns, void *node, void *cookie);
	static int bfs_read_attrdir(void *ns, void *node, void *cookie, long *num,
					struct dirent *buf, size_t bufferSize);
	static int bfs_remove_attr(void *ns, void *node, const char *name);
	static int bfs_rename_attr(void *ns, void *node, const char *oldname,
					const char *newname);
	static int bfs_stat_attr(void *ns, void *node, const char *name,
					struct attr_info *buf);
	static int bfs_write_attr(void *ns, void *node, const char *name, int type,
					const void *buf, size_t *len, off_t pos);
	static int bfs_read_attr(void *ns, void *node, const char *name, int type,
					void *buf, size_t *len, off_t pos);

	// index support
	static int bfs_open_indexdir(void *ns, void **cookie);
	static int bfs_close_indexdir(void *ns, void *cookie);
	static int bfs_free_indexdir_cookie(void *ns, void *node, void *cookie);
	static int bfs_rewind_indexdir(void *ns, void *cookie);
	static int bfs_read_indexdir(void *ns, void *cookie, long *num,struct dirent *dirent,
					size_t bufferSize);
	static int bfs_create_index(void *ns, const char *name, int type, int flags);
	static int bfs_remove_index(void *ns, const char *name);
	static int bfs_rename_index(void *ns, const char *oldname, const char *newname);
	static int bfs_stat_index(void *ns, const char *name, struct index_info *indexInfo);

	// query support
	static int bfs_open_query(void *ns, const char *query, ulong flags,
					port_id port, long token, void **cookie);
	static int bfs_close_query(void *ns, void *cookie);
	static int bfs_free_query_cookie(void *ns, void *node, void *cookie);
	static int bfs_read_query(void *ns, void *cookie, long *num,
					struct dirent *buf, size_t bufsize);

#if 0
	static int		bfs_remove_vnode(void *ns, void *node, char r);
	static int		bfs_secure_vnode(void *ns, void *node);
	static int		bfs_create(void *ns, void *dir, const char *name,
						int perms, int omode, vnode_id *vnid, void **cookie);
	static int		bfs_mkdir(void *ns, void *dir, const char *name, int perms);
	static int		bfs_unlink(void *ns, void *dir, const char *name);
	static int		bfs_rmdir(void *ns, void *dir, const char *name);
	static int		bfs_wstat(void *ns, void *node, struct stat *st, long mask);
	static int		bfs_write(void *ns, void *node, void *cookie, off_t pos,
							const void *buf, size_t *len);
	static int		bfs_ioctl(void *ns, void *node, void *cookie, int cmd,
							void *buf, size_t len);
	static int		bfs_wfsstat(void *ns, struct bfs_info *);
	static int		bfs_sync(void *ns);
	static int     	bfs_initialize(const char *devname, void *parms, size_t len);
#endif 
}	// extern "C"


/* vnode_ops struct. Fill this in to tell the kernel how to call
	functions in your driver.
*/

vnode_ops fs_entry =  {
	&bfs_read_vnode,			// read_vnode
	&bfs_release_vnode,			// write_vnode
	NULL, 						// remove_vnode
	NULL,						// secure_vnode
	&bfs_walk,					// walk
	&bfs_access,				// access
	NULL, 						// create
	NULL, 						// mkdir
	NULL,						// symlink
	NULL,						// link
	NULL,						// rename
	NULL, 						// unlink
	NULL, 						// rmdir
	&bfs_read_link,				// readlink
	&bfs_open_dir,				// opendir
	&bfs_close_dir,				// closedir
	&bfs_free_dir_cookie,		// free_dircookie
	&bfs_rewind_dir,			// rewinddir
	&bfs_read_dir,				// readdir
	&bfs_open,					// open file
	&bfs_close,					// close file
	&bfs_free_cookie,			// free cookie
	&bfs_read,					// read file
	NULL, 						// write file
	NULL,						// readv
	NULL,						// writev
	&bfs_ioctl,					// ioctl
	&bfs_setflags,				// setflags file
	&bfs_rstat,					// rstat
	NULL, 						// wstat
	NULL,						// fsync
	NULL,						// initialize
	&bfs_mount,					// mount
	&bfs_unmount,				// unmount
	NULL,						// sync
	&bfs_rfsstat,				// rfsstat
	NULL,						// wfsstat
	NULL,						// select
	NULL,						// deselect

	&bfs_open_indexdir,			// open index dir
	&bfs_close_indexdir,		// close index dir
	&bfs_free_indexdir_cookie,	// free index dir cookie
	&bfs_rewind_indexdir,		// rewind index dir
	&bfs_read_indexdir,			// read index dir
	&bfs_create_index,			// create index
	&bfs_remove_index,			// remove index
	&bfs_rename_index,			// rename index
	&bfs_stat_index,			// stat index

	&bfs_open_attrdir,			// open attr dir
	&bfs_close_attrdir,			// close attr dir
	&bfs_free_attrdir_cookie,	// free attr dir cookie
	&bfs_rewind_attrdir,		// rewind attr dir
	&bfs_read_attrdir,			// read attr dir
	&bfs_write_attr,			// write attr
	&bfs_read_attr,				// read attr
	&bfs_remove_attr,			// remove attr
	&bfs_rename_attr,			// rename attr
	&bfs_stat_attr,				// stat attr

	&bfs_open_query,			// open query
	&bfs_close_query,			// close query
	&bfs_free_query_cookie,		// free query cookie
	&bfs_read_query				// read query
};

#define BFS_IO_SIZE 65536
int32	api_version = B_CUR_FS_API_VERSION;


static int
bfs_mount(nspace_id nsid, const char *device, ulong flags, void *parms,
		size_t len, void **data, vnode_id *rootID)
{
	FUNCTION();

#ifndef USER
	load_driver_symbols("obfs");
#endif

	Volume *volume = new Volume(nsid);

	status_t status;
	if ((status = volume->Mount(device,flags)) == B_OK) {
		*data = volume;
		*rootID = volume->ToVnode(volume->Root());
		INFORM(("mounted \"%s\" (root node at %Ld, device = %s)\n",volume->Name(),*rootID,device));
	}
	else
		delete volume;

	D(if (status < B_OK)
		RETURN_ERROR(status);
	);
	return status;
}


static int
bfs_unmount(void *ns)
{
	FUNCTION();
	Volume* volume = (Volume *)ns;

	int status = volume->Unmount();
	delete volume;

	//remove_cached_device_blocks(ns->fd, 0);
	//result = close(ns->fd);
	//free(ns);

	return status;
}

// bfs_rfsstat - Fill in bfs_info struct for device.

static int
bfs_rfsstat(void *_ns, struct fs_info *info)
{
	FUNCTION();
	Volume *volume = (Volume *)_ns;
	
	// File system flags.
	info->flags = B_FS_IS_PERSISTENT | B_FS_IS_READONLY | B_FS_HAS_ATTR
			| B_FS_HAS_MIME | B_FS_HAS_QUERY;

	// whatever is appropriate here? Just use the same value as BFS (and iso9660) for now
	info->io_size = BFS_IO_SIZE;

	info->block_size = volume->BlockSize();
	info->total_blocks = volume->NumBlocks();

	// Free blocks = 0, read only
	info->free_blocks = 0;
	
	// Volume name
	strncpy(info->volume_name, volume->Name(), sizeof(info->volume_name) - 1);
	*(info->volume_name + sizeof(info->volume_name) - 1) = '\0';
	
	// File system name
	strcpy(info->fsh_name,"obfs");
	
	return B_NO_ERROR;
}


//	#pragma mark -

// bfs_walk - the walk function just "walks" through a directory looking for
//				the specified file. When you find it, call get_vnode on its vnid to init
//				it for the kernel.

static int
bfs_walk(void *_ns, void *_directory, const char *file, char **_resolvedPath, vnode_id *vnid)
{
	FUNCTION_START(("file = %s\n",file));
	Volume *volume = (Volume *)_ns;
	
	Inode *directory = (Inode *)_directory;
	BPlusTree *tree;
	if (directory->GetTree(&tree) != B_OK)
		RETURN_ERROR(B_BAD_VALUE);

	//off_t offset;
	status_t status = tree->Find((uint8 *)file,(uint16)strlen(file),vnid);
	if (status != B_OK)
		RETURN_ERROR(status);

	Inode *inode;
	if ((status = get_vnode(volume->ID(),*vnid,(void **)&inode)) != B_OK) {
		REPORT_ERROR(status);
		return B_ENTRY_NOT_FOUND;
	}

	// Is inode a symlink? Then resolve it, if we should

	if (inode->IsSymLink() && _resolvedPath != NULL) {
		status_t status = B_OK;
		char *newPath = NULL;
		
		// Symbolic links can store their target in the data stream (for links
		// that take more than 144 bytes of storage [the size of the data_stream
		// structure]), or directly instead of the data_stream class
		// So we have to deal with both cases here.
		
		// Note: we would naturally call bfs_read_link() here, but the API of the
		// vnode layer would require us to always reserve a large chunk of memory
		// for the path, so we're not going to do that

		if (inode->Flags() & INODE_LONG_SYMLINK) {
			size_t readBytes = inode->Node()->data.size;
			char *data = (char *)malloc(readBytes);
			if (data != NULL) {
				status = inode->ReadAt(0, data, &readBytes);
				if (status == B_OK && readBytes == inode->Node()->data.size)
					status = new_path(data, &newPath);

				free(data);
			} else
				status = B_NO_MEMORY;
		} else
			status = new_path((char *)&inode->Node()->short_symlink, &newPath);

		put_vnode(volume->ID(), inode->ID());
		if (status == B_OK)
			*_resolvedPath = newPath;

		RETURN_ERROR(status);
	}

	return B_OK;
}

// bfs_read_vnode - Using vnode id, read in vnode information into fs-specific struct,
//				and return it in node. the reenter flag tells you if this function
//				is being called via some other fs routine, so that things like 
//				double-locking can be avoided.

static int
bfs_read_vnode(void *_ns, vnode_id vnid, char reenter, void **node)
{
	FUNCTION_START(("vnode_id = %Ld\n",vnid));
	Volume *volume = (Volume *)_ns;
	
	Inode *inode = new Inode(volume,vnid,reenter);
	if (inode->InitCheck() == B_OK) {
		*node = (void *)inode;
		return B_OK;
	}

	delete inode;
	RETURN_ERROR(B_ERROR);
}


static int
bfs_release_vnode(void *ns, void *_node, char reenter)
{
	FUNCTION_START(("node = %p\n",_node));
	Inode *inode = (Inode *)_node;
	
	delete inode;

	return B_NO_ERROR;
}


int 
bfs_ioctl(void *ns, void *node, void *cookie, int cmd, void *buf, size_t len)
{
	FUNCTION_START(("node = %p, cmd = %d, buf = %p, len = %ld\n",node,cmd,buf,len));
	return B_BAD_VALUE;
}


int 
bfs_setflags(void *ns, void *node, void *cookie, int flags)
{
	FUNCTION_START(("node = %p, flags = %d",node,flags));
	return B_OK;
}

// bfs_rstat - fill in stat struct

static int
bfs_rstat(void *_ns, void *_node, struct stat *st)
{
	FUNCTION();

	Volume *volume = (Volume *)_ns;
	Inode *inode = (Inode *)_node;
	bfs_inode *node = inode->Node();

	st->st_dev = volume->ID();
	st->st_ino = inode->ID();
	st->st_nlink = 1;
	st->st_blksize = BFS_IO_SIZE;

	st->st_uid = node->uid;
	st->st_gid = node->gid;
	st->st_mode = node->mode;
	st->st_size = node->data.size;

	st->st_atime = time(NULL);
	st->st_mtime = st->st_ctime = (time_t)(node->last_modified_time >> INODE_TIME_SHIFT);
	st->st_crtime = (time_t)(node->create_time >> INODE_TIME_SHIFT);

	return B_NO_ERROR;
}

// bfs_open - Create a vnode cookie, if necessary, to use when
// 				reading/writing a file

static int
bfs_open(void *_ns, void *_node, int omode, void **cookie)
{
	FUNCTION();

	// we could actually use a cookie to keep track of:
	//	- the last block_run
	//	- the location in the data_stream (indirect, double indirect,
	//	  position in block_run array)
	//
	// This could greatly speed up continuous reads of big files, especially
	// in the indirect block section.

	return B_OK;
}

// bfs_read - Read a file specified by node, using information in cookie
//				and at offset specified by pos. read len bytes into buffer buf.

static int
bfs_read(void *_ns, void *_node, void *_cookie, off_t pos, void *buffer, size_t *_length)
{
	//FUNCTION();
	Inode *inode = (Inode *)_node;
	
	if (inode->IsSymLink()) {
		*_length = 0;
		RETURN_ERROR(B_BAD_VALUE);
	}
	
	return inode->ReadAt(pos,buffer,_length);
}

// bfs_close - Do whatever is necessary to close a file, EXCEPT for freeing
//				the cookie!

static int
bfs_close(void * /*ns*/, void * /*node*/, void * /*cookie*/)
{
	FUNCTION();
	return B_OK;
}


static int
bfs_free_cookie(void * /*ns*/, void * /*node*/, void * /*cookie*/)
{
	return B_OK;
}

// bfs_access - checks permissions for access.

static int
bfs_access(void *ns, void *node, int mode)
{
	FUNCTION();
	return B_OK;
}

static int
bfs_read_link(void *_ns, void *_node, char *buffer, size_t *bufferSize)
{
	FUNCTION();

	Inode *inode = (Inode *)_node;
	
	if (inode->IsSymLink())
		RETURN_ERROR(B_BAD_VALUE);

	if (inode->Flags() & INODE_LONG_SYMLINK)
		RETURN_ERROR(inode->ReadAt(0, buffer, bufferSize));

	size_t numBytes = strlen((char *)&inode->Node()->short_symlink);
	if (numBytes > *bufferSize) {
		FATAL(("link size is greater than buffer size, %ld > %ld\n",numBytes,*bufferSize));
		numBytes = *bufferSize;
	} else
		*bufferSize = numBytes;

	memcpy(buffer, inode->Node()->short_symlink, numBytes);

	return B_OK;
}


//	#pragma mark -

// bfs_opendir - creates fs-specific "cookie" struct that keeps track of where
//					you are at in reading through directory entries in bfs_readdir.

static int
bfs_open_dir(void *_ns, void *_node, void **_cookie)
{
	FUNCTION();
	
	if (_ns == NULL || _node == NULL || _cookie == NULL)
		RETURN_ERROR(B_BAD_VALUE);
	
	Inode *inode = (Inode *)_node;
	
	if (!inode->IsDirectory())
		RETURN_ERROR(B_BAD_VALUE);

	BPlusTree *tree;
	if (inode->GetTree(&tree) != B_OK)
		RETURN_ERROR(B_BAD_VALUE);

	TreeIterator *iterator = new TreeIterator(tree);
	if (iterator == NULL)
		RETURN_ERROR(B_NO_MEMORY);

	*_cookie = iterator;
	return B_OK;
}

// bfs_readdir - read 1 or more dirents, keep state in cookie, return
//					0 when no more entries.

static int
bfs_read_dir(void *_ns, void *_node, void *_cookie, long *num, 
			struct dirent *dirent, size_t bufferSize)
{
	FUNCTION();

	TreeIterator *iterator = (TreeIterator *)_cookie;
	if (iterator == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	uint16 length;
	vnode_id id;
	status_t status = iterator->GetNextEntry(dirent->d_name,&length,bufferSize,&id);
	if (status == B_ENTRY_NOT_FOUND) {
		*num = 0;
		return B_OK;
	} else if (status != B_OK)
		RETURN_ERROR(status);

	Volume *volume = (Volume *)_ns;

	dirent->d_dev = volume->ID();
	dirent->d_ino = id;
	dirent->d_reclen = length;

	*num = 1;
	return B_OK;
}
			
// bfs_rewinddir - set cookie to represent beginning of directory, so
//					later bfs_readdir calls start at beginning.

static int
bfs_rewind_dir(void * /*ns*/, void * /*node*/, void *_cookie)
{
	FUNCTION();
	TreeIterator *iterator = (TreeIterator *)_cookie;

	if (iterator == NULL)
		RETURN_ERROR(B_BAD_VALUE);
	
	return iterator->Rewind();
}

// bfs_closedir - Do whatever you need to to close a directory (sometimes
//					nothing), but DON'T free the cookie!

static int		
bfs_close_dir(void * /*ns*/, void * /*node*/, void * /*_cookie*/)
{
	FUNCTION();
	return B_OK;
}

// bfs_free_dircookie - Free the fs-specific cookie struct

static int
bfs_free_dir_cookie(void *ns, void *node, void *_cookie)
{
	TreeIterator *iterator = (TreeIterator *)_cookie;
	
	if (iterator == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	delete iterator;
	return B_OK;
}


//	#pragma mark -
//	Attribute functions

int 
bfs_open_attrdir(void *_ns, void *_node, void **cookie)
{
	FUNCTION();
	
	Inode *inode = (Inode *)_node;
	if (inode == NULL || inode->Node() == NULL)
		RETURN_ERROR(B_ERROR);

	AttributeIterator *iterator = new AttributeIterator(inode);
	if (iterator == NULL)
		RETURN_ERROR(B_NO_MEMORY);

	*cookie = iterator;
	return B_OK;
}


int
bfs_close_attrdir(void *ns, void *node, void *cookie)
{
	FUNCTION();
	return B_OK;
}


int
bfs_free_attrdir_cookie(void *ns, void *node, void *_cookie)
{
	FUNCTION();
	AttributeIterator *iterator = (AttributeIterator *)_cookie;

	if (iterator == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	delete iterator;
	return B_OK;
}


int
bfs_rewind_attrdir(void *_ns, void *_node, void *_cookie)
{
	FUNCTION();
	
	AttributeIterator *iterator = (AttributeIterator *)_cookie;
	if (iterator == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	RETURN_ERROR(iterator->Rewind());
}


int 
bfs_read_attrdir(void *_ns, void *node, void *_cookie, long *num, struct dirent *dirent, size_t bufsize)
{
	FUNCTION();
	AttributeIterator *iterator = (AttributeIterator *)_cookie;

	if (iterator == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	uint32 type;
	size_t length;
	status_t status = iterator->GetNext(dirent->d_name,&length,&type,&dirent->d_ino);
	if (status == B_ENTRY_NOT_FOUND) {
		*num = 0;
		return B_OK;
	} else if (status != B_OK)
		RETURN_ERROR(status);

	Volume *volume = (Volume *)_ns;

	dirent->d_dev = volume->ID();
	dirent->d_reclen = length;

	*num = 1;
	return B_OK;
}


int
bfs_remove_attr(void *ns, void *node, const char *name)
{
	FUNCTION_START(("name = \"%s\"\n",name));
	RETURN_ERROR(B_ENTRY_NOT_FOUND);
}


int
bfs_rename_attr(void *ns, void *node, const char *oldname,const char *newname)
{
	FUNCTION_START(("name = \"%s\",to = \"%s\"\n",oldname,newname));
	RETURN_ERROR(B_ENTRY_NOT_FOUND);
}


int
bfs_stat_attr(void *ns, void *_node, const char *name,struct attr_info *attrInfo)
{
	FUNCTION_START(("name = \"%s\"\n",name));

	Inode *inode = (Inode *)_node;
	if (inode == NULL || inode->Node() == NULL)
		RETURN_ERROR(B_ERROR);
	
	small_data *smallData = inode->FindSmallData((const char *)name);
	if (smallData != NULL) {
		attrInfo->type = smallData->type;
		attrInfo->size = smallData->data_size;

		return B_OK;
	}
	// search in the attribute directory
	Inode *attribute = inode->GetAttribute(name);
	if (attribute != NULL) {
		attrInfo->type = attribute->Node()->type;
		attrInfo->size = attribute->Node()->data.size;

		inode->ReleaseAttribute(attribute);
		return B_OK;
	}

	RETURN_ERROR(B_ENTRY_NOT_FOUND);
}


int
bfs_write_attr(void *ns, void *node, const char *name, int type,const void *buf, size_t *len, off_t pos)
{
	FUNCTION_START(("name = \"%s\"\n",name));
	RETURN_ERROR(B_ERROR);
}


int
bfs_read_attr(void *ns, void *_node, const char *name, int type,void *buffer, size_t *_length, off_t pos)
{
	Inode *inode = (Inode *)_node;
	FUNCTION_START(("id = %Ld, name = \"%s\", len = %ld\n",inode->ID(),name,*_length));
	
	if (inode == NULL || inode->Node() == NULL)
		RETURN_ERROR(B_ERROR);

	if (pos < 0)
		pos = 0;

	// search in the small_data section
	small_data *smallData = inode->FindSmallData(name);
	if (smallData != NULL) {
		size_t length = *_length;
		if (pos > smallData->data_size) {
			*_length = 0;
			return B_OK;
		}
		if (length + pos > smallData->data_size)
			length = smallData->data_size - pos;

		memcpy(buffer,smallData->Data() + pos,length);
		*_length = length;
		return B_OK;
	}
	// search in the attribute directory
	Inode *attribute = inode->GetAttribute(name);
	if (attribute != NULL) {
		status_t status = attribute->ReadAt(pos,buffer,_length);
		inode->ReleaseAttribute(attribute);
		RETURN_ERROR(status);
	}

	RETURN_ERROR(B_ENTRY_NOT_FOUND);
}


//	#pragma mark -
//	Index functions


int 
bfs_open_indexdir(void *_ns, void **_cookie)
{
	FUNCTION();

	if (_ns == NULL || _cookie == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	Volume *volume = (Volume *)_ns;
	
	if (volume->IndicesNode() == NULL)
		RETURN_ERROR(B_ENTRY_NOT_FOUND);

	// since we are storing the indices root node in our Volume object,
	// we can just use the directory traversal functions (since it is
	// just a directory)

	RETURN_ERROR(bfs_open_dir(_ns,volume->IndicesNode(),_cookie));
}


int 
bfs_close_indexdir(void *_ns, void *_cookie)
{
	FUNCTION();
	if (_ns == NULL || _cookie == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	Volume *volume = (Volume *)_ns;
	RETURN_ERROR(bfs_close_dir(_ns,volume->IndicesNode(),_cookie));
}


int 
bfs_free_indexdir_cookie(void *_ns, void *_node, void *_cookie)
{
	FUNCTION();
	if (_ns == NULL || _cookie == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	Volume *volume = (Volume *)_ns;
	RETURN_ERROR(bfs_free_dir_cookie(_ns,volume->IndicesNode(),_cookie));
}


int 
bfs_rewind_indexdir(void *_ns, void *_cookie)
{
	FUNCTION();
	if (_ns == NULL || _cookie == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	Volume *volume = (Volume *)_ns;
	RETURN_ERROR(bfs_rewind_dir(_ns,volume->IndicesNode(),_cookie));
}


int 
bfs_read_indexdir(void *_ns, void *_cookie, long *num, struct dirent *dirent, size_t bufferSize)
{
	FUNCTION();
	if (_ns == NULL || _cookie == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	Volume *volume = (Volume *)_ns;
	RETURN_ERROR(bfs_read_dir(_ns,volume->IndicesNode(),_cookie,num,dirent,bufferSize));
}


int 
bfs_create_index(void *ns, const char *name, int type, int flags)
{
	FUNCTION();
	RETURN_ERROR(B_ERROR);
}


int 
bfs_remove_index(void *ns, const char *name)
{
	FUNCTION();
	RETURN_ERROR(B_ENTRY_NOT_FOUND);
}


int 
bfs_rename_index(void *ns, const char *oldname, const char *newname)
{
	FUNCTION_START(("from = %s, to = %s\n",oldname,newname));
	RETURN_ERROR(B_ENTRY_NOT_FOUND);
}


int 
bfs_stat_index(void *_ns, const char *name, struct index_info *indexInfo)
{
	FUNCTION_START(("name = %s\n",name));
	if (_ns == NULL || name == NULL || indexInfo == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	Volume *volume = (Volume *)_ns;
	Index index(volume);
	status_t status = index.SetTo(name);
	if (status < B_OK)
		RETURN_ERROR(status);

	bfs_inode *node = index.Node()->Node();

	indexInfo->type = index.Type();
	indexInfo->size = node->data.size;
	indexInfo->modification_time = (time_t)(node->last_modified_time >> INODE_TIME_SHIFT);
	indexInfo->creation_time = (time_t)(node->create_time >> INODE_TIME_SHIFT);
	indexInfo->uid = node->uid;
	indexInfo->gid = node->gid;

	return B_OK;
}


//	#pragma mark -
//	Query functions


int 
bfs_open_query(void *_ns,const char *expression,ulong flags,port_id port,long token,void **cookie)
{
	FUNCTION();
	if (_ns == NULL || expression == NULL || cookie == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	PRINT(("query = \"%s\", flags = %lu, port_id = %ld, token = %ld\n",expression,flags,port,token));

	Volume *volume = (Volume *)_ns;
	
	Query *query = new Query((char *)expression);
	if (query == NULL)
		RETURN_ERROR(B_NO_MEMORY);
	
	if (query->InitCheck() < B_OK) {
		FATAL(("Could not parse query, stopped at: \"%s\"\n",query->Position()));
		delete query;
		RETURN_ERROR(B_BAD_VALUE);
	}

	QueryFetcher *fetcher = new QueryFetcher(volume,query);
	if (fetcher == NULL) {
		delete query;
		RETURN_ERROR(B_NO_MEMORY);
	}
	*cookie = (void *)fetcher;
	
	return B_OK;
}


int
bfs_close_query(void *ns, void *cookie)
{
	FUNCTION();
	return B_OK;
}


int
bfs_free_query_cookie(void *ns, void *node, void *cookie)
{
	FUNCTION();
	if (cookie == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	QueryFetcher *fetcher = (QueryFetcher *)cookie;
	Query *query = fetcher->GetQuery();
	delete fetcher;
	delete query;

	return B_OK;
}


int
bfs_read_query(void */*ns*/,void *cookie,long *num,struct dirent *dirent,size_t bufferSize)
{
	FUNCTION();
	QueryFetcher *fetcher = (QueryFetcher *)cookie;
	if (fetcher == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	status_t status = fetcher->GetNextEntry(dirent,bufferSize);
	if (status == B_OK)
		*num = 1;
	else if (status == B_ENTRY_NOT_FOUND)
		*num = 0;
	else
		return status;

	return B_OK;
}

