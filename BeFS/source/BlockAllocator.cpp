/* BlockAllocator - block bitmap handling and allocation policies
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include "cpp.h"
#include "Debug.h"
#include "BlockAllocator.h"
#include "Volume.h"
#include "Inode.h"

#ifdef USER
#	define spawn_kernel_thread spawn_thread
#endif

// Things the BlockAllocator should do:

// - find a range of blocks of a certain size nearby a specific position
// - allocating a unsharp range of blocks for pre-allocation
// - free blocks
// - know how to deal with each allocation, special handling for directories,
//   files, symlinks, etc. (type sensitive allocation policies)

// The current implementation is very basic and will be heavily optimized
// in the future.
// Furthermore, the allocation policies used here (when they will be in place)
// should have some real world tests.


class AllocationGroup {
	public:
		AllocationGroup();

		status_t GetFree(uint16 start,uint16 numBlocks,uint16 minimum,block_run &run);

		int32 fNumBits;
		int32 fStart;
		bool fIsFull;
};


AllocationGroup::AllocationGroup()
	:
	fIsFull(true)
{
}


status_t 
AllocationGroup::GetFree(uint16 start, uint16 numBlocks, uint16 minimum, block_run &run)
{
	return B_DEVICE_FULL;
}


//	#pragma mark -


BlockAllocator::BlockAllocator(Volume *volume)
	:
	fVolume(volume)
{
}


BlockAllocator::~BlockAllocator()
{
	delete[] fGroups;
}


status_t 
BlockAllocator::Initialize()
{
	if (fLock.InitCheck() < B_OK)
		return B_ERROR;

	fNumGroups = fVolume->AllocationGroups();
	fBlocksPerGroup = fVolume->SuperBlock().blocks_per_ag;
	fGroups = new AllocationGroup[fNumGroups];
	if (fGroups == NULL)
		return B_NO_MEMORY;
	
	thread_id id = spawn_kernel_thread((thread_func)BlockAllocator::initialize,"bfs block allocator",B_LOW_PRIORITY,(void *)this);
	if (id < B_OK)
		return initialize(this);

	return resume_thread(id);
}


status_t 
BlockAllocator::initialize(BlockAllocator *allocator)
{
	Locker lock(allocator->fLock);

	Volume *volume = allocator->fVolume;
	uint32 blocks = allocator->fBlocksPerGroup;
	uint32 numBits =  8 * blocks * volume->BlockSize();

	uint32 *buffer = (uint32 *)malloc(numBits >> 3);
	if (buffer == NULL)
		RETURN_ERROR(B_NO_MEMORY);

	AllocationGroup *groups = allocator->fGroups;
	off_t offset = 1;
	int32 num = allocator->fNumGroups;

	for (int32 i = 0;i < num;i++) {
		if (cached_read(volume->Device(),offset,buffer,blocks,volume->BlockSize()) < B_OK)
			break;

		// the last allocation group may contain less blocks than the others
		groups[i].fNumBits = i == num - 1 ? allocator->fVolume->NumBlocks() - i * numBits : numBits;
		groups[i].fStart = offset;

		// finds all free ranges in this allocation group
		// this is currently not used at all!
		// It will be used to optimize access to the free blocks
		int32 maxRange = 0;
		off_t start;
		int32 range = 0;
		int32 size = groups[i].fNumBits,num = 0;

		for (int32 k = 0;k < (size >> 2);k++)
		{
			for (int32 j = 0;j < 32 && num < size;j++,num++)
			{
				if (buffer[k] & (1UL << j))
				{
					if (range > 0)
					{
						if (range > 512)
							PRINT(("ag %ld: range of %ld blocks starting at %Ld\n",k,range,start));
						if (maxRange < range)
							maxRange = range;
						range = 0;
					}
				}
				else
				{
					if (range == 0)
						start = num;
					range++;
				}
			}
		}

		offset += blocks;
	}
	free(buffer);

	return B_OK;
}


status_t
BlockAllocator::AllocateBlocks(int32 group,uint16 start,uint16 numBlocks,uint16 minimum, block_run &run)
{
	for (int32 i = 0;i < fNumGroups;i++) {
		if (fGroups[group].GetFree(start,numBlocks,minimum,run) == B_OK)
			return B_OK;

		group = (group + 1) % fNumGroups;
	}
	return B_DEVICE_FULL;
}


status_t 
BlockAllocator::AllocateForInode(Inode *parent, mode_t type, block_run &run)
{
	// apply some allocation policies here (AllocateBlocks() will break them
	// if necessary) - we will start with those described in Dominic Giampaolo's
	// "Practical File System Design", and see how good they work
	return AllocateBlocks(parent->BlockRun().allocation_group,0,1,1,run);
}


status_t 
BlockAllocator::Allocate(Inode *inode, uint16 numBlocks, block_run &run, uint16 minimum)
{
	// apply some allocation policies here (AllocateBlocks() will break them
	// if necessary)
	return AllocateBlocks(inode->BlockRun().allocation_group,0,numBlocks,minimum,run);
}


status_t 
BlockAllocator::Free(block_run &run)
{
	return B_OK;
}

