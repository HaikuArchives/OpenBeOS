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

// What makes the code complicated is the fact that we are not just reading
// in the whole bitmap and operate on that in memory - e.g. a 13 GB partition
// with a block size of 2048 bytes already has a 800kB bitmap, and the size
// of partitions will grow even more - so that's not an option.
// Instead we are reading in every block when it's used - since an allocation
// group can span several blocks in the block bitmap, the AllocationBlock
// class is there to make handling those easier.

// The current implementation is very basic and will be heavily optimized
// in the future.
// Furthermore, the allocation policies used here (when they will be in place)
// should have some real world tests.


class AllocationBlock : public CachedBlock {
	public:
		AllocationBlock(Volume *volume);
		
		void Allocate(uint16 start,uint16 numBlocks = 0xffff);
		void Free(uint16 start,uint16 numBlocks = 0xffff);
		inline bool IsUsed(uint16 block);

		status_t SetTo(AllocationGroup &group,uint16 block);

		int32 NumBlockBits() const { return fNumBits; }

	private:
		int32 fNumBits;
};


class AllocationGroup {
	public:
		AllocationGroup();

		void AddFreeRange(int32 start,int32 blocks);
		bool IsFull() const { return fIsFull; }

		int32 fNumBits;
		int32 fStart;
		int32 fFirstFree,fLargest,fLargestFirst;
		bool fIsFull;
};


AllocationBlock::AllocationBlock(Volume *volume)
	: CachedBlock(volume)
{
}


status_t 
AllocationBlock::SetTo(AllocationGroup &group, uint16 block)
{
	// 8 bits for every block
	fNumBits = group.fNumBits - ((fVolume->BlockSize() << 3) * block);

	return CachedBlock::SetTo(group.fStart + block) != NULL ? B_OK : B_ERROR;
}


bool 
AllocationBlock::IsUsed(uint16 block)
{
	if (block > fNumBits)
		return true;
	return ((uint32 *)fBlock)[block >> 5] & (1UL << (block % 32));
}


void
AllocationBlock::Allocate(uint16 start,uint16 numBlocks)
{
	start = start % fNumBits;
	if (numBlocks == 0xffff) {
		// allocate all blocks after "start"
		numBlocks = fNumBits - start;
	} else if (start + numBlocks > fNumBits) {
		FATAL(("should allocate more blocks than there are in a block!\n"));
		numBlocks = fNumBits - start;
	}

	int32 block = start >> 5;

	while (numBlocks > 0) {
		uint32 mask = 0;
		for (int32 i = start % 32;i < 32 && numBlocks;i++,numBlocks--)
			mask |= 1UL << (i % 32);

		((uint32 *)fBlock)[block++] |= mask;
		start = 0;
	}
}


void
AllocationBlock::Free(uint16 start,uint16 numBlocks)
{
	start = start % fNumBits;
	if (numBlocks == 0xffff) {
		// free all blocks after "start"
		numBlocks = fNumBits - start;
	} else if (start + numBlocks > fNumBits) {
		FATAL(("should free more blocks than there are in a block!\n"));
		numBlocks = fNumBits - start;
	}

	int32 block = start >> 5;

	while (numBlocks > 0) {
		uint32 mask = 0;
		for (int32 i = start % 32;i < 32 && numBlocks;i++,numBlocks--)
			mask |= 1UL << (i % 32);

		((uint32 *)fBlock)[block++] &= ~mask;
		start = 0;
	}
}


//	#pragma mark -


AllocationGroup::AllocationGroup()
	:
	fFirstFree(-1),
	fLargest(-1),
	fLargestFirst(-1),
	fIsFull(true)
{
}


void 
AllocationGroup::AddFreeRange(int32 start, int32 blocks)
{
	D(if (blocks > 512)
		PRINT(("range of %ld blocks starting at %ld\n",blocks,start)));

	if (fFirstFree == -1)
		fFirstFree = start;

	if (fLargest < blocks) {
		fLargest = blocks;
		fLargestFirst = start;
	}
	if (blocks)
		fIsFull = false;
}


//	#pragma mark -


