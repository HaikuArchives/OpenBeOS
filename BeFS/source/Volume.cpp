/* Volume - BFS super block, mounting, etc-
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
*/


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
	fID(id)
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
	// for now, just open the device read-only
	fDevice = open(deviceName,O_RDONLY);

	status_t status = fDevice;
	if (status < B_OK)
		return status;

	char buffer[1024];
		
	if (read_pos(fDevice,0,buffer,sizeof(buffer)) != sizeof(buffer))
		return B_IO_ERROR;

	// Note: that does work only for x86!
	memcpy(&fSuperBlock,buffer + 512,sizeof(disk_super_block));

	if (IsValidSuperBlock()) {
		if (init_cache_for_device(fDevice, NumBlocks()) == B_OK) {
			fRootNode = new Inode(this,ToVnode(Root()));

			if (fRootNode && fRootNode->InitCheck() == B_OK) {
				if (new_vnode(fID,ToVnode(Root()),(void *)fRootNode) == B_OK) {
					// try to get indices root dir
					
					// question: why doesn't get_vnode() work here??
					// it returns an error, and bfs_read_vnode is never called...
					fIndicesNode = new Inode(this,ToVnode(Indices()));
					if (fIndicesNode == NULL
						|| fIndicesNode->InitCheck() < B_OK
						|| !fIndicesNode->IsDirectory()) {
						dprintf("bfs: volume doesn't have indices!\n");

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

			dprintf("could not create root node: new_vnode() failed!\n");

			remove_cached_device_blocks(fDevice,NO_WRITES);
		} else {
			dprintf("could not initialize cache!\n");
			status = B_IO_ERROR;
		}
	}
	else
		status = B_BAD_VALUE;

	close(fDevice);

	return status;
}


status_t
Volume::Unmount()
{
	//if (fIndicesNode)
	//	put_vnode(fID,ToVnode(Indices()));
	delete fIndicesNode;

	remove_cached_device_blocks(fDevice,NO_WRITES);
	close(fDevice);

	return 0;
}

