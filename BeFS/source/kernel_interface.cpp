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
extern "C" {
	#define _IMPEXP_KERNEL
	#include <KernelExport.h>
	#include <fsproto.h>
	#include <lock.h>
	#include <cache.h>
}

#ifdef USER
#	define dprintf printf
#endif


/*  Start of fundamental (read-only) required functions */
extern "C" {
	static int fs_mount(nspace_id nsid, const char *device, ulong flags,
						void *parms, size_t len, void **data, vnode_id *vnid);
	static int fs_unmount(void *_ns);
	
	static int fs_walk(void *_ns, void *_base, const char *file,
						char **newpath, vnode_id *vnid);
	
	static int fs_read_vnode(void *_ns, vnode_id vnid, char r, void **node);
	static int fs_write_vnode(void *_ns, void *_node, char r);
	static int fs_rstat(void *_ns, void *_node, struct stat *st);
	static int fs_open(void *_ns, void *_node, int omode, void **cookie);
	static int fs_read(void *_ns, void *_node, void *cookie, off_t pos,
						void *buf, size_t *len);
	// fs_free_cookie - free cookie for file created in open.
	static int fs_free_cookie(void *ns, void *node, void *cookie);
	static int fs_close(void *ns, void *node, void *cookie);
	
	// fs_access - 		checks permissions for access.
	static int fs_access(void *_ns, void *_node, int mode);
	
	// fs_opendir - 	creates fs-specific "cookie" struct that can tell where
	//					we are at in the directory list.
	static int fs_opendir(void* _ns, void* _node, void** cookie);
	// fs_readdir - 	read 1 or more dirents, keep state in cookie, return
	//					0 when no more entries.
	static int fs_readdir(void *_ns, void *_node, void *cookie,
						long *num, struct dirent *buf, size_t bufsize);
	// fs_rewinddir -	set cookie to represent beginning of directory, so
	//					later fs_readdir calls start at beginning.
	static int fs_rewinddir(void *_ns, void *_node, void *cookie);
	// fs_closedir -	Do whatever you need to to close a directory (sometimes
	//					nothing), but DON'T free the cookie!
	static int fs_closedir(void *_ns, void *_node, void *cookie);
	// fs_fee_dircookie - Free the fs-specific cookie struct
	static int fs_free_dircookie(void *_ns, void *_node, void *cookie);
	
	// fs_rfsstat - Fill in fs_info struct for device.
	static int fs_rfsstat(void *_ns, struct fs_info *);
	
	// fs_readlink - Read in the name of a symbolic link.
	static int fs_readlink(void *_ns, void *_node, char *buf, size_t *bufsize);

	/* End of fundamental (read-only) required functions. */

#if 0
static int		fs_remove_vnode(void *ns, void *node, char r);
static int		fs_secure_vnode(void *ns, void *node);
static int		fs_create(void *ns, void *dir, const char *name,
					int perms, int omode, vnode_id *vnid, void **cookie);
static int		fs_mkdir(void *ns, void *dir, const char *name, int perms);
static int		fs_unlink(void *ns, void *dir, const char *name);
static int		fs_rmdir(void *ns, void *dir, const char *name);
static int		fs_wstat(void *ns, void *node, struct stat *st, long mask);
static int		fs_write(void *ns, void *node, void *cookie, off_t pos,
						const void *buf, size_t *len);
static int		fs_ioctl(void *ns, void *node, void *cookie, int cmd,
						void *buf, size_t len);
static int		fs_wfsstat(void *ns, struct fs_info *);
static int		fs_sync(void *ns);
static int     	fs_initialize(const char *devname, void *parms, size_t len);
#endif 
}	// extern "C"


/* vnode_ops struct. Fill this in to tell the kernel how to call
	functions in your driver.
*/

