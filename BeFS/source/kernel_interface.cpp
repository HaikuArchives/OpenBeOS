/* kernel_interface - file system interface to BeOS' vnode layer
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
*/


#include "cpp.h"
#include "Volume.h"
#include "Inode.h"
#include "BPlusTree.h"

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
						long *num, struct dirent *buf, size_t bufsize);
	static int bfs_rewind_dir(void *_ns, void *_node, void *cookie);
	static int bfs_close_dir(void *_ns, void *_node, void *cookie);
	static int bfs_free_dir_cookie(void *_ns, void *_node, void *cookie);
	
	// bfs_rfsstat - Fill in bfs_info struct for device.
	static int bfs_rfsstat(void *_ns, struct fs_info *);
	
	// bfs_read_link - Read in the name of a symbolic link.
	static int bfs_read_link(void *_ns, void *_node, char *buf, size_t *bufsize);

	// attribute support
	static int bfs_open_attrdir(void *ns, void *node, void **cookie);
	static int bfs_close_attrdir(void *ns, void *node, void *cookie);
	static int bfs_free_attrdir_cookie(void *ns, void *node, void *cookie);
	static int bfs_rewind_attrdir(void *ns, void *node, void *cookie);
	static int bfs_read_attrdir(void *ns, void *node, void *cookie, long *num,
					struct dirent *buf, size_t bufsize);
	static int bfs_remove_attr(void *ns, void *node, const char *name);
	static int bfs_rename_attr(void *ns, void *node, const char *oldname,
					const char *newname);
	static int bfs_stat_attr(void *ns, void *node, const char *name,
					struct attr_info *buf);
	static int bfs_write_attr(void *ns, void *node, const char *name, int type,
					const void *buf, size_t *len, off_t pos);
	static int bfs_read_attr(void *ns, void *node, const char *name, int type,
					void *buf, size_t *len, off_t pos);

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

vnode_ops bfs_entry =  {
	&bfs_read_vnode,			// read_vnode
	&bfs_release_vnode,			// write_vnode
	NULL, 						// remove_vnode
	NULL,						// secure_vnode
	&bfs_walk,					// walk
	&bfs_access,				// access
	NULL, 						// create
	NULL, 						// mkdir
	NULL,
	NULL,
	NULL,
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
	NULL,						// ioctl
	NULL,						// setflags file
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
	NULL,						// open index dir
	NULL,						// close index dir
	NULL,						// free index dir cookie
	NULL,						// rewind index dir
	NULL,						// read index dir
	NULL,						// create index
	NULL,						// remove index
	NULL,						// rename index
	NULL,						// stat index
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
	NULL,						// open query
	NULL,						// close query
	NULL,						// free query cookie
	NULL						// read query
};

#define BFS_IO_SIZE 65536
int32	api_version = B_CUR_FS_API_VERSION;


static int
bfs_mount(nspace_id nsid, const char *device, ulong flags, void * parms,
		size_t len, void **data, vnode_id *rootID)
{
	dprintf("bfs_mount()\n");

//#ifdef DEBUG
//	load_driver_symbols("obfs");
//#endif

	Volume *volume = new Volume(nsid);

	status_t status;
	if ((status = volume->Mount(device,flags)) == B_OK) {
		*data = volume;
		*rootID = volume->ToVnode(volume->Root());
		dprintf("yeah! %Ld\n",*rootID);
	}
	else
		delete volume;

	return status;
}


