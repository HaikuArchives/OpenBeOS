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

#include <string.h>
#include <stdio.h>

// BeOS vnode layer stuff
#include <KernelExport.h>
#ifndef _IMPEXP_KERNEL
#	define _IMPEXP_KERNEL
#endif

extern "C" {
	#include <fsproto.h>
	#include <lock.h>
	#include <cache.h>
}
#include <fs_index.h>

#ifdef USER
#	define dprintf printf
#endif


extern "C" {
	static int bfs_mount(nspace_id nsid, const char *device, ulong flags,
					void *parms, size_t len, void **data, vnode_id *vnid);
	static int bfs_unmount(void *_ns);
	static int bfs_read_fs_stat(void *_ns, struct fs_info *);
	static int bfs_write_fs_stat(void *ns, struct fs_info *, long mode);
	static int bfs_initialize(const char *devname, void *parms, size_t len);

	static int bfs_sync(void *ns);

	static int bfs_read_vnode(void *_ns, vnode_id vnid, char r, void **node);
	static int bfs_release_vnode(void *_ns, void *_node, char r);
	static int bfs_remove_vnode(void *ns, void *node, char r);

	static int bfs_walk(void *_ns, void *_base, const char *file,
					char **newpath, vnode_id *vnid);
	
	static int bfs_ioctl(void *ns, void *node, void *cookie, int cmd, void *buf,size_t len);
	static int bfs_setflags(void *ns, void *node, void *cookie, int flags);

	static int bfs_select(void *ns, void *node, void *cookie, uint8 event,
					uint32 ref, selectsync *sync);
	static int bfs_deselect(void *ns, void *node, void *cookie, uint8 event,
					selectsync *sync);
	static int bfs_fsync(void *ns,void *node);

	static int bfs_create(void *ns, void *dir, const char *name,
					int perms, int omode, vnode_id *vnid, void **cookie);
	static int bfs_symlink(void *ns, void *dir, const char *name,
					const char *path);
	static int bfs_link(void *ns, void *dir, const char *name, void *node);
	static int bfs_unlink(void *ns, void *dir, const char *name);
	static int bfs_rename(void *ns, void *oldDir, const char *oldName, void *newDir, const char *newName);

	static int bfs_read_stat(void *_ns, void *_node, struct stat *st);
	static int bfs_write_stat(void *ns, void *node, struct stat *st, long mask);

	static int bfs_open(void *_ns, void *_node, int omode, void **cookie);
	static int bfs_read(void *_ns, void *_node, void *cookie, off_t pos,
					void *buf, size_t *len);
	static int bfs_write(void *ns, void *node, void *cookie, off_t pos,
					const void *buf, size_t *len);
	static int bfs_free_cookie(void *ns, void *node, void *cookie);
	static int bfs_close(void *ns, void *node, void *cookie);
	
	static int bfs_access(void *_ns, void *_node, int mode);
	static int bfs_read_link(void *_ns, void *_node, char *buffer, size_t *bufferSize);

	// directory functions	
	static int bfs_mkdir(void *ns, void *dir, const char *name, int perms);
	static int bfs_rmdir(void *ns, void *dir, const char *name);
	static int bfs_open_dir(void *_ns, void *_node, void **cookie);
	static int bfs_read_dir(void *_ns, void *_node, void *cookie,
					long *num, struct dirent *dirent, size_t bufferSize);
	static int bfs_rewind_dir(void *_ns, void *_node, void *cookie);
	static int bfs_close_dir(void *_ns, void *_node, void *cookie);
	static int bfs_free_dir_cookie(void *_ns, void *_node, void *cookie);

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
}	// extern "C"


/* vnode_ops struct. Fill this in to tell the kernel how to call
	functions in your driver.
*/

