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
	fTree(NULL),
	fLock("bfs inode")
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
		RETURN_ERROR(B_IO_ERROR);

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
		FATAL(("inode at block %Ld corrupt!\n",fBlockNumber));
		RETURN_ERROR(B_BAD_DATA);
	}
	
	// Add some tests to check the integrity of the other stuff here,
	// especially for the data_stream!

	// it's more important to know that the inode is corrupt
	// so we check for the lock not until here
	return fLock.InitCheck();
}


status_t 
Inode::CheckPermissions(int accessMode) const
{
	uid_t user = geteuid();
	gid_t group = getegid();

	// you never have write access to a read-only volume
	if (accessMode & W_OK && fVolume->IsReadOnly())
		return B_READ_ONLY_DEVICE;

	// root users always have full access (but they can't execute anything)
	if (user == 0 && !((accessMode & X_OK) && (Mode() & S_IXUSR) == 0))
		return B_OK;

	// shift mode bits, to check directly against accessMode
	mode_t mode = Mode();
	if (user == Node()->uid)
		mode >>= 6;
	else if (group == Node()->gid)
		mode >>= 3;

	if (accessMode & ~(mode & S_IRWXO))
		return B_NOT_ALLOWED;

	return B_OK;
}


//	#pragma mark -


status_t
Inode::MakeSpaceForSmallData(Transaction *transaction,const char *name,uint32 bytes)
{
	while (bytes > 0) {
		small_data *item = Node()->small_data_start,*max = NULL;
		while (!item->IsLast(Node())) {
			// should not remove that one
			if (*item->Name() == FILE_NAME_NAME)
				continue;
			
			if (max == NULL || max->Size() < item->Size())
				max = item;
			
			// remove the first one large enough to free the needed amount of bytes
			if (bytes < item->Size())
				break;
		}
		
		if (item->IsLast(Node()) && item->Size() < bytes)
			return B_ERROR;

		bytes -= max->Size();
		
		// ToDo: implement me!
		// -> move the attribute to a real attribute file

		RemoveSmallData(NULL,max);
	}
	return B_OK;
}


/**	Removes the given attribute from the small_data section.
 *	Note that you need to write back the inode yourself after having called
 *	that method.
 */

status_t
Inode::RemoveSmallData(const char *name,small_data *item)
{
	if (item == NULL) {
		if (name == NULL)
			return B_BAD_VALUE;

		small_data *item = Node()->small_data_start;
		while (!item->IsLast(Node()) || !strcmp(item->Name(),name))
			item = item->Next();
	
		if (strcmp(item->Name(),name))
			return B_ENTRY_NOT_FOUND;
	}

	if (!item->IsLast(Node())) {
		// find the last attribute
		small_data *last = item;
		while (!last->IsLast(Node()))
			last = last->Next();

		small_data *next = item->Next();
		memmove(item,next,(uint8 *)last - (uint8 *)next + last->Size());
		// move the "last" one to its new location
		last = (small_data *)((uint8 *)last - ((uint8 *)next - (uint8 *)item));

		// correctly terminate the small_data section if needed
		if (!last->IsLast(Node())) {
			last = last->Next();
			memset(last,0,sizeof(small_data));
		}
	} else
		memset(item,0,item->Size());

	return B_OK;
}


/**	Try to place the given attribute in the small_data section - if the
 *	new attribute is too big to fit in that section, it returns B_DEVICE_FULL.
 *	In that case, the attribute should be written to a real attribute file;
 *	if the attribute was already part of the small_data section, but the new
 *	one wouldn't fit, the old one is automatically removed from the small_data
 *	section.
 *	Note that you need to write back the inode yourself after having called that
 *	method - it's a bad API decision that it needs a transaction but enforces you
 *	to write back the inode automatically, but it's just more efficient in most
 *	cases...
 */