BlockAllocator::BlockAllocator(Volume *volume)
	:
	fVolume(volume),
	fGroups(NULL)
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
		int32 start,range = 0;
		int32 size = groups[i].fNumBits,num = 0;

		for (int32 k = 0;k < (size >> 2);k++) {
			for (int32 j = 0;j < 32 && num < size;j++,num++) {
				if (buffer[k] & (1UL << j)) {
					if (range > 0) {
						groups[i].AddFreeRange(start,range);
						range = 0;
					}
				} else if (range++ == 0)
					start = num;
			}
		}
		if (range)
			groups[i].AddFreeRange(start,range);

		offset += blocks;
	}
	free(buffer);

	return B_OK;
}


status_t
BlockAllocator::AllocateBlocks(Transaction *transaction,int32 group,uint16 start,uint16 maximum,uint16 minimum, block_run &run)
{
	AllocationBlock cached(fVolume);
	Locker lock(fLock);

	// the first scan through all allocation groups will look for the
	// wanted maximum of blocks, the second scan will just look to
	// satisfy the minimal requirement
	uint16 numBlocks = maximum;

	for (int32 i = 0;i < fNumGroups * 2;i++,group++,start = 0) {
		group = group % fNumGroups;

		if (start >= fGroups[group].fNumBits || fGroups[group].IsFull())
			continue;

//		printf("group %ld: num = %ld, start = %ld, firstFree = %ld, largest = %ld, largestFirst = %ld, isFull = %s\n",
//				group,fGroups[group].fNumBits,fGroups[group].fStart,fGroups[group].fFirstFree,
//				fGroups[group].fLargest,fGroups[group].fLargestFirst,fGroups[group].fIsFull);

		if (i >= fNumGroups) {
			// if the minimum is the same as the maximum, it's not necessary to
			// search for in the allocation groups a second time
			if (maximum == minimum)
				return B_DEVICE_FULL;

			numBlocks = minimum;
		}

		// the wanted maximum is smaller than the largest free block in the group
		// or already smaller than the minimum
		// -> disabled because it's currently not maintained after the first allocation
		//if (numBlocks > fGroups[group].fLargest)
		//	continue;

		if (start < fGroups[group].fFirstFree)
			start = fGroups[group].fFirstFree;

		// there may be more than one block per allocation group - and
		// we iterate through it to find a place for the allocation.
		// (one allocation can't exceed one allocation group)

		uint32 block = start / fGroups[group].fNumBits;
		int32 range = 0, rangeStart = 0,rangeBlock = 0;

		for (;block < fBlocksPerGroup;block++) {
			if (cached.SetTo(fGroups[group],block) < B_OK)
				RETURN_ERROR(B_ERROR);

			// find a block large enough to hold the allocation
			for (int32 bit = start % cached.NumBlockBits();bit < cached.NumBlockBits();bit++) {
				if (!cached.IsUsed(bit)) {
					if (range == 0) {
						// start new range
						rangeStart = block * cached.NumBlockBits() + bit;
						rangeBlock = block;
					}

					// have we found a range large enough to hold numBlocks?
					if (++range >= maximum)
						break;
				} else if (i >= fNumGroups && range >= minimum) {
					// we have found a block larger than the required minimum (second pass)
					break;
				} else {
					// end of a range
					range = 0;
				}
			}

			// if we found a suitable block, mark the blocks as in use, and write
			// the updated block bitmap back to disk
			if (range >= numBlocks) {
				// adjust allocation size
				if (numBlocks < maximum)
					numBlocks = range;
				// update first free block in group (doesn't have to be free)
				if (rangeStart == fGroups[group].fFirstFree) {
					fGroups[group].fFirstFree = rangeStart + numBlocks;
					if (fGroups[group].fFirstFree >= fGroups[group].fNumBits)
						fGroups[group].fIsFull = true;
				}

				if (block != rangeBlock) {
					// allocate the part that's in the current block
					cached.Allocate(0,(rangeStart + numBlocks) % cached.NumBlockBits());
					if (cached.WriteBack(transaction) < B_OK)
						RETURN_ERROR(B_ERROR);

					// set the blocks in the previous block
					if (cached.SetTo(fGroups[group],block - 1) < B_OK)
						cached.Allocate(rangeStart);
					else
						RETURN_ERROR(B_ERROR);
				} else {
					// just allocate the bits in the current block
					cached.Allocate(rangeStart,numBlocks);
				}
				run.allocation_group = group;
				run.start = rangeStart;
				run.length = numBlocks;
				
				// ToDo: the super block has to be written back to disk!
				fVolume->SuperBlock().used_blocks += numBlocks;

				return cached.WriteBack(transaction);
			}

			// start from the beginning of the next block
			start = 0;
		}
	}
	return B_DEVICE_FULL;
}


