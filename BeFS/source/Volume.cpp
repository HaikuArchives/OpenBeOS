/* Volume - BFS super block, mounting, etc.
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include "Debug.h"
#include "cpp.h"
#include "Volume.h"
#include "Inode.h"

#include <KernelExport.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>


Volume::Volume(nspace_id id)
	:
	fID(id),
	fBlockAllocator(this),
	fFlags(0)
{
}


Volume::~Volume()
{
}


bool
Volume::IsValidSuperBlock()
{
	if (fSuperBlock.magic1 != (int32)SUPER_BLOCK_MAGIC1
		|| fSuperBlock.magic2 != (int32)SUPER_BLOCK_MAGIC2
		|| fSuperBlock.magic3 != (int32)SUPER_BLOCK_MAGIC3
		|| (int32)fSuperBlock.block_size != fSuperBlock.inode_size
		|| fSuperBlock.fs_byte_order != SUPER_BLOCK_FS_LENDIAN
		|| (1UL << fSuperBlock.block_shift) != fSuperBlock.block_size
		|| fSuperBlock.num_ags < 1
		|| fSuperBlock.ag_shift < 1
		|| fSuperBlock.blocks_per_ag < 1
		|| fSuperBlock.num_blocks < 10
		|| fSuperBlock.used_blocks > fSuperBlock.num_blocks
		|| fSuperBlock.num_ags != divide_roundup(fSuperBlock.num_blocks,1L << fSuperBlock.ag_shift))
		return false;

	return true;
}


status_t
Volume::Mount(const char *deviceName,uint32 flags)
{
	if (flags & B_MOUNT_READ_ONLY)
		fFlags |= VOLUME_READ_ONLY;

	fDevice = open(deviceName,flags & B_MOUNT_READ_ONLY ? O_RDONLY : O_RDWR);
	
	// if we couldn't open the device, try read-only (don't rely on a specific error code)
	if (fDevice < B_OK && (flags & B_MOUNT_READ_ONLY) == 0) {
		fDevice = open(deviceName,O_RDONLY);
		fFlags |= VOLUME_READ_ONLY;
	}

	if (fDevice < B_OK)
		RETURN_ERROR(fDevice);

	// check if it's a regular file, and if so, disable the cache for the
	// underlaying file system
	struct stat stat;
	if (fstat(fDevice,&stat) < 0)
		RETURN_ERROR(B_ERROR);

	if (stat.st_mode & S_FILE && ioctl(fDevice,IOCTL_FILE_UNCACHED_IO,NULL) < 0) {
		// don't mount if the cache couldn't be disabled
		RETURN_ERROR(B_ERROR);
	}

	// read the super block
	char buffer[1024];
	if (read_pos(fDevice,0,buffer,sizeof(buffer)) != sizeof(buffer))
		return B_IO_ERROR;

	status_t status = B_OK;

	// Note: that does work only for x86, for PowerPC, the super block
	// is located at offset 0!
	memcpy(&fSuperBlock,buffer + 512,sizeof(disk_super_block));

	if (IsValidSuperBlock()) {
		if (init_cache_for_device(fDevice, NumBlocks()) == B_OK) {
			if (fBlockAllocator.Initialize() == B_OK) {
				fRootNode = new Inode(this,ToVnode(Root()));
	
				if (fRootNode && fRootNode->InitCheck() == B_OK) {
					if (new_vnode(fID,ToVnode(Root()),(void *)fRootNode) == B_OK) {
						// try to get indices root dir
						
						// question: why doesn't get_vnode() work here??
						// answer: we have not yet backpropagated the pointer to the
						// volume in bfs_mount(), so bfs_read_vnode() can't get it.
						// But it's not needed to that anyway.

						fIndicesNode = new Inode(this,ToVnode(Indices()));
						if (fIndicesNode == NULL
							|| fIndicesNode->InitCheck() < B_OK
							|| !fIndicesNode->IsDirectory()) {
							INFORM(("bfs: volume doesn't have indices!\n"));
	
							if (fIndicesNode) {
								// in this case, BFS should be mounted as read-only
								fIndicesNode = NULL;
							}
						}
	
						// all went fine
						return B_OK;
					} else
						status = B_NO_MEMORY;
				} else
					status = B_BAD_VALUE;
	
				FATAL(("could not create root node: new_vnode() failed!\n"));
			} else {
				status = B_NO_MEMORY;
				FATAL(("could not initialize block bitmap allocator!\n"));
			}

			remove_cached_device_blocks(fDevice,NO_WRITES);
		} else {
			FATAL(("could not initialize cache!\n"));
			status = B_IO_ERROR;
		}
		FATAL(("invalid super block!\n"));
	}
	else
		status = B_BAD_VALUE;

	close(fDevice);

	return status;
}


status_t
Volume::Unmount()
{
	delete fIndicesNode;

	remove_cached_device_blocks(fDevice,NO_WRITES);
	close(fDevice);

	return 0;
}

