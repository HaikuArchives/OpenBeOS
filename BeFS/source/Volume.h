#ifndef VOLUME_H
#define VOLUME_H
/* Volume - BFS super block, mounting, etc.
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
*/


#include <KernelExport.h>

extern "C" {
	#ifndef _IMPEXP_KERNEL
	#	define _IMPEXP_KERNEL
	#endif
	#include "fsproto.h"
}

//#include "Bitmap.h"
#include "bfs.h"

class Inode;


class Volume
{
	public:
		Volume(nspace_id id);
		~Volume();

		status_t			Mount(const char *device,uint32 flags);
		status_t			Unmount();

		bool				IsValidSuperBlock();

		block_run			Root() const { return fSuperBlock.root_dir; }
		Inode				*RootNode() const { return fRootNode; }
		block_run			Indices() const { return fSuperBlock.indices; }
		Inode				*IndicesNode() const { return fIndicesNode; }
		int					Device() const { return fDevice; }

		nspace_id			ID() const { return fID; }
		const char			*Name() const { return fSuperBlock.name; }

		off_t				NumBlocks() const { return fSuperBlock.num_blocks; }
		uint32				BlockSize() const { return fSuperBlock.block_size; }
		uint32				BlockShift() const { return fSuperBlock.block_shift; }
		uint32				InodeSize() const { return fSuperBlock.inode_size; }
		uint32				AllocationGroups() const { return fSuperBlock.num_ags; }
		uint32				AllocationGroupShift() const { return fSuperBlock.ag_shift; }

		off_t				ToOffset(block_run run) const { return ToBlock(run) << fSuperBlock.block_shift; }
		off_t				ToBlock(block_run run) const { return ((((off_t)run.allocation_group) << fSuperBlock.ag_shift) | (off_t)run.start); }
		
		off_t				ToVnode(block_run run) const { return ToBlock(run); }
		off_t				ToVnode(off_t block) const { return block; }
		off_t				VnodeToBlock(vnode_id id) const { return (off_t)id; }

	protected:
		nspace_id			fID;
		int					fDevice;
		disk_super_block	fSuperBlock;
		
		Inode				*fRootNode;
		Inode				*fIndicesNode;
};

#endif	/* VOLUME_H */