vnode_ops fs_entry =  {
	&bfs_read_vnode,			// read_vnode
	&bfs_release_vnode,			// write_vnode
	&bfs_remove_vnode,			// remove_vnode
	NULL,						// secure_vnode (not needed)
	&bfs_walk,					// walk
	&bfs_access,				// access
	&bfs_create,				// create
	&bfs_mkdir,					// mkdir
	&bfs_symlink,				// symlink
	&bfs_link,					// link
	&bfs_rename,				// rename
	&bfs_unlink,				// unlink
	&bfs_rmdir,					// rmdir
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
	&bfs_write,					// write file
	NULL,						// readv
	NULL,						// writev
	&bfs_ioctl,					// ioctl
	&bfs_setflags,				// setflags file
	&bfs_read_stat,				// read stat
	&bfs_write_stat,			// write stat
	&bfs_fsync,					// fsync
	&bfs_initialize,			// initialize
	&bfs_mount,					// mount
	&bfs_unmount,				// unmount
	&bfs_sync,					// sync
	&bfs_read_fs_stat,			// read fs stat
	&bfs_write_fs_stat,			// write fs stat
	&bfs_select,				// select
	&bfs_deselect,				// deselect

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


/**	Fill in bfs_info struct for device.
 */

static int
bfs_read_fs_stat(void *_ns, struct fs_info *info)
{
	FUNCTION();
	Volume *volume = (Volume *)_ns;
	
	// File system flags.
	info->flags = B_FS_IS_PERSISTENT | B_FS_HAS_ATTR | B_FS_HAS_MIME | B_FS_HAS_QUERY |
			(volume->IsReadOnly() ? B_FS_IS_READONLY : 0);

	// whatever is appropriate here? Just use the same value as BFS (and iso9660) for now
	info->io_size = BFS_IO_SIZE;

	info->block_size = volume->BlockSize();
	info->total_blocks = volume->NumBlocks();

	info->free_blocks = volume->NumBlocks() - volume->UsedBlocks();

	// Volume name
	strncpy(info->volume_name, volume->Name(), sizeof(info->volume_name) - 1);
	info->volume_name[sizeof(info->volume_name) - 1] = '\0';
	
	// File system name
	strcpy(info->fsh_name,"obfs");
	
	return B_NO_ERROR;
}


static int
bfs_write_fs_stat(void *_ns, struct fs_info *info, long mask)
{
	FUNCTION_START(("mask = %ld\n",mask));
	Volume *volume = (Volume *)_ns;
	disk_super_block &superBlock = volume->SuperBlock();
	
	// ToDo: we need a lock for the super_block here...

	status_t status = B_BAD_VALUE;

	if (mask & WFSSTAT_NAME) {
		strncpy(superBlock.name,info->volume_name,sizeof(superBlock.name) - 1);
		superBlock.name[sizeof(superBlock.name) - 1] = '\0';
		// ToDo: write the super block back!

		status = B_OK;
	}
	return status;
}


int 
bfs_initialize(const char *deviceName, void *parms, size_t len)
{
	FUNCTION_START(("deviceName = %s, parameter len = %ld\n",deviceName,len));
	return B_ERROR;
}


int 
bfs_sync(void *ns)
{
	FUNCTION();
	return B_ERROR;
}


//	#pragma mark -


/**	Using vnode id, read in vnode information into fs-specific struct,
 *	and return it in node. the reenter flag tells you if this function
 *	is being called via some other fs routine, so that things like 
 *	double-locking can be avoided.
 */

static int
bfs_read_vnode(void *_ns, vnode_id id, char reenter, void **node)
{
	FUNCTION_START(("vnode_id = %Ld\n",id));
	Volume *volume = (Volume *)_ns;

	if (id < 0 || id > volume->NumBlocks()) {
		FATAL(("inode at %Ld requested!\n",id));
		return B_ERROR;
	}

	Inode *inode = new Inode(volume,id,reenter);
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
bfs_remove_vnode(void *_ns, void *_node, char reenter)
{
	FUNCTION();

	if (_ns == NULL || _node == NULL)
		return B_BAD_VALUE;

	Volume *volume = (Volume *)_ns;
	Inode *inode = (Inode *)_node;

	Transaction transaction(volume,inode->BlockNumber());

	status_t status = inode->SetFileSize(&transaction,0);
	if (status < B_OK)
		return status;

	// ToDo: free attributes, etc.
	// perhaps there should be an implementation of Inode::ShrinkStream() that
	// just frees the data_stream, but doesn't change the inode (since it is
	// freed anyway) - that would make an undelete command possible

	if ((status = volume->Free(&transaction,inode->BlockRun())) == B_OK) {
		transaction.Done();
		delete inode;
	}

	return B_OK;
}


//	#pragma mark -


/**	the walk function just "walks" through a directory looking for the
 *	specified file. It calls get_vnode() on its vnode-id to init it
 *	for the kernel.
 */

static int
bfs_walk(void *_ns, void *_directory, const char *file, char **_resolvedPath, vnode_id *vnid)
{
	FUNCTION_START(("file = %s\n",file));

	if (_ns == NULL || _directory == NULL || file == NULL)
		return B_BAD_VALUE;

	Volume *volume = (Volume *)_ns;
	Inode *directory = (Inode *)_directory;

	// check access permissions
	status_t status = directory->CheckPermissions(X_OK);
	if (status < B_OK)
		RETURN_ERROR(status);

	BPlusTree *tree;
	if (directory->GetTree(&tree) != B_OK)
		RETURN_ERROR(B_BAD_VALUE);

	if ((status = tree->Find((uint8 *)file,(uint16)strlen(file),vnid)) < B_OK)
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
				status = inode->ReadAt(0, (uint8 *)data, &readBytes);
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


int 
bfs_ioctl(void *_ns, void *_node, void *_cookie, int cmd, void *buffer, size_t bufferLength)
{
	FUNCTION_START(("node = %p, cmd = %d, buf = %p, len = %ld\n",_node,cmd,buffer,bufferLength));

	if (_ns == NULL)
		return B_BAD_VALUE;

	Volume *volume = (Volume *)_ns;
	Inode *inode = (Inode *)_node;

	switch (cmd) {
		case IOCTL_FILE_UNCACHED_IO:
			if (inode != NULL)
				PRINT(("trying to make access to inode %lx uncached. Not yet implemented!\n",inode->ID()));
			return B_ERROR;
#ifdef DEBUG
		case 56742:
		{
			// allocate all free blocks and zero them out (a test for the BlockAllocator)!
			BlockAllocator &allocator = volume->Allocator();
			Transaction transaction(volume,0);
			CachedBlock cached(volume);
			block_run run;
			while (allocator.AllocateBlocks(&transaction,8,0,64,1,run) == B_OK) {
				PRINT(("write block_run(%ld, %d, %d)\n",run.allocation_group,run.start,run.length));
				for (int32 i = 0;i < run.length;i++) {
					uint8 *block = cached.SetTo(run);
					if (block != NULL) {
						memset(block,0,volume->BlockSize());
						cached.WriteBack(&transaction);
					}
				}
			}
			return B_OK;
		}
		case 56743:
			dump_super_block(&volume->SuperBlock());
			return B_OK;
		case 56744:
			if (inode != NULL)
				dump_inode(inode->Node());
			return B_OK;
		case 56745:
			if (inode != NULL)
				dump_block((const char *)inode->Node(),volume->BlockSize());
			return B_OK;
#endif
	}
	return B_BAD_VALUE;
}


int 
bfs_setflags(void *ns, void *node, void *cookie, int flags)
{
	FUNCTION_START(("node = %p, flags = %d",node,flags));
	return B_OK;
}


int 
bfs_select(void *ns, void *node, void *cookie, uint8 event, uint32 ref, selectsync *sync)
{
	FUNCTION_START(("event = %d, ref = %lu, sync = %p\n",event,ref,sync));
	notify_select_event(sync, ref);

	return B_OK;
}


int 
bfs_deselect(void *ns, void *node, void *cookie, uint8 event, selectsync *sync)
{
	FUNCTION();
	return B_OK;
}


int 
bfs_fsync(void *ns, void *node)
{
	FUNCTION();
	return B_ERROR;
}


/**	Fills in the stat struct for a node
 */

static int
bfs_read_stat(void *_ns, void *_node, struct stat *st)
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


int 
bfs_write_stat(void *_ns, void *_node, struct stat *stat, long mask)
{
	FUNCTION();

	if (_ns == NULL || _node == NULL || stat == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	Volume *volume = (Volume *)_ns;
	Inode *inode = (Inode *)_node;

	// that may be incorrect here - I don't think we need write access to
	// change most of the stat...
	// we should definitely check a bit more if the new stats are correct and valid...
	
	status_t status = inode->CheckPermissions(W_OK);
	if (status < B_OK)
		RETURN_ERROR(status);

	WriteLocked locked(inode->Lock());
	if (locked.IsLocked() < B_OK)
		RETURN_ERROR(B_ERROR);

	Transaction transaction(volume,volume->ToBlock(inode->BlockRun()));

	bfs_inode *node = inode->Node();
	
	if (mask & WSTAT_MODE) {
		PRINT(("original mode = %ld, stat->st_mode = %ld\n",node->mode,stat->st_mode));
	}

	if (mask & WSTAT_UID)
		node->uid = stat->st_uid;
	if (mask & WSTAT_GID)
		node->gid = stat->st_gid;

	if (mask & WSTAT_SIZE) {
		if (inode->IsDirectory())
			return B_IS_A_DIRECTORY;
		else {
			status = inode->SetFileSize(&transaction,stat->st_size);

			// update "size" index
		}
	}

	if (mask & WSTAT_MTIME) {
		node->last_modified_time = (bigtime_t)stat->st_mtime << INODE_TIME_SHIFT;
		// update "last_modified" index
	}
	if (mask & WSTAT_CRTIME) {
		node->create_time = (bigtime_t)stat->st_crtime << INODE_TIME_SHIFT;
	}

	if ((status = inode->WriteBack(&transaction)) == B_OK)
		transaction.Done();

	notify_listener(B_STAT_CHANGED,volume->ID(),0,0,inode->ID(),NULL);

	return status;
}


int 
bfs_create(void *_ns, void *_directory, const char *name, int omode, int mode, vnode_id *vnid, void **_cookie)
{
	FUNCTION_START(("name = \"%s\", perms = %ld, omode = %ld\n",name,mode,omode));

	if (_ns == NULL || _directory == NULL || _cookie == NULL
		|| name == NULL || *name == '\0')
		RETURN_ERROR(B_BAD_VALUE);

	Volume *volume = (Volume *)_ns;
	Inode *directory = (Inode *)_directory;

	if (!directory->IsDirectory())
		RETURN_ERROR(B_BAD_TYPE);

	status_t status = directory->CheckPermissions(W_OK);
	if (status < B_OK)
		RETURN_ERROR(status);

	// we need to safe the open mode for O_APPEND in bfs_write()
	*(int *)_cookie = omode;

	Transaction transaction(volume,volume->ToBlock(directory->BlockRun()));

	status = Inode::Create(&transaction,directory,name,S_FILE | (mode & S_IUMSK),omode,0,vnid);
	if (status == B_OK)
		transaction.Done();

	return status;
}


int 
bfs_symlink(void *ns, void *dir, const char *name, const char *path)
{
	FUNCTION();
	return B_ERROR;
}


int 
bfs_link(void *ns, void *dir, const char *name, void *node)
{
	FUNCTION_START(("name = \"%s\"\n",name));
	return B_ERROR;
}


int 
bfs_unlink(void *_ns, void *_directory, const char *name)
{
	FUNCTION_START(("name = \"%s\"\n",name));

	if (_ns == NULL || _directory == NULL || name == NULL || *name == '\0')
		return B_BAD_VALUE;

	Volume *volume = (Volume *)_ns;
	Inode *directory = (Inode *)_directory;

	status_t status = directory->CheckPermissions(W_OK);
	if (status < B_OK)
		return status;

	Transaction transaction(volume,directory->BlockNumber());

	if ((status = directory->Remove(&transaction,name)) == B_OK)
		transaction.Done();

	return status;
}


int 
bfs_rename(void *ns, void *oldDir, const char *oldName, void *newDir, const char *newName)
{
	FUNCTION_START(("oldName = \"%s\", newName = \"%s\"\n",oldName,newName));
	return B_ERROR;
}


/**	Opens the file with the specified mode.
 */

static int
bfs_open(void *_ns, void *_node, int omode, void **_cookie)
{
	FUNCTION();

	if (_ns == NULL || _node == NULL || _cookie == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	Volume *volume = (Volume *)_ns;
	Inode *inode = (Inode *)_node;

	status_t status = inode->CheckPermissions(oModeToAccess(omode));
	if (status < B_OK)
		RETURN_ERROR(status);

	// if omode & O_TRUNC, truncate the file
	if (omode & O_TRUNC) {
		Transaction transaction(volume,inode->BlockNumber());
		WriteLocked locked(inode->Lock());

		status_t status = inode->SetFileSize(&transaction,0);
		if (status < B_OK)
			return status;
		
		transaction.Done();
	}

	// we could actually use a cookie to keep track of:
	//	- the last block_run
	//	- the location in the data_stream (indirect, double indirect,
	//	  position in block_run array)
	//
	// This could greatly speed up continuous reads of big files, especially
	// in the indirect block section.

	// we need to safe the open mode for O_APPEND in bfs_write()
	*(int *)_cookie = omode;

	return B_OK;
}


/**	Read a file specified by node, using information in cookie
 *	and at offset specified by pos. read len bytes into buffer buf.
 */

static int
bfs_read(void *_ns, void *_node, void *_cookie, off_t pos, void *buffer, size_t *_length)
{
	//FUNCTION();
	Inode *inode = (Inode *)_node;
	
	if (inode->IsSymLink()) {
		*_length = 0;
		RETURN_ERROR(B_BAD_VALUE);
	}

	ReadLocked locked(inode->Lock());
	return inode->ReadAt(pos,(uint8 *)buffer,_length);
}


int 
bfs_write(void *_ns, void *_node, void *cookie, off_t pos, const void *buffer, size_t *_length)
{
	//FUNCTION();
	Volume *volume = (Volume *)_ns;
	Inode *inode = (Inode *)_node;

	if (inode->IsSymLink()) {
		*_length = 0;
		RETURN_ERROR(B_BAD_VALUE);
	}

	int omode = (int)cookie;

	if (omode & O_APPEND)
		pos = inode->Size();

	WriteLocked locked(inode->Lock());
	if (locked.IsLocked() < B_OK)
		RETURN_ERROR(B_ERROR);

	Transaction transaction(volume,inode->BlockNumber());

	status_t status = inode->WriteAt(&transaction,pos,(const uint8 *)buffer,_length);
	
	if (status == B_OK)
		transaction.Done();

	return status;
}


/**	Do whatever is necessary to close a file, EXCEPT for freeing
 *	the cookie!
 */

static int
bfs_close(void * _ns, void * _node, void * /*cookie*/)
{
	FUNCTION();
	
	if (_ns == NULL || _node == NULL)
		return B_BAD_VALUE;

	// trim the preallocated blocks here
	Inode *inode = (Inode *)_node;

	status_t status = inode->Trim();
	if (status < B_OK)
		FATAL(("Could not trim preallocated blocks!"));

	// ToDo: update indices (size & co.)

	return B_OK;
}


static int
bfs_free_cookie(void * /*ns*/, void * /*node*/, void * /*cookie*/)
{
	return B_OK;
}


/**	Checks access permissions, return B_NOT_ALLOWED if the action
 *	is not allowed.
 */

static int
bfs_access(void *_ns, void *_node, int accessMode)
{
	FUNCTION();
	
	if (_ns == NULL || _node == NULL)
		return B_BAD_VALUE;

	Inode *inode = (Inode *)_node;
	status_t status = inode->CheckPermissions(accessMode);
	if (status < B_OK)
		RETURN_ERROR(status);

	return B_OK;
}


static int
bfs_read_link(void *_ns, void *_node, char *buffer, size_t *bufferSize)
{
	FUNCTION();

	Inode *inode = (Inode *)_node;
	
	if (!inode->IsSymLink())
		RETURN_ERROR(B_BAD_VALUE);

	if (inode->Flags() & INODE_LONG_SYMLINK) {
		if (inode->Size() > *bufferSize)
			return B_NAME_TOO_LONG;

		RETURN_ERROR(inode->ReadAt(0, (uint8 *)buffer, bufferSize));
	}

	size_t numBytes = strlen((char *)&inode->Node()->short_symlink);

	if (numBytes > *bufferSize) {
		FATAL(("link size is greater than buffer size, %ld > %ld\n",numBytes,*bufferSize));
		return B_NAME_TOO_LONG;
	} else
		*bufferSize = numBytes;

	memcpy(buffer, inode->Node()->short_symlink, numBytes);

	return B_OK;
}


//	#pragma mark -
//	Directory functions


int 
bfs_mkdir(void *_ns, void *_directory, const char *name, int mode)
{
	FUNCTION_START(("name = \"%s\", perms = %ld\n",name,mode));

	if (_ns == NULL || _directory == NULL
		|| name == NULL || *name == '\0')
		RETURN_ERROR(B_BAD_VALUE);

	Volume *volume = (Volume *)_ns;
	Inode *directory = (Inode *)_directory;

	if (!directory->IsDirectory())
		RETURN_ERROR(B_BAD_TYPE);

	status_t status = directory->CheckPermissions(W_OK);
	if (status < B_OK)
		RETURN_ERROR(status);

	Transaction transaction(volume,volume->ToBlock(directory->BlockRun()));

	status = Inode::Create(&transaction,directory,name,S_DIRECTORY | (mode & S_IUMSK),0,0);
	if (status == B_OK)
		transaction.Done();

	return status;
}


int 
bfs_rmdir(void *_ns, void *_directory, const char *name)
{
	FUNCTION_START(("name = \"%s\"\n",name));
	
	if (_ns == NULL || _directory == NULL || name == NULL || *name == '\0')
		return B_BAD_VALUE;

	Inode *directory = (Inode *)_directory;

	Transaction transaction(directory->GetVolume(),directory->BlockNumber());

	status_t status = directory->Remove(&transaction,name,true);
	if (status == B_OK)
		transaction.Done();

	return status;
}


/**	creates fs-specific "cookie" struct that keeps track of where
 *	you are at in reading through directory entries in bfs_readdir.
 */

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

		
/** Sets the TreeIterator back to the beginning of the directory
 */

static int
bfs_rewind_dir(void * /*ns*/, void * /*node*/, void *_cookie)
{
	FUNCTION();
	TreeIterator *iterator = (TreeIterator *)_cookie;

	if (iterator == NULL)
		RETURN_ERROR(B_BAD_VALUE);
	
	return iterator->Rewind();
}


static int		
bfs_close_dir(void * /*ns*/, void * /*node*/, void * /*_cookie*/)
{
	FUNCTION();
	// Do whatever you need to to close a directory, but DON'T free the cookie!
	return B_OK;
}


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
bfs_remove_attr(void *_ns, void *_node, const char *name)
{
	FUNCTION_START(("name = \"%s\"\n",name));

	if (_ns == NULL || _node == NULL || name == NULL)
		return B_BAD_VALUE;

	Volume *volume = (Volume *)_ns;
	Inode *inode = (Inode *)_node;

	status_t status = inode->CheckPermissions(W_OK);
	if (status < B_OK)
		return status;

	Transaction transaction(volume,inode->BlockNumber());

	if ((status = inode->RemoveSmallData(&transaction,name)) == B_OK) {
		status = inode->WriteBack(&transaction);
	} else if (status == B_ENTRY_NOT_FOUND) {
		// remove the attribute file if it exists
		Vnode vnode(volume,inode->Attributes());
		Inode *attributes;
		if ((status = vnode.Get(&attributes)) < B_OK)
			return status;

		if ((status = attributes->Remove(&transaction,name)) < B_OK)
			return status;

		if (attributes->IsEmpty()) {
			// remove attribute directory (don't fail if to remove the
			// attribute if that fails)
			if (remove_vnode(volume->ID(),attributes->ID()) == B_OK) {
				// update the inode, so that no one will ever doubt it's deleted :-)
				attributes->Node()->flags |= INODE_DELETED;
				if (attributes->WriteBack(&transaction) == B_OK) {
					inode->Attributes().SetTo(0,0,0);
					inode->WriteBack(&transaction);
				}
			}
		}
	}
	if (status == B_OK)
		transaction.Done();

	RETURN_ERROR(status);
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
	Inode *attribute;
	status_t status = inode->GetAttribute(name,&attribute);
	if (status == B_OK) {
		attrInfo->type = attribute->Node()->type;
		attrInfo->size = attribute->Node()->data.size;

		inode->ReleaseAttribute(attribute);
		return B_OK;
	}

	RETURN_ERROR(status);
}


int
bfs_write_attr(void *_ns, void *_node, const char *name, int type,const void *buffer, size_t *length, off_t pos)
{
	FUNCTION_START(("name = \"%s\"\n",name));

	// writing the name attribute using this function is not allowed
	if (_ns == NULL || _node == NULL || name == NULL || *name == '\0'
		|| (name[0] == FILE_NAME_NAME && name[1] == '\0'))
		RETURN_ERROR(B_BAD_VALUE);

	Volume *volume = (Volume *)_ns;
	Inode *inode = (Inode *)_node;

	Transaction transaction(volume,inode->BlockNumber());

	Inode *attribute = NULL;
	status_t status;
	if (inode->GetAttribute(name,&attribute) < B_OK) {
		// if the attribute doesn't exist yet, try to put it in the small_data
		// section first - if that fails (due to insufficent space), create
		// a real attribute file
		status = inode->AddSmallData(&transaction,name,type,(const uint8 *)buffer,*length);
		if (status == B_DEVICE_FULL) {
			status = inode->CreateAttribute(&transaction,name,type,&attribute);
			if (status < B_OK)
				RETURN_ERROR(status);
		} else if (status == B_OK)
			status = inode->WriteBack(&transaction);
	}

	if (attribute != NULL) {
		status = attribute->WriteAt(&transaction,pos,(const uint8 *)buffer,length);
		inode->ReleaseAttribute(attribute);
	}

	if (status == B_OK)
		transaction.Done();

	return status;
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
	Inode *attribute;
	status_t status = inode->GetAttribute(name,&attribute);
	if (status == B_OK) {
		status = attribute->ReadAt(pos,(uint8 *)buffer,_length);
		inode->ReleaseAttribute(attribute);
		RETURN_ERROR(status);
	}

	RETURN_ERROR(status);
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
	// we can just use the directory traversal functions (and since it is
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
bfs_open_query(void *_ns,const char *queryString,ulong flags,port_id port,long token,void **cookie)
{
	FUNCTION();
	if (_ns == NULL || queryString == NULL || cookie == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	PRINT(("query = \"%s\", flags = %lu, port_id = %ld, token = %ld\n",queryString,flags,port,token));

	Volume *volume = (Volume *)_ns;
	
	Expression *expression = new Expression((char *)queryString);
	if (expression == NULL)
		RETURN_ERROR(B_NO_MEMORY);
	
	if (expression->InitCheck() < B_OK) {
		FATAL(("Could not parse query, stopped at: \"%s\"\n",expression->Position()));
		delete expression;
		RETURN_ERROR(B_BAD_VALUE);
	}

	Query *query = new Query(volume,expression);
	if (query == NULL) {
		delete expression;
		RETURN_ERROR(B_NO_MEMORY);
	}
	*cookie = (void *)query;
	
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

	Query *query = (Query *)cookie;
	Expression *expression = query->GetExpression();
	delete query;
	delete expression;

	return B_OK;
}


int
bfs_read_query(void */*ns*/,void *cookie,long *num,struct dirent *dirent,size_t bufferSize)
{
	FUNCTION();
	Query *query = (Query *)cookie;
	if (query == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	status_t status = query->GetNextEntry(dirent,bufferSize);
	if (status == B_OK)
		*num = 1;
	else if (status == B_ENTRY_NOT_FOUND)
		*num = 0;
	else
		return status;

	return B_OK;
}