status_t 
BlockAllocator::AllocateForInode(Transaction *transaction,const block_run *parent, mode_t type, block_run &run)
{
	// apply some allocation policies here (AllocateBlocks() will break them
	// if necessary) - we will start with those described in Dominic Giampaolo's
	// "Practical File System Design", and see how good they work
	
	// files are going in the same allocation group as its parent, sub-directories
	// will be inserted 8 allocation groups after the one of the parent
	uint16 group = parent->allocation_group;
	if ((type & (S_DIRECTORY | S_INDEX_DIR | S_ATTR_DIR)) == S_DIRECTORY)
		group += 8;

	return AllocateBlocks(transaction,group,0,1,1,run);
}


status_t 
BlockAllocator::Allocate(Transaction *transaction,const Inode *inode, off_t numBlocks, block_run &run, uint16 minimum)
{
	if (numBlocks <= 0)
		return B_ERROR;

	// one block_run can't hold more data than it is in one allocation group
	if (numBlocks > fGroups[0].fNumBits)
		numBlocks = fGroups[0].fNumBits;

	// apply some allocation policies here (AllocateBlocks() will break them
	// if necessary)
	uint16 group = inode->BlockRun().allocation_group;
	uint16 start = 0;

	// are there already allocated blocks? (then just allocate near the last)
	if (inode->Size() > 0) {
		data_stream *data = &inode->Node()->data;
		// we currently don't care for when the data stream is
		// already grown into the indirect ranges
		if (data->max_double_indirect_range == 0
			&& data->max_indirect_range == 0) {
			int32 last = 0;
			for (;last < NUM_DIRECT_BLOCKS - 1;last++)
				if (data->direct[last + 1].IsZero())
					break;
			
			group = data->direct[last].allocation_group;
			start = data->direct[last].start + data->direct[last].length;
		}
	} else if (inode->IsDirectory()) {
		// directory data will go in the same allocation group as the inode is in
		// but after the inode data
		start = inode->BlockRun().start;
	} else {
		// file data will start in the next allocation group
		group = inode->BlockRun().allocation_group + 1;
	}

	return AllocateBlocks(transaction,group,start,numBlocks,minimum,run);
}


status_t 
BlockAllocator::Free(Transaction *transaction,block_run &run)
{
	if (run.allocation_group >= fNumGroups)
		return B_ENTRY_NOT_FOUND;
	if (run.length == 0)
		return B_BAD_VALUE;

	AllocationBlock cached(fVolume);
	Locker lock(fLock);

	uint16 group = run.allocation_group;
	uint16 start = run.start;
	uint32 block = run.start / fGroups[group].fNumBits;
	uint16 length = run.length;

	if (fGroups[group].fFirstFree > start)
		fGroups[group].fFirstFree = start;

	for (;block < fBlocksPerGroup;block++) {
		if (cached.SetTo(fGroups[group],block) < B_OK)
			RETURN_ERROR(B_IO_ERROR);

		uint16 freeLength = length;
		if (start + length > cached.NumBlockBits())
			freeLength = cached.NumBlockBits() - start;

		cached.Free(start,freeLength);

		if (cached.WriteBack(transaction) < B_OK)
			return B_IO_ERROR;

		length -= freeLength;
		if (length <= 0)
			break;

		start = 0;
	}

	// ToDo: the super block has to be written back to disk!
	fVolume->SuperBlock().used_blocks -= run.length;

	return B_OK;
}