status_t
Inode::AddSmallData(Transaction *transaction,const char *name,uint32 type,const uint8 *data,uint32 length,bool force)
{
	if (name == NULL || data == NULL || type == 0)
		return B_BAD_VALUE;

	// reject any requests that can't fit into the small_data section
	uint32 nameLength = strlen(name);
	uint32 spaceNeeded = sizeof(small_data) + nameLength + 3 + length + 1;
	if (spaceNeeded > fVolume->InodeSize() - sizeof(bfs_inode))
		return B_DEVICE_FULL;

	small_data *item = Node()->small_data_start;
	while (!item->IsLast(Node()) || !strcmp(item->Name(),name))
		item = item->Next();

	// is the attribute already in the small_data section?
	// then just replace the data part of that one
	if (!strcmp(item->Name(),name)) {
		// find last attribute
		small_data *last = item;
		while (!last->IsLast(Node()))
			last = last->Next();

		// try to change the attributes value
		if (item->data_size > length
			|| force
			|| ((uint8 *)last + length - item->data_size) < ((uint8 *)Node() + fVolume->InodeSize())) {
			// make room for the new attribute if needed (and we are forced to do so)
			if (force
				&& ((uint8 *)last + length - item->data_size) < ((uint8 *)Node() + fVolume->InodeSize())) {
				// ToDo: we're lazy here and requesting the full difference between
				// the old size and the new size - we could also take the free bytes
				// at the end of the section into account...
				if (MakeSpaceForSmallData(transaction,name,length - item->data_size) < B_OK)
					return B_ERROR;
			}

			// move the attributes after the current one
			if (!item->IsLast(Node())) {
				small_data *next = item->Next();
				memmove((uint8 *)item + spaceNeeded,next,(uint8 *)last - (uint8 *)next + last->Size());
			}
			memcpy(item->Data(),data,length);
			item->Data()[length] = '\0';
			item->type = type;

			return B_OK;
		}

		// could not replace the old attribute, so remove it!
		if (RemoveSmallData(name,item) < B_OK)
			return B_ERROR;

		return B_DEVICE_FULL;
	}

	// try to add the new attribute!

	if (item->type != 0 || (uint8 *)item + spaceNeeded > (uint8 *)Node() + fVolume->InodeSize()) {
		// there is not enough space for it!
		if (!force)
			return B_DEVICE_FULL;

		// make room for the new attribute
		if (MakeSpaceForSmallData(transaction,name,spaceNeeded) < B_OK)
			return B_ERROR;

		// get new last item!
		item = Node()->small_data_start;
		while (!item->IsLast(Node()))
			item = item->Next();
	}

	memset(item,0,spaceNeeded);
	item->type = type;
	item->name_size = nameLength;
	item->data_size = length;
	strcpy(item->Name(),name);
	memcpy(item->Data(),data,length);

	// correctly terminate the small_data section
	if (!item->IsLast(Node())) {
		item = item->Next();
		memset(item,0,sizeof(small_data));
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
 *	you can safely stop calling it at any point (you don't need
 *	to iterate through the whole list).
 */

status_t
Inode::GetNextSmallData(small_data **smallData) const
{
	if (!Node())
		RETURN_ERROR(B_ERROR);

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


const char *
Inode::Name() const
{
	small_data *smallData = NULL;
	while (GetNextSmallData(&smallData) == B_OK) {
		if (*smallData->Name() == FILE_NAME_NAME)
			return (const char *)smallData->Data();
	}
	return NULL;
}


/**	Changes or set the name of a file: in the inode small_data section only, it
 *	doesn't change it in the parent directory's b+tree.
 *	Note that you need to write back the inode yourself after having called
 *	that method. It suffers from the same API decision as AddSmallData() does
 *	(and for the same reason).
 */

status_t 
Inode::SetName(Transaction *transaction,const char *name)
{
	if (name == NULL || *name == '\0')
		return B_BAD_VALUE;

	const char nameTag[2] = {FILE_NAME_NAME, 0};

	return AddSmallData(transaction,nameTag,FILE_NAME_TYPE,(uint8 *)name,strlen(name),true);
}


//	#pragma mark -


Inode *
Inode::GetAttribute(const char *name)
{
	// does this inode even have attributes?
	if (Attributes().IsZero())
		return NULL;

	Inode *attributes;
	if (get_vnode(fVolume->ID(),fVolume->ToVnode(Attributes()),(void **)&attributes) != 0
		|| attributes == NULL) {
		FATAL(("get_vnode() failed in Inode::GetAttribute(name = \"%s\")\n",name));
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

	// does it make sense to hold the attributes directory inode until here?
	// probably not... perhaps we should better move that right into GetAttribute().
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
			RETURN_ERROR(B_NO_MEMORY);

		*tree = fTree;
		status_t status = fTree->InitCheck();
		if (status < B_OK) {
			delete fTree;
			fTree = NULL;
		}
		RETURN_ERROR(status);
	}
	RETURN_ERROR(B_BAD_VALUE);
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
				RETURN_ERROR(B_ERROR);

			off_t start = pos - Node()->data.max_indirect_range;
			int32 indirectSize = (16 << fVolume->BlockShift()) * (fVolume->BlockSize() / sizeof(block_run));
			int32 directSize = 4 << fVolume->BlockShift();
			int32 index = start / indirectSize;
			
			//printf("\tstart = %Ld, indirectSize = %ld, directSize = %ld, index = %ld\n",start,indirectSize,directSize,index);
			//printf("\tlook for indirect block at %ld,%d\n",indirect[index].allocation_group,indirect[index].start);

			indirect = (block_run *)cached.SetTo(indirect[index]);
			if (indirect == NULL)
				RETURN_ERROR(B_ERROR);
			
			int32 current = (start % indirectSize) / directSize;

			run = indirect[current];
			offset = Node()->data.max_indirect_range + (index * indirectSize) + (current * directSize);
			//printf("\tfCurrent = %ld, fRunFileOffset = %Ld, fRunBlockEnd = %Ld, fRun = %ld,%d\n",fCurrent,fRunFileOffset,fRunBlockEnd,fRun.allocation_group,fRun.start);
		} else {
			// access to indirect blocks

			CachedBlock cached(fVolume,Node()->data.indirect);
			block_run *indirect = (block_run *)cached.Block();
			if (indirect == NULL)
				RETURN_ERROR(B_ERROR);

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
			RETURN_ERROR(B_ERROR);
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
		RETURN_ERROR(B_ERROR);
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
		RETURN_ERROR(B_BAD_VALUE);
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
			RETURN_ERROR(B_BAD_VALUE);
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
			RETURN_ERROR(B_BAD_VALUE);
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
					RETURN_ERROR(B_BAD_VALUE);
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
			RETURN_ERROR(B_BAD_VALUE);
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
			RETURN_ERROR(B_BAD_VALUE);
		}
	}
	
	*_length = bytesRead;
	return B_NO_ERROR;
}


status_t 
Inode::WriteAt(Transaction *transaction,off_t pos,void *buffer,size_t *length)
{
	// not yet implemented
	// should be pretty similar to ReadAt(), as long as the space is
	// already reserved in the inode's data_stream

	RETURN_ERROR(B_ERROR);
}


status_t 
Inode::GrowStream(Transaction *transaction, off_t size)
{
	data_stream *data = &Node()->data;

	// is the data stream already large enough to hold the new size?
	// (can be the case with preallocated blocks)
	if (size < data->max_direct_range
		|| size < data->max_indirect_range
		|| size < data->max_double_indirect_range) {
		data->size = size;
		PRINT(("enough space in inode preallocated!\n"));
		
		return B_OK;
	}

	// how many bytes are still needed? (unused ranges are always zero)
	off_t bytes;		
	if (data->size < data->max_double_indirect_range)
		bytes = size - data->max_double_indirect_range;
	else if (data->size < data->max_indirect_range)
		bytes = size - data->max_indirect_range;
	else if (data->size < data->max_direct_range)
		bytes = size - data->max_direct_range;
	else
		bytes = size - data->size;

	// do we have enough free blocks on the disk?
	off_t blocks = (bytes + fVolume->BlockSize() - 1) / fVolume->BlockSize();
	if (blocks > fVolume->FreeBlocks())
		return B_DEVICE_FULL;

	while (blocks > 0) {
		// the requested blocks do not need to be returned with a
		// single allocation, so we need to iterate until we have
		// enough blocks allocated
		block_run run;
		if (fVolume->Allocate(transaction,this,blocks,run) < B_OK)
			return B_DEVICE_FULL;

		// okay, we have the needed blocks, so just distribute them to the
		// different ranges of the stream (direct, indirect & double indirect)
		
		blocks -= run.length;
		
		if (data->size <= data->max_direct_range) {
			// let's try to put them into the direct block range
			int32 free = 0;
			for (;free < NUM_DIRECT_BLOCKS;free++)
				if (data->direct[free].IsZero())
					break;

			if (free < NUM_DIRECT_BLOCKS) {
				// can we merge the last allocated run with the new one?
				int32 last = free - 1;
				if (free > 0
					&& data->direct[last].allocation_group == run.allocation_group
					&& data->direct[last].start + data->direct[last].length == run.start) {
					data->direct[last].length += run.length;
				} else {
					data->direct[free] = run;
				}
				data->max_direct_range += run.length * fVolume->BlockSize();
				continue;
			}
		}
		
		// when we are here, we need to grow into the indirect or double
		// indirect range - but that's not yet implemented, so bail out!

		RETURN_ERROR(B_ERROR);
	}
	// update the size of the data stream
	data->size = size;

	return B_OK;
}


status_t 
Inode::ShrinkStream(Transaction *transaction, off_t size)
{
	return B_ERROR;
}


status_t 
Inode::SetFileSize(Transaction *transaction, off_t size)
{
	if (size < 0)
		return B_BAD_VALUE;

	if (size == Node()->data.size)
		return B_OK;

	// should the data stream grow or shrink?
	status_t status;
	if (size > Node()->data.size)
		status = GrowStream(transaction,size);
	else
		status = ShrinkStream(transaction,size);

	if (status < B_OK)
		return status;

	return WriteBack(transaction);
}


status_t 
Inode::Append(Transaction *transaction,off_t bytes)
{
	return SetFileSize(transaction,Size() + bytes);
}


status_t 
Inode::Create(Inode *directory, const char *name, int32 mode, int omode, off_t *id)
{
	Volume *volume = directory->fVolume;

	BPlusTree *tree;
	if (directory->GetTree(&tree) != B_OK)
		RETURN_ERROR(B_BAD_VALUE);

	// does the file already exist?
	off_t offset;
	if (tree->Find((uint8 *)name,(uint16)strlen(name),&offset) == B_OK) {
		// return if the file should be a directory or opened in exclusive mode
		if (mode & S_DIRECTORY || omode & O_EXCL)
			return B_FILE_EXISTS;

		// if it's a directory, bail out!
		// if omode & O_TRUNC, truncate the existing file
		if (omode & O_TRUNC) {
			//Transaction transaction(volume,directory->BlockNumber());

			//status_t status = inode->SetFileSize(&transaction,0);
			//if (status < B_OK)
			//	return status;

			//transaction.Done();
		}

		return B_OK;
	}

	// allocate space for the new inode
	Transaction transaction(volume,directory->BlockNumber());
	block_run run;

	status_t status = volume->AllocateForInode(&transaction,directory,mode,run);
	if (status < B_OK)
		return status;

	Inode *inode = new Inode(volume,volume->ToVnode(run));
	if (inode == NULL)
		return B_NO_MEMORY;

	// add name to the parent's B+tree
	if (tree->Insert(&transaction,name,volume->ToBlock(run)) < B_OK)
		RETURN_ERROR(B_ERROR);

	// update indices - only if it's a regular file!

	/** fill the bfs_inode structure **/

	bfs_inode *node = inode->Node();
	memset(node,0,volume->BlockSize());

	node->magic1 = INODE_MAGIC1;
	node->inode_num = run;
	node->parent = directory->BlockRun();

	node->uid = geteuid();
	node->gid = getegid();
	node->mode = mode;
	node->flags = 0;

	node->create_time = time(NULL) << INODE_TIME_SHIFT;
	node->last_modified_time = node->create_time;

	node->inode_size = volume->InodeSize();

	// initialize b+tree if it's a directory (and add "." & ".." if it's
	// a standard directory for files - not attributes or indices)
	if (mode & S_DIRECTORY) {
		BPlusTree *tree = inode->fTree = new BPlusTree(&transaction,inode);
		if (tree == NULL || tree->InitCheck() < B_OK)
			return B_ERROR;

		if ((mode & S_INDEX_DIR) == 0) {
			if (tree->Insert(&transaction,".",inode->BlockNumber()) < B_OK
				|| tree->Insert(&transaction,"..",volume->ToBlock(inode->Parent())) < B_OK)
				return B_ERROR;
		}
	}

	if ((status = inode->WriteBack(&transaction)) == B_OK)
		transaction.Done();

	if (new_vnode(volume->ID(),inode->ID(),inode) != B_OK)
		return B_ERROR;

	notify_listener(B_ENTRY_CREATED,volume->ID(),directory->ID(),0,inode->ID(),name);
	if (id != NULL)
		*id = inode->ID();

	return B_OK;
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

	// if you haven't yet access to the attributes directory, get it
	if (fAttributes == NULL) {
		if (get_vnode(volume->ID(),volume->ToVnode(fInode->Attributes()),(void **)&fAttributes) != 0
			|| fAttributes == NULL) {
			FATAL(("get_vnode() failed in AttributeIterator::GetNext(vnode_id = %Ld,name = \"%s\")\n",fInode->ID(),name));
			return B_ENTRY_NOT_FOUND;
		}

		BPlusTree *tree;
		if (fAttributes->GetTree(&tree) < B_OK
			|| (fIterator = new TreeIterator(tree)) == NULL) {
			FATAL(("could not get tree in AttributeIterator::GetNext(vnode_id = %Ld,name = \"%s\")\n",fInode->ID(),name));
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
		// when we know how get_vnode() is implemented in the kernel, we could
		// get rid of those checks (if attribute != NULL) - and we will in OpenBeOS :-)
	if (status == B_OK && attribute != NULL) {
		*_type = attribute->Node()->type;
		*_length = attribute->Node()->data.size;
		*_id = id;
	} else if (status == B_OK) {
		FATAL(("get_vnode() failed in AttributeIterator::GetNext(vnode_id = %Ld,name = \"%s\")\n",fInode->ID(),name));
		status = B_ERROR;
	}

	put_vnode(volume->ID(),id);
	
	return status;
}

