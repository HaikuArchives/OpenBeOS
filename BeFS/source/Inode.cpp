/* Inode - inode access functions
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
*/


#include "cpp.h"
#include "Inode.h"
#include "BPlusTree.h"

#include <string.h>


Inode::Inode(Volume *volume,vnode_id id,uint8 reenter)
	: CachedBlock(volume,volume->VnodeToBlock(id)),
	fTree(NULL)
{
	dprintf("new inode!\n");
}


Inode::~Inode()
{
	dprintf("delete inode!\n");
	delete fTree;
}


status_t 
Inode::InitCheck()
{
	if (!Node())
		return B_IO_ERROR;

	// test inode magic and flags
	if (Node()->magic1 != INODE_MAGIC1
		|| !(Node()->flags & INODE_IN_USE)
		|| Node()->inode_num.length != 1
		// matches inode size?
		|| Node()->inode_size != fVolume->InodeSize()
		// parent resides on disk?
		|| Node()->parent.allocation_group > fVolume->AllocationGroups()
		|| Node()->parent.allocation_group < 0
		|| Node()->parent.start > (1L << fVolume->AllocationGroupShift())
		|| Node()->parent.length != 1
		// attributes, too?
		|| Node()->attributes.allocation_group > fVolume->AllocationGroups()
		|| Node()->attributes.allocation_group < 0
		|| Node()->attributes.start > (1L << fVolume->AllocationGroupShift())) {
		dprintf("bfs: inode at block %Ld corrupt!\n",fBlockNumber);
		return B_ERROR;
	}
	return B_OK;
}


status_t
Inode::GetNextSmallData(small_data **smallData)
{
	if (!Node())
		return B_ERROR;

	small_data *data = *smallData;

	// begin from the start?
	if (data == NULL)
		data = Node()->small_data_start;
	else
		data = data->Next();

	// is already last item?
	if (data->IsLast(Node()))
		return B_ENTRY_NOT_FOUND;

	*smallData = data;

	return B_OK;
}


status_t
Inode::GetTree(BPlusTree **tree)
{
	if (fTree) {
		*tree = fTree;
		return B_OK;
	}

	if (IsDirectory()) {
		fTree = new BPlusTree(this);
		if (!fTree)
			return B_NO_MEMORY;

		*tree = fTree;
		return fTree->InitCheck();
	}
	return B_BAD_VALUE;
}


status_t
Inode::FindBlockRun(off_t pos,block_run &run,off_t &offset)
{
	// Inode::ReadAt() does already does this
	//if (pos > Node()->data.size)
	//	return B_ENTRY_NOT_FOUND;

	// find matching block run

	if (Node()->data.max_direct_range > 0 && pos >= Node()->data.max_direct_range) {
		if (Node()->data.max_double_indirect_range > 0 && pos >= Node()->data.max_indirect_range) {
			// access to double indirect blocks

			//printf("find double indirect block: %ld,%d!\n",Node()->data.double_indirect.allocation_group,Node()->data.double_indirect.start);

			CachedBlock cached(fVolume,Node()->data.indirect);
			block_run *indirect = (block_run *)cached.Block();
			if (indirect == NULL)
				return B_ERROR;

			off_t start = pos - Node()->data.max_indirect_range;
			int32 indirectSize = (16 << fVolume->BlockShift()) * (fVolume->BlockSize() / sizeof(block_run));
			int32 directSize = 4 << fVolume->BlockShift();
			int32 index = start / indirectSize;
			
			//printf("\tstart = %Ld, indirectSize = %ld, directSize = %ld, index = %ld\n",start,indirectSize,directSize,index);
			//printf("\tlook for indirect block at %ld,%d\n",indirect[index].allocation_group,indirect[index].start);

			indirect = (block_run *)cached.SetTo(indirect[index]);
			if (indirect == NULL)
				return B_ERROR;
			
			int32 current = (start % indirectSize) / directSize;

			run = indirect[current];
			offset = Node()->data.max_indirect_range + (index * indirectSize) + (current * directSize);
			//printf("\tfCurrent = %ld, fRunFileOffset = %Ld, fRunBlockEnd = %Ld, fRun = %ld,%d\n",fCurrent,fRunFileOffset,fRunBlockEnd,fRun.allocation_group,fRun.start);
		} else {
			// access to indirect blocks

			CachedBlock cached(fVolume,Node()->data.indirect);
			block_run *indirect = (block_run *)cached.Block();
			if (indirect == NULL)
				return B_ERROR;

			int32 indirectRuns = (Node()->data.indirect.length << fVolume->BlockShift()) / sizeof(block_run);

			off_t runBlockEnd = Node()->data.max_direct_range;
			int32 current = -1;

			while (++current < indirectRuns) {
				if (indirect[current].IsZero())
					break;

				runBlockEnd += indirect[current].length << fVolume->BlockShift();
				if (runBlockEnd > pos) {
					run = indirect[current];
					offset = runBlockEnd - (run.length << fVolume->BlockShift());
					//printf("reading from indirect block: %ld,%d\n",fRun.allocation_group,fRun.start);
					//printf("### indirect-run[%ld] = (%ld,%d,%d), offset = %Ld\n",fCurrent,fRun.allocation_group,fRun.start,fRun.length,fRunFileOffset);
					return B_OK;
				}
			}
			return B_ERROR;
		}
	} else {
		// access from direct blocks

		off_t runBlockEnd = 0LL;
		int32 current = -1;

		while (++current < NUM_DIRECT_BLOCKS) {
			if (Node()->data.direct[current].IsZero())
				break;

			runBlockEnd += Node()->data.direct[current].length << fVolume->BlockShift();
			if (runBlockEnd > pos) {
				run = Node()->data.direct[current];
				offset = runBlockEnd - (run.length << fVolume->BlockShift());
				//printf("### run[%ld] = (%ld,%d,%d), offset = %Ld\n",fCurrent,fRun.allocation_group,fRun.start,fRun.length,fRunFileOffset);
				return B_OK;
			}
		}
		return B_ERROR;
	}	
	return B_OK;
}


