#ifndef BLOCK_ALLOCATOR_H
#define BLOCK_ALLOCATOR_H
/* BlockAllocator - block bitmap handling and allocation policies
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include <Lock.h>


class AllocationGroup;
class Volume;
class Inode;
struct disk_super_block;
struct block_run;


class BlockAllocator {
	public:
		BlockAllocator(Volume *volume);
		~BlockAllocator();

		status_t Initialize();

		status_t AllocateForInode(Inode *parent,mode_t type,block_run &run);
		status_t Allocate(Inode *inode,uint16 numBlocks,block_run &run,uint16 minimum = 1);
		status_t Free(block_run &run);

	private:
		status_t AllocateBlocks(int32 group, uint16 start, uint16 numBlocks, uint16 minimum, block_run &run);
		static status_t initialize(BlockAllocator *);

		Volume			*fVolume;
		Benaphore		fLock;
		AllocationGroup	*fGroups;
		int32			fNumGroups,fBlocksPerGroup;
};

#endif	/* BLOCK_ALLOCATOR_H */