vnode_ops fs_entry =  {
	&fs_read_vnode,						// read_vnode func ptr
	&fs_write_vnode,					// write_vnode func ptr
	NULL, 								// remove_vnode func ptr
	NULL,								// secure_vnode func ptr
	&fs_walk,							// walk func ptr
	&fs_access,							// access func ptr
	NULL, 								// create func ptr
	NULL, 								// mkdir func ptr
	NULL,
	NULL,
	NULL,
	NULL, 								// unlink func ptr
	NULL, 								// rmdir func ptr
	&fs_readlink,						// readlink func ptr
	&fs_opendir,						// opendir func ptr
	&fs_closedir,						// closedir func ptr
	&fs_free_dircookie,					// free_dircookie func ptr
	&fs_rewinddir,						// rewinddir func ptr
	&fs_readdir,						// readdir func ptr
	&fs_open,							// open file func ptr
	&fs_close,							// close file func ptr
	&fs_free_cookie,					// free cookie func ptr
	&fs_read,							// read file func ptr
	NULL, 								// write file func ptr
	NULL, /* readv */
	NULL, /* writev */
	NULL,								// ioctl func ptr
	NULL,								// setflags file func ptr
	&fs_rstat,							// rstat func ptr
	NULL, 								// wstat func ptr
	NULL,
	NULL,								// initialize func ptr
	&fs_mount,							// mount func ptr
	&fs_unmount,						// unmount func ptr
	NULL,								// sync func ptr
	&fs_rfsstat,						// rfsstat func ptr
	NULL,								// wfsstat func ptr
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

#define BFS_IO_SIZE 65536
int32	api_version = B_CUR_FS_API_VERSION;


static int
fs_mount(nspace_id nsid, const char *device, ulong flags, void * parms,
		size_t len, void **data, vnode_id *rootID)
{
	dprintf("fs_mount()\n");

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
fs_unmount(void *ns)
{
	dprintf("fs_unmount()\n");
	Volume* volume = (Volume *)ns;

	int status = volume->Unmount();
	delete volume;

	//remove_cached_device_blocks(ns->fd, 0);
	//result = close(ns->fd);
	//free(ns);

	return status;
}

// fs_rfsstat - Fill in fs_info struct for device.
static int
fs_rfsstat(void *_ns, struct fs_info *info)
{
	dprintf("fs_rfsstat()\n");

	Volume *volume = (Volume *)_ns;
	
	//dprintf("fs_rfsstat - ENTER\n");
	
	// Fill in device id.
	//info->dev = volume->Device();
	
	// Root vnode ID
	//info->root = ISO_ROOTNODE_ID;
	
	// File system flags.
	info->flags = B_FS_IS_PERSISTENT | B_FS_IS_READONLY;

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

/* fs_walk - the walk function just "walks" through a directory looking for
			the specified file. When you find it, call get_vnode on its vnid to init
			it for the kernel.
*/

static int
fs_walk(void *_ns, void *_directory, const char *file, char **newpath, vnode_id *vnid)
{
	Volume *volume = (Volume *)_ns;
	dprintf("fs_walk(vol_id = %ld,file = %s)\n",volume->ID(),file);

//	if (!strcmp(".",file))
//	{
//		*vnid = volume->ToVnode(volume->Root());
//
//		void *vnode = NULL;
//		if (get_vnode(volume->ID(),*vnid,(void **)&vnode) != 0)
//			return B_ENTRY_NOT_FOUND;
//
//		return B_NO_ERROR;
//	}

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

/* fs_read_vnode - Using vnode id, read in vnode information into fs-specific struct,
			and return it in node. the reenter flag tells you if this function
			is being called via some other fs routine, so that things like 
			double-locking can be avoided.
*/

static int
fs_read_vnode(void *_ns, vnode_id vnid, char reenter, void **node)
{
	Volume *volume = (Volume *)_ns;

	dprintf("fs_read_vnode(vnode_id = %Ld)\n",vnid);
	
	Inode *inode = new Inode(volume,vnid,reenter);
	if (inode->InitCheck() == B_OK) {
		*node = (void *)inode;
		return B_OK;
	}
	delete inode;

	return -1;
}

static int
fs_write_vnode(void *ns, void *_node, char reenter)
{
	Inode *inode = (Inode *)_node;
	
	delete inode;

	dprintf("fs_write_vnode(node = %p)\n",_node);
	return B_NO_ERROR;
}

// fs_rstat - fill in stat struct

static int
fs_rstat(void *_ns, void *_node, struct stat *st)
{
	Volume *volume = (Volume *)_ns;
	Inode *inode = (Inode *)_node;
	bfs_inode *node = inode->Node();

	dprintf("fs_rstat()\n");
	
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

// fs_open - Create a vnode cookie, if necessary, to use when
// 				reading/writing a file

static int
fs_open(void *_ns, void *_node, int omode, void **cookie)
{
	dprintf("fs_open()\n");
	return 0;
}

// fs_read
// Read a file specified by node, using information in cookie
// and at offset specified by pos. read len bytes into buffer buf.

static int
fs_read(void *_ns, void *_node, void *_cookie, off_t pos, void *buffer, size_t *_length)
{
	Inode *inode = (Inode *)_node;
	
	if (inode->IsSymLink()) {
		*_length = 0;
		return B_BAD_VALUE;
	}
	
	dprintf("fs_read()\n");
	return inode->ReadAt(pos,buffer,_length);
}

// fs_close - Do whatever is necessary to close a file, EXCEPT for freeing
//				the cookie!

static int
fs_close(void *ns, void *node, void *cookie)
{
	dprintf("fs_close()\n");
	#pragma unused (ns)
	#pragma unused (node)
	#pragma unused (cookie)
	return 0;
}

static int
fs_free_cookie(void *ns, void *node, void *cookie)
{
	#pragma unused (ns)
	#pragma unused (node)
	#pragma unused (cookie)
	return 0;
}

// fs_access - checks permissions for access.

static int
fs_access(void *ns, void *node, int mode)
{
	dprintf("fs_access()\n");
	return 0;
}

static int
fs_readlink(void *_ns, void *_node, char *buf, size_t *bufsize)
{
	dprintf("fs_readlink()\n");
	return 0;
}

// fs_opendir - creates fs-specific "cookie" struct that keeps track of where
//					you are at in reading through directory entries in fs_readdir.

static int
fs_opendir(void *_ns, void *_node, void **cookie)
{
	dprintf("fs_opendir()\n");
	
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

// fs_readdir - read 1 or more dirents, keep state in cookie, return
//					0 when no more entries.

static int
fs_readdir(void *_ns, void *_node, void *_cookie, long *num, 
			struct dirent *dirent, size_t bufsize)
{
	TreeIterator *iterator = (TreeIterator *)_cookie;
	if (iterator == NULL)
		return B_BAD_VALUE;

	dprintf("fs_readdir() xxx\n");

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
			
// fs_rewinddir - set cookie to represent beginning of directory, so
//					later fs_readdir calls start at beginning.

static int
fs_rewinddir(void * /*ns*/, void * /*node*/, void *_cookie)
{
	TreeIterator *iterator = (TreeIterator *)_cookie;
	
	if (iterator == NULL)
		return B_BAD_VALUE;
	
	iterator->Rewind();
	return 0;
}

// fs_closedir - Do whatever you need to to close a directory (sometimes
//					nothing), but DON'T free the cookie!

static int		
fs_closedir(void * /*ns*/, void * /*node*/, void * /*_cookie*/)
{
	return 0;
}

// fs_free_dircookie - Free the fs-specific cookie struct

static int
fs_free_dircookie(void *ns, void *node, void *_cookie)
{
	TreeIterator *iterator = (TreeIterator *)_cookie;
	
	if (iterator == NULL)
		return B_BAD_VALUE;

	delete iterator;
	return 0;
}