status_t 
Inode::ReadAt(off_t pos, void *buffer, size_t *_length)
{
	// set/check boundaries for pos/length

	if (pos < 0)
		pos = 0;
	else if (pos > Node()->data.size) {
		*_length = 0;
		return B_NO_ERROR;
	}

	size_t length = *_length;

	if (pos + length > Node()->data.size)
		length = Node()->data.size - pos;

	block_run run;
	off_t offset;
	if (FindBlockRun(pos,run,offset) < B_OK) {
		*_length = 0;
		return B_BAD_VALUE;
	}
	
	uint32 bytesRead = 0;
	uint32 block_size = fVolume->BlockSize();
	uint32 block_shift = fVolume->BlockShift();
	uint8 *block;

	// the first block_run we read could not be aligned to the block_size boundary
	// (read partial block at the beginning)

	// pos % block_size == (pos - offset) % block_size, offset % block_size == 0
	if (pos % block_size != 0) {
		run.start += (pos - offset) / block_size;
		run.length -= (pos - offset) / block_size;

		CachedBlock cached(fVolume,run);
		if ((block = cached.Block()) == NULL) {
			*_length = 0;
			return B_BAD_VALUE;
		}
		
		bytesRead = block_size - (pos % block_size);
		if (length < bytesRead)
			bytesRead = length;

		memcpy(buffer,block + (pos % block_size),bytesRead);
		pos += bytesRead;
		
		length -= bytesRead;
		if (length == 0) {
			*_length = bytesRead;
			return B_OK;
		}
		
		if (FindBlockRun(pos,run,offset) < B_OK) {
			*_length = bytesRead;
			return B_BAD_VALUE;
		}
	}
	
	// the first block_run is already filled it at this point
	// read the following complete blocks using cached_read(),
	// the last partial block is read using the CachedBlock class

	bool partial = false;

	while (length > 0) {
		// offset is the offset to the current pos in the block_run
		run.start += (pos - offset) >> block_shift;
		run.length -= (pos - offset) >> block_shift;

		if ((run.length << block_shift) > length) {
			if (length < block_size) {
				CachedBlock cached(fVolume,run);
				if ((block = cached.Block()) == NULL) {
					*_length = bytesRead;
					return B_BAD_VALUE;
				}
				memcpy((uint8 *)buffer + bytesRead,block,length);
				bytesRead += length;
				break;
			}
			run.length = length >> block_shift;
			partial = true;
		}
		
		if (cached_read(fVolume->Device(),fVolume->ToBlock(run),(uint8 *)buffer + bytesRead,
						run.length,block_size) != B_OK) {
			*_length = bytesRead;
			return B_BAD_VALUE;
		}

		int32 bytes = run.length << block_shift;
		length -= bytes;
		bytesRead += bytes;
		if (length == 0)
			break;

		pos += bytes;
		
		if (partial) {
			// if the last block was read only partially, point block_run
			// to the remaining part
			run.start += run.length;
			run.length = 1;
			offset = pos;
		} else if (FindBlockRun(pos,run,offset) < B_OK) {
			*_length = bytesRead;
			return B_BAD_VALUE;
		}
	}
	
	*_length = bytesRead;
	return B_NO_ERROR;
}


