/* Inode - inode access functions
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include "Debug.h"
#include "cpp.h"
#include "Inode.h"
#include "BPlusTree.h"

#include <string.h>


Inode::Inode(Volume *volume,vnode_id id,uint8 reenter)
	: CachedBlock(volume,volume->VnodeToBlock(id)),
	fTree(NULL)
{
}


Inode::~Inode()
{
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
		FATAL(("bfs: inode at block %Ld corrupt!\n",fBlockNumber));
		return B_ERROR;
	}
	return B_OK;
}


/**	Iterates through the small_data section of an inode.
 *	To start at the beginning of this section, you let smallData
 *	point to NULL, like:
 *		small_data *data = NULL;
 *		while (inode->GetNextSmallData(&data) { ... }
 *
 *	This function is reentrant and don't allocate any memory;
 *	you can safely stop calling it at any point.
 */

status_t
Inode::GetNextSmallData(small_data **smallData) const
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


small_data *
Inode::FindSmallData(const char *name) const
{
	small_data *smallData = NULL;
	while (GetNextSmallData(&smallData) == B_OK) {
		if (!strcmp(smallData->Name(),name))
			return smallData;
	}
	return NULL;
}


Inode *
Inode::GetAttribute(const char *name)
{
	// does this inode even have attributes?
	if (Attributes().IsZero())
		return NULL;

	Inode *attributes;
	if (get_vnode(fVolume->ID(),fVolume->ToVnode(Attributes()),(void **)&attributes) != 0
		|| attributes == NULL) {
		FATAL(("bfs: get_vnode() failed in Inode::GetAttribute(name = \"%s\")\n",name));
		return NULL;
	}

	BPlusTree *tree;
	if (attributes->GetTree(&tree) == B_OK) {
		vnode_id id;
		if (tree->Find((uint8 *)name,(uint16)strlen(name),&id) == B_OK) {
			Inode *attribute;
			if (get_vnode(fVolume->ID(),id,(void **)&attribute) == B_OK)
				return attribute;
		}
	}
	
	put_vnode(fVolume->ID(),attributes->ID());
	return NULL;
}


void
Inode::ReleaseAttribute(Inode *attribute)
{
	if (attribute == NULL)
		return;
	
	put_vnode(fVolume->ID(),attribute->ID());
	put_vnode(fVolume->ID(),fVolume->ToVnode(Attributes()));
}


/**	Gives the caller direct access to the b+tree for a given directory.
 *	The tree is created on demand, but lasts until the inode is
 *	deleted.
 */

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


/** Finds the block_run where "pos" is located in the data_stream of
 *	the inode.
 *	If successful, "offset" will then be set to the file offset
 *	of the block_run returned; so "pos - offset" is for the block_run
 *	what "pos" is for the whole stream.
 */

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
	else if (pos >= Node()->data.size) {
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
		put_vnode(fAttributes->GetVolume()->ID(),fAttributes->ID());

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
AttributeIterator::GetNext(char *name, size_t *_length, uint32 *_type, vnode_id *_id)
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
			*_type = fCurrentSmallData->type;
			*_length = fCurrentSmallData->name_size;
			*_id = (vnode_id)((uint8 *)fCurrentSmallData - (uint8 *)fInode->Node()->small_data_start);

			return B_OK;
		}
	}
	
	// read attributes out of the attribute directory

	if (fInode->Attributes().IsZero())
		return B_ENTRY_NOT_FOUND;

	Volume *volume = fInode->GetVolume();

	if (fAttributes == NULL) {
		if (get_vnode(volume->ID(),volume->ToVnode(fInode->Attributes()),(void **)&fAttributes) != 0
			|| fAttributes == NULL) {
			FATAL(("bfs: get_vnode() failed in AttributeIterator::GetNext(vnode_id = %Ld,name = \"%s\")\n",fInode->ID(),name));
			return B_ENTRY_NOT_FOUND;
		}
		
		BPlusTree *tree;
		if (fAttributes->GetTree(&tree) < B_OK
			|| (fIterator = new TreeIterator(tree)) == NULL) {
			FATAL(("bfs: could not get tree in AttributeIterator::GetNext(vnode_id = %Ld,name = \"%s\")\n",fInode->ID(),name));
			return B_ENTRY_NOT_FOUND;
		}
	}

	block_run run;
	uint16 length;
	vnode_id id;
	status_t status = fIterator->GetNextEntry(name,&length,B_FILE_NAME_LENGTH,&id);
	if (status < B_OK)
		return status;

	Inode *attribute;
	status = get_vnode(volume->ID(),id,(void **)&attribute);
	if (status == B_OK && attribute != NULL) {
		*_type = attribute->Node()->type;
		*_length = attribute->Node()->data.size;
		*_id = id;
	} else if (status == B_OK) {
		FATAL(("bfs: get_vnode() failed in AttributeIterator::GetNext(vnode_id = %Ld,name = \"%s\")\n",fInode->ID(),name));
		status = B_ERROR;
	}

	put_vnode(volume->ID(),id);
	
	return status;
}