static int
bfs_unmount(void *ns)
{
	dprintf("bfs_unmount()\n");
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
	dprintf("bfs_rfsstat()\n");

	Volume *volume = (Volume *)_ns;
	
	//dprintf("bfs_rfsstat - ENTER\n");
	
	// Fill in device id.
	//info->dev = volume->Device();
	
	// Root vnode ID
	//info->root = ISO_ROOTNODE_ID;
	
	// File system flags.
	info->flags = B_FS_IS_PERSISTENT | B_FS_IS_READONLY | B_FS_HAS_ATTR | B_FS_HAS_MIME;

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

// bfs_walk - the walk function just "walks" through a directory looking for
//				the specified file. When you find it, call get_vnode on its vnid to init
//				it for the kernel.

static int
bfs_walk(void *_ns, void *_directory, const char *file, char **newpath, vnode_id *vnid)
{
	Volume *volume = (Volume *)_ns;
	dprintf("bfs_walk(file = %s)\n",file);

	Inode *directory = (Inode *)_directory;
	BPlusTree *tree;
	if (directory->GetTree(&tree) != B_OK)
		return B_BAD_VALUE;

	//off_t offset;
	status_t status = tree->Find((uint8 *)file,(uint16)strlen(file),vnid);
	if (status != B_OK)
		return status;

	Inode *inode;
	if (get_vnode(volume->ID(),*vnid,(void **)&inode) != 0)
		return B_ENTRY_NOT_FOUND;

	// if "found" is a symlink, we have to do some more stuff here...

	return B_OK;
}

// bfs_read_vnode - Using vnode id, read in vnode information into fs-specific struct,
//				and return it in node. the reenter flag tells you if this function
//				is being called via some other fs routine, so that things like 
//				double-locking can be avoided.

static int
bfs_read_vnode(void *_ns, vnode_id vnid, char reenter, void **node)
{
	Volume *volume = (Volume *)_ns;

	dprintf("bfs_read_vnode(vnode_id = %Ld)\n",vnid);
	
	Inode *inode = new Inode(volume,vnid,reenter);
	if (inode->InitCheck() == B_OK) {
		*node = (void *)inode;
		return B_OK;
	}
	delete inode;

	return -1;
}


static int
bfs_release_vnode(void *ns, void *_node, char reenter)
{
	dprintf("bfs_release_vnode(node = %p)\n",_node);
	Inode *inode = (Inode *)_node;
	
	delete inode;

	return B_NO_ERROR;
}

// bfs_rstat - fill in stat struct

static int
bfs_rstat(void *_ns, void *_node, struct stat *st)
{
	dprintf("bfs_rstat()\n");

	Volume *volume = (Volume *)_ns;
	Inode *inode = (Inode *)_node;
	bfs_inode *node = inode->Node();

	st->st_dev = volume->ID();
	st->st_ino = inode->VnodeID();
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
	dprintf("bfs_open()\n");
	return 0;
}

// bfs_read - Read a file specified by node, using information in cookie
//				and at offset specified by pos. read len bytes into buffer buf.

static int
bfs_read(void *_ns, void *_node, void *_cookie, off_t pos, void *buffer, size_t *_length)
{
	Inode *inode = (Inode *)_node;
	
	if (inode->IsSymLink()) {
		*_length = 0;
		return B_BAD_VALUE;
	}
	
	dprintf("bfs_read()\n");
	return inode->ReadAt(pos,buffer,_length);
}

// bfs_close - Do whatever is necessary to close a file, EXCEPT for freeing
//				the cookie!

static int
bfs_close(void * /*ns*/, void * /*node*/, void * /*cookie*/)
{
	dprintf("bfs_close()\n");
	return 0;
}


static int
bfs_free_cookie(void * /*ns*/, void * /*node*/, void * /*cookie*/)
{
	return 0;
}

// bfs_access - checks permissions for access.

static int
bfs_access(void *ns, void *node, int mode)
{
	dprintf("bfs_access()\n");
	return 0;
}


static int
bfs_read_link(void *_ns, void *_node, char *buf, size_t *bufsize)
{
	dprintf("bfs_readlink()\n");
	return 0;
}

// bfs_opendir - creates fs-specific "cookie" struct that keeps track of where
//					you are at in reading through directory entries in bfs_readdir.

static int
bfs_open_dir(void *_ns, void *_node, void **cookie)
{
	dprintf("bfs_opendir()\n");
	
	Inode *inode = (Inode *)_node;
	
	if (!inode->IsDirectory())
		return B_BAD_VALUE;

	BPlusTree *tree;
	if (inode->GetTree(&tree) != B_OK)
		return B_BAD_VALUE;

	TreeIterator *iterator = new TreeIterator(tree);
	if (iterator == NULL)
		return B_NO_MEMORY;

	*cookie = iterator;
	return 0;
}

// bfs_readdir - read 1 or more dirents, keep state in cookie, return
//					0 when no more entries.

static int
bfs_read_dir(void *_ns, void *_node, void *_cookie, long *num, 
			struct dirent *dirent, size_t bufsize)
{
	TreeIterator *iterator = (TreeIterator *)_cookie;
	if (iterator == NULL)
		return B_BAD_VALUE;

	dprintf("bfs_readdir() xxx\n");

	uint16 length;
	vnode_id id;
	status_t status = iterator->GetNextEntry(dirent->d_name,&length,bufsize,&id);
	if (status == B_ENTRY_NOT_FOUND) {
		*num = 0;
		return B_OK;
	} else if (status != B_OK)
		return status;


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
	TreeIterator *iterator = (TreeIterator *)_cookie;
	
	if (iterator == NULL)
		return B_BAD_VALUE;
	
	iterator->Rewind();
	return 0;
}

// bfs_closedir - Do whatever you need to to close a directory (sometimes
//					nothing), but DON'T free the cookie!

static int		
bfs_close_dir(void * /*ns*/, void * /*node*/, void * /*_cookie*/)
{
	return 0;
}

// bfs_free_dircookie - Free the fs-specific cookie struct

static int
bfs_free_dir_cookie(void *ns, void *node, void *_cookie)
{
	TreeIterator *iterator = (TreeIterator *)_cookie;
	
	if (iterator == NULL)
		return B_BAD_VALUE;

	delete iterator;
	return 0;
}


//	#pragma mark -


int 
bfs_open_attrdir(void *ns, void *node, void **cookie)
{
	dprintf("bfs_open_attrdir()\n");
	return B_ERROR;
}


int
bfs_close_attrdir(void *ns, void *node, void *cookie)
{
	dprintf("bfs_close_attrdir()\n");
	return B_ERROR;
}


int
bfs_free_attrdir_cookie(void *ns, void *node, void *cookie)
{
	dprintf("bfs_free_attrdir_cookie()\n");
	return B_ERROR;
}


int
bfs_rewind_attrdir(void *ns, void *node, void *cookie)
{
	dprintf("bfs_rewind_attrdir()\n");
	return B_ERROR;
}


int 
bfs_read_attrdir(void *ns, void *node, void *cookie, long *num, struct dirent *buf, size_t bufsize)
{
	dprintf("bfs_read_attrdir()\n");
	return B_ERROR;
}


int
bfs_remove_attr(void *ns, void *node, const char *name)
{
	dprintf("bfs_remove_attr(name = \"%s\")\n",name);
	return B_ENTRY_NOT_FOUND;
}


int
bfs_rename_attr(void *ns, void *node, const char *oldname,const char *newname)
{
	dprintf("bfs_rename_attr(name = \"%s\",to = \"%s\"\n)",oldname,newname);
	return B_ENTRY_NOT_FOUND;
}


int
bfs_stat_attr(void *ns, void *node, const char *name,struct attr_info *buf)
{
	dprintf("bfs_stat_attr(name = \"%s\")\n",name);
	return B_ENTRY_NOT_FOUND;
}


int
bfs_write_attr(void *ns, void *node, const char *name, int type,const void *buf, size_t *len, off_t pos)
{
	dprintf("bfs_write_attr(name = \"%s\")\n",name);
	return B_ERROR;
}


int
bfs_read_attr(void *ns, void *node, const char *name, int type,void *buf, size_t *len, off_t pos)
{
	dprintf("bfs_read_attr(name = \"%s\")\n",name);
	return B_ENTRY_NOT_FOUND;
}