status_t 
Inode::WriteAt(off_t pos,void *buffer,size_t *length)
{
	// not yet implemented
	// should be pretty similar to ReadAt(), as long as the space is
	// already reserved in the inode's data_stream

	return B_ERROR;
}


//	#pragma mark -


AttributeIterator::AttributeIterator(Inode *inode)
	:
	fCurrentSmallData(NULL),
	fInode(inode),
	fAttributes(NULL),
	fIterator(NULL),
	fBuffer(NULL)
{
}


AttributeIterator::~AttributeIterator()
{
	if (fAttributes)
		put_vnode(fAttributes->GetVolume()->ID(),fAttributes->VnodeID());

	delete fIterator;
}


status_t 
AttributeIterator::Rewind()
{
	fCurrentSmallData = NULL;

	if (fIterator != NULL)
		fIterator->Rewind();

	return B_OK;
}


status_t 
AttributeIterator::GetNext(char *name, uint32 *type, void **data, size_t *length)
{
	// read attributes out of the small data section

	if (fCurrentSmallData == NULL || !fCurrentSmallData->IsLast(fInode->Node())) {
		if (fCurrentSmallData == NULL)
			fCurrentSmallData = fInode->Node()->small_data_start;
		else
			fCurrentSmallData = fCurrentSmallData->Next();

		// skip name attribute
		if (!fCurrentSmallData->IsLast(fInode->Node())
			&& fCurrentSmallData->name_size == FILE_NAME_NAME_LENGTH
			&& *fCurrentSmallData->Name() == FILE_NAME_NAME)
			fCurrentSmallData = fCurrentSmallData->Next();

		if (!fCurrentSmallData->IsLast(fInode->Node())) {
			strncpy(name,fCurrentSmallData->Name(),B_FILE_NAME_LENGTH);
			*type = fCurrentSmallData->type;
			*data = fCurrentSmallData->Data();
			*length = fCurrentSmallData->data_size;
			
			return B_OK;
		}
	}
	
	// read attributes out of the attribute directory

	if (fInode->Attributes().IsZero())
		return B_ENTRY_NOT_FOUND;

	if (fAttributes == NULL) {
		Volume *volume = fInode->GetVolume();
		if (get_vnode(volume->ID(),volume->ToVnode(fInode->Attributes()),(void **)&fAttributes) != 0
			|| fAttributes == NULL)
			return B_ENTRY_NOT_FOUND;
		
		BPlusTree *tree;
		if (fAttributes->GetTree(&tree) < B_OK
			|| (fIterator = new TreeIterator(tree)) == NULL)
			return B_ENTRY_NOT_FOUND;
	}

//	block_run run;
//	status = fAttributes->GetNextEntry(name,&run);
//	if (status < B_OK)
//	{
//		free(fAttributeBuffer);  fAttributeBuffer = NULL;
//		return status;
//	}
//	
//	Attribute *attribute = (Attribute *)Inode::Factory(fDisk,run);
//	if (attribute == NULL || attribute->InitCheck() < B_OK)
//		return B_IO_ERROR;
//	
//	*type = attribute->Type();
//
//	void *buffer = realloc(fAttributeBuffer,attribute->Size());
//	if (buffer == NULL)
//	{
//		free(fAttributeBuffer);  fAttributeBuffer = NULL;
//		delete attribute;
//		return B_NO_MEMORY;
//	}
//	fAttributeBuffer = buffer;
//	
//	ssize_t size =  attribute->Read(fAttributeBuffer,attribute->Size());
//	delete attribute;
//	
//	*length = size;
//	*data = fAttributeBuffer;
//
//	return size < B_OK ? size : B_OK;
}

