/* BPlusTree - BFS B+Tree implementation
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** Roughly based on 'btlib' written by Marcus J. Ranum
**
** Copyright (c) 2001-2002 pinc Software. All Rights Reserved.
** This file may be used under the terms of the OpenBeOS License.
*/


#include "Debug.h"
#include "cpp.h"
#include "BPlusTree.h"
#include "Inode.h"
#include "Stack.h"

#include <TypeConstants.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


// Node Caching for the BPlusTree class
//
// With write support, there is the need for a function that allocates new
// nodes by either returning empty nodes, or by growing the file's data stream
//
// !! The CachedNode class assumes that you have properly locked the stream
// !! before asking for nodes.
//
// Note: This code will fail if the block size is smaller than the node size!
// Since BFS supports block sizes of 1024 bytes or greater, and the node size
// is hard-coded to 1024 bytes, that's not an issue now.

void 
CachedNode::Unset()
{
	if (fTree == NULL || fTree->fStream == NULL) {
		REPORT_ERROR(B_BAD_VALUE);
		return;
	}

	if (fBlock != NULL) {
		release_block(fTree->fStream->GetVolume()->Device(),fBlockNumber);
	
		fBlock = NULL;
		fNode = NULL;
	}
}


bplustree_node *
CachedNode::SetTo(off_t offset,bool check)
{
	if (fTree == NULL || fTree->fStream == NULL) {
		REPORT_ERROR(B_BAD_VALUE);
		return NULL;
	}

	Unset();

	// You can only ask for nodes at valid positions - you can't
	// even access the b+tree header with this method (use SetToHeader()
	// instead)
	if (offset > fTree->fHeader->maximum_size - fTree->fNodeSize
		|| offset <= 0
		|| (offset % fTree->fNodeSize) != 0)
		return NULL;

	if (InternalSetTo(offset) != NULL && check) {
		// sanity checks (links, all_key_count)
		bplustree_header *header = fTree->fHeader;
		if (!header->IsValidLink(fNode->left_link)
			|| !header->IsValidLink(fNode->right_link)
			|| !header->IsValidLink(fNode->overflow_link)
			|| (int8 *)fNode->Values() + fNode->all_key_count * sizeof(off_t) >
					(int8 *)fNode + fTree->fNodeSize) {
			FATAL(("invalid node read from offset %Ld, inode at %Ld\n",
					offset,fTree->fStream->ID()));
			return NULL;
		}
	}
	return fNode;
}


bplustree_header *
CachedNode::SetToHeader()
{
	if (fTree == NULL || fTree->fStream == NULL) {
		REPORT_ERROR(B_BAD_VALUE);
		return NULL;
	}

	Unset();
	
	InternalSetTo(0LL);
	return (bplustree_header *)fNode;
}


bplustree_node *
CachedNode::InternalSetTo(off_t offset)
{
	fNode = NULL;

	off_t fileOffset;
	block_run run;
	if (fTree->fStream->FindBlockRun(offset,run,fileOffset) == B_OK) {
		Volume *volume = fTree->fStream->GetVolume();

		int32 blockOffset = (offset - fileOffset) / volume->BlockSize();
		fBlockNumber = volume->ToBlock(run) + blockOffset;

		fBlock = (uint8 *)get_block(volume->Device(),fBlockNumber,volume->BlockSize());
		if (fBlock) {
			// the node is somewhere in that block... (confusing offset calculation)
			fNode = (bplustree_node *)(fBlock + offset -
						(fileOffset + blockOffset * volume->BlockSize()));
		} else
			REPORT_ERROR(B_IO_ERROR);
	}
	return fNode;
}


status_t
CachedNode::Free(Transaction *transaction,off_t offset)
{
	if (transaction == NULL || fTree == NULL || fTree->fStream == NULL
		|| offset == BPLUSTREE_NULL)
		RETURN_ERROR(B_BAD_VALUE);

	fNode->left_link = fTree->fHeader->free_node_pointer;
	if (WriteBack(transaction) == B_OK) {
		fTree->fHeader->free_node_pointer = offset;
		return fTree->fCachedHeader.WriteBack(transaction);
	}
	return B_ERROR;
}


bplustree_node *
CachedNode::Allocate(Transaction *transaction, off_t *_offset)
{
	if (transaction == NULL || fTree == NULL || fTree->fStream == NULL) {
		REPORT_ERROR(B_BAD_VALUE);
		return NULL;
	}

	// if there are any free nodes, recycle them
	if (fTree->fHeader && SetTo(fTree->fHeader->free_node_pointer,false) != NULL) {
		*_offset = fTree->fHeader->free_node_pointer;
		
		// set new free node pointer
		fTree->fHeader->free_node_pointer = fNode->left_link;
		if (fTree->fCachedHeader.WriteBack(transaction) == B_OK)
			return fNode;

		return NULL;
	}
	// allocate new node
	// -> not yet implemented
	return NULL;
}


status_t 
CachedNode::WriteBack(Transaction *transaction)
{
	if (transaction == NULL || fTree == NULL || fTree->fStream == NULL || fNode == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	return transaction->WriteBlocks(fBlockNumber,fBlock);
}


//	#pragma mark -


BPlusTree::BPlusTree(Transaction *transaction,Inode *stream,int32 keyType,int32 nodeSize,bool allowDuplicates)
	:
	fStream(NULL),
	fHeader(NULL),
	fCachedHeader(this)
{
	SetTo(transaction,stream,keyType,nodeSize,allowDuplicates);
}


BPlusTree::BPlusTree(Inode *stream,bool allowDuplicates)
	:
	fStream(NULL),
	fHeader(NULL),
	fCachedHeader(this)
{
	SetTo(stream,allowDuplicates);
}


BPlusTree::BPlusTree()
	:
	fStream(NULL),
	fHeader(NULL),
	fCachedHeader(this),
	fNodeSize(BPLUSTREE_NODE_SIZE),
	fAllowDuplicates(true),
	fStatus(B_NO_INIT)
{
}


BPlusTree::~BPlusTree()
{
}


status_t
BPlusTree::Initialize(int32 nodeSize)
{
	fStream = NULL;

	fNodeSize = nodeSize;

	// allocate new header if needed
	if (fHeader == NULL) {
		fHeader = (bplustree_header *)malloc(fNodeSize);
		if (fHeader == NULL) {
			FATAL(("no memory for the b+tree header! Prepare to die...\n"));
			return B_NO_MEMORY;
		}
	}
 	memset(fHeader,0,fNodeSize);
 	
 	return B_OK;
}


status_t
BPlusTree::SetTo(Transaction *transaction,Inode *stream,int32 keyType,int32 nodeSize,bool allowDuplicates)
{
	// initializes in-memory B+Tree

	fCachedHeader.Unset();
	fStream = stream;

	fHeader = fCachedHeader.SetToHeader();
	if (fHeader == NULL) {
		// allocate space for new header + node!
		// -> not yet implemented
		RETURN_ERROR(fStatus = B_NO_INIT);
	}

	fAllowDuplicates = allowDuplicates;
	fNodeSize = nodeSize;

	// initialize b+tree header
 	fHeader->magic = BPLUSTREE_MAGIC;
 	fHeader->node_size = fNodeSize;
 	fHeader->max_number_of_levels = 1;
 	fHeader->data_type = keyType;
 	fHeader->root_node_pointer = fNodeSize;
 	fHeader->free_node_pointer = BPLUSTREE_NULL;
 	fHeader->maximum_size = fNodeSize * 2;
 	
	return fStatus = B_OK;
}


status_t
BPlusTree::SetTo(Inode *stream,bool allowDuplicates)
{
	// get on-disk B+Tree header

	fCachedHeader.Unset();
	fStream = stream;

	fHeader = fCachedHeader.SetToHeader();
	if (fHeader == NULL)
		RETURN_ERROR(fStatus = B_NO_INIT);
	
	// is header valid?

	if (fHeader->magic != BPLUSTREE_MAGIC
		|| fHeader->maximum_size != stream->Node()->data.size
		|| (fHeader->root_node_pointer % fHeader->node_size) != 0
		|| !fHeader->IsValidLink(fHeader->root_node_pointer)
		|| !fHeader->IsValidLink(fHeader->free_node_pointer))
		RETURN_ERROR(fStatus = B_BAD_DATA);

	fAllowDuplicates = allowDuplicates;
	fNodeSize = fHeader->node_size;

	{
		uint32 toMode[] = {S_STR_INDEX, S_INT_INDEX, S_UINT_INDEX, S_LONG_LONG_INDEX,
						   S_ULONG_LONG_INDEX, S_FLOAT_INDEX, S_DOUBLE_INDEX};
		uint32 mode = stream->Mode() & (S_STR_INDEX | S_INT_INDEX | S_UINT_INDEX | S_LONG_LONG_INDEX
						   | S_ULONG_LONG_INDEX | S_FLOAT_INDEX | S_DOUBLE_INDEX);
	
		if (fHeader->data_type > BPLUSTREE_DOUBLE_TYPE
			|| (stream->Mode() & S_INDEX_DIR) && toMode[fHeader->data_type] != mode
			|| !stream->IsDirectory()) {
			D(	dump_bplustree_header(fHeader);
				dump_inode(stream->Node());
			);
			RETURN_ERROR(fStatus = B_BAD_TYPE);
		}

		 // although it's in stat.h, the S_ALLOW_DUPS flag is obviously unused
		fAllowDuplicates = (stream->Mode() & (S_INDEX_DIR | 0777)) == S_INDEX_DIR;
	}

	CachedNode cached(this,fHeader->root_node_pointer);
	RETURN_ERROR(fStatus = cached.Node() ? B_OK : B_BAD_DATA);
}


status_t
BPlusTree::InitCheck()
{
	return fStatus;
}


//	#pragma mark -


int32
BPlusTree::CompareKeys(const void *key1, int keyLength1, const void *key2, int keyLength2)
{
	type_code type = 0;
	switch (fHeader->data_type)
	{
	    case BPLUSTREE_STRING_TYPE:
	    	type = B_STRING_TYPE;
	    	break;
		case BPLUSTREE_INT32_TYPE:
	    	type = B_INT32_TYPE;
	    	break;
		case BPLUSTREE_UINT32_TYPE:
	    	type = B_UINT32_TYPE;
	    	break;
		case BPLUSTREE_INT64_TYPE:
	    	type = B_INT64_TYPE;
	    	break;
		case BPLUSTREE_UINT64_TYPE:
	    	type = B_UINT64_TYPE;
	    	break;
		case BPLUSTREE_FLOAT_TYPE:
	    	type = B_FLOAT_TYPE;
	    	break;
		case BPLUSTREE_DOUBLE_TYPE:
	    	type = B_DOUBLE_TYPE;
	    	break;
	}
   	return compareKeys(type,key1,keyLength1,key2,keyLength2);
}


status_t
BPlusTree::FindKey(bplustree_node *node,uint8 *key,uint16 keyLength,uint16 *index,off_t *next)
{
	if (node->all_key_count == 0)
	{
		if (index)
			*index = 0;
		if (next)
			*next = node->overflow_link;
		return B_ENTRY_NOT_FOUND;
	}

	off_t *values = node->Values();
	int16 saveIndex;

	// binary search in the key array
	for (int16 first = 0,last = node->all_key_count - 1;first <= last;)
	{
		uint16 i = (first + last) >> 1;

		uint16 searchLength;
		uint8 *searchKey = node->KeyAt(i,&searchLength);
		
		int32 cmp = CompareKeys(key,keyLength,searchKey,searchLength);
		if (cmp < 0)
		{
			last = i - 1;
			saveIndex = i;
		}
		else if (cmp > 0)
		{
			saveIndex = first = i + 1;
		}
		else
		{
			if (index)
				*index = i;
			if (next)
				*next = values[i];
			return B_OK;
		}
	}

	if (index)
		*index = saveIndex;
	if (next)
	{
		if (saveIndex == node->all_key_count)
			*next = node->overflow_link;
		else
			*next = values[saveIndex];
	}
	return B_ENTRY_NOT_FOUND;
}


/**	Prepares the stack to contain all nodes that were passed while
 *	following the key, from the root node to the leaf node that could
 *	or should contain that key.
 */

status_t
BPlusTree::SeekDown(Stack<node_and_key> &stack,uint8 *key,uint16 keyLength)
{
	// set the root node to begin with
	node_and_key nodeAndKey;
	nodeAndKey.nodeOffset = fHeader->root_node_pointer;

	CachedNode cached(this);
	bplustree_node *node;
	while ((node = cached.SetTo(nodeAndKey.nodeOffset)) != NULL) {
		// if we are already on leaf level, we're done
		if (node->overflow_link == BPLUSTREE_NULL) {
			// node that the keyIndex is not properly set here (but it's not
			// needed in the calling functions anyway)!
			nodeAndKey.keyIndex = 0;
			stack.Push(nodeAndKey);
			return B_OK;
		}

		off_t nextOffset;
		status_t status = FindKey(node,key,keyLength,&nodeAndKey.keyIndex,&nextOffset);
		
		if (status == B_ENTRY_NOT_FOUND && nextOffset == nodeAndKey.nodeOffset)
			RETURN_ERROR(B_ERROR);

		// put the node offset & the correct keyIndex on the stack
		stack.Push(nodeAndKey);

		nodeAndKey.nodeOffset = nextOffset;
	}
	RETURN_ERROR(B_ERROR);
}


void
BPlusTree::InsertKey(bplustree_node *node,uint8 *key,uint16 keyLength,off_t value,uint16 index)
{
	// should never happen, but who knows?
	if (index > node->all_key_count)
		return;

	off_t *values = node->Values();
	uint16 *keyLengths = node->KeyLengths();
	uint8 *keys = node->Keys();

	node->all_key_count++;
	node->all_key_length += keyLength;

	off_t *newValues = node->Values();
	uint16 *newKeyLengths = node->KeyLengths();

	// move values and copy new value into them
	memmove(newValues + index + 1,values + index,sizeof(off_t) * (node->all_key_count - 1 - index));
	memmove(newValues,values,sizeof(off_t) * index);

	newValues[index] = value;

	// move and update key length index
	for (uint16 i = node->all_key_count;i-- > index + 1;)
		newKeyLengths[i] = keyLengths[i - 1] + keyLength;
	memmove(newKeyLengths,keyLengths,sizeof(uint16) * index);

	int32 keyStart;
	newKeyLengths[index] = keyLength + (keyStart = index > 0 ? newKeyLengths[index - 1] : 0);

	// move keys and copy new key into them
	int32 size = node->all_key_length - newKeyLengths[index];
	if (size > 0)
		memmove(keys + newKeyLengths[index],keys + newKeyLengths[index] - keyLength,size);

	memcpy(keys + keyStart,key,keyLength);
}


status_t
BPlusTree::InsertDuplicate(bplustree_node */*node*/,uint16 /*index*/)
{
//	printf("DUPLICATE ENTRY!!\n");

	return B_OK;
}


status_t
BPlusTree::SplitNode(bplustree_node *node,off_t nodeOffset,bplustree_node *other,off_t otherOffset,uint16 *_keyIndex,uint8 *key,uint16 *_keyLength,off_t *_value)
{
	if (*_keyIndex > node->all_key_count + 1)
		return B_BAD_VALUE;

	uint16 *inKeyLengths = node->KeyLengths();
	off_t *inKeyValues = node->Values();
	uint8 *inKeys = node->Keys();
	uint8 *outKeys = other->Keys();
	uint16 keyIndex = *_keyIndex;

	// how many keys will fit in one (half) page?
	// that loop will find the answer to this question and
	// change the key lengths indices for their new home

	// "bytes" is the number of bytes written for the new key,
	// "bytesBefore" are the bytes before that key
	// "bytesAfter" are the bytes after the new key, if any
	int32 bytes = 0,bytesBefore = 0,bytesAfter = 0;

	size_t size = fNodeSize >> 1;
	int32 out,in;
	for (in = out = 0;in < node->all_key_count + 1;) {
		if (!bytes)
			bytesBefore = in > 0 ? inKeyLengths[in - 1] : 0;

		if (in == keyIndex && !bytes) {
			bytes = *_keyLength;
		} else {
			if (keyIndex < out) {
				bytesAfter = inKeyLengths[in] - bytesBefore;
				// fix the key lengths for the new node
				inKeyLengths[in] = bytesAfter + bytesBefore + bytes;
			}
			in++;
		}
		out++;

		if (round_up(sizeof(bplustree_node) + bytesBefore + bytesAfter + bytes) +
						out * (sizeof(uint16) + sizeof(off_t)) >= size) {
			// we have found the number of keys in the new node!
			break;
		}
	}

	// if the new key was not inserted, set the length of the keys
	// that can be copied directly
	if (keyIndex >= out && in > 0)
		bytesBefore = inKeyLengths[in - 1];

	if (bytesBefore < 0 || bytesAfter < 0)
		return B_BAD_DATA;

	other->left_link = node->left_link;
	other->right_link = nodeOffset;
	other->all_key_length = bytes + bytesBefore + bytesAfter;
	other->all_key_count = out;

	uint16 *outKeyLengths = other->KeyLengths();
	off_t *outKeyValues = other->Values();
	int32 keys = out > keyIndex ? keyIndex : out;

	if (bytesBefore) {
		// copy the keys
		memcpy(outKeys,inKeys,bytesBefore);
		memcpy(outKeyLengths,inKeyLengths,keys * sizeof(uint16));
		memcpy(outKeyValues,inKeyValues,keys * sizeof(off_t));
	}
	if (bytes) {
		// copy the newly inserted key
		memcpy(outKeys + bytesBefore,key,bytes);
		outKeyLengths[keyIndex] = bytes + bytesBefore;
		outKeyValues[keyIndex] = *_value;

		if (bytesAfter) {
			// copy the keys after the new key
			memcpy(outKeys + bytesBefore + bytes,inKeys + bytesBefore,bytesAfter);
			keys = out - keyIndex - 1;
			memcpy(outKeyLengths + keyIndex + 1,inKeyLengths + keyIndex,keys * sizeof(uint16));
			memcpy(outKeyValues + keyIndex + 1,inKeyValues + keyIndex,keys * sizeof(off_t));
		}
	}

	// if the new key was already inserted, we shouldn't use it again
	if (in != out)
		keyIndex--;

	int32 total = bytesBefore + bytesAfter;

	// if we have split an index node, we have to drop the first key
	// of the next node (which can also be the new key to insert)
	if (node->overflow_link != BPLUSTREE_NULL) {
		if (in == keyIndex) {
			other->overflow_link = *_value;
			keyIndex--;
		} else {
			other->overflow_link = inKeyValues[in];
			total = inKeyLengths[in++];
		}
	}

	// and now the same game for the other page and the rest of the keys
	// (but with memmove() instead of memcpy(), because they may overlap)

	bytesBefore = bytesAfter = bytes = 0;
	out = 0;
	int32 skip = in;
	while (in < node->all_key_count + 1) {
		if (in == keyIndex && !bytes) {
			// it's enough to set bytesBefore once here, because we do
			// not need to know the exact length of all keys in this
			// loop
			bytesBefore = in > skip ? inKeyLengths[in - 1] : 0;
			bytes = *_keyLength;
		} else {
			if (in < node->all_key_count) {
				inKeyLengths[in] -= total;
				if (bytes) {
					inKeyLengths[in] += bytes;
					bytesAfter = inKeyLengths[in] - bytesBefore - bytes;
				}
			}
			in++;
		}

		out++;

		// break out when all keys are done
		if (in > node->all_key_count && keyIndex < in)
			break;
	}

	// adjust the byte counts (since we were a bit lazy in the loop)
	if (keyIndex >= in && keyIndex - skip < out)
		bytesAfter = inKeyLengths[in] - bytesBefore - total;
	else if (keyIndex < skip)
		bytesBefore = node->all_key_length - total;

	if (bytesBefore < 0 || bytesAfter < 0)
		return B_BAD_DATA;

	node->left_link = otherOffset;
		// right link, and overflow link can stay the same
	node->all_key_length = bytes + bytesBefore + bytesAfter;
	node->all_key_count = out - 1;

	// array positions have changed
	outKeyLengths = node->KeyLengths();
	outKeyValues = node->Values();

	// move the keys in the old node: the order is important here,
	// because we don't want to overwrite any contents

	keys = keyIndex <= skip ? out : keyIndex - skip;
	keyIndex -= skip;

	if (bytesBefore)
		memmove(inKeys,inKeys + total,bytesBefore);
	if (bytesAfter)
		memmove(inKeys + bytesBefore + bytes,inKeys + total + bytesBefore,bytesAfter);

	if (bytesBefore)
		memmove(outKeyLengths,inKeyLengths + skip,keys * sizeof(uint16));
	in = out - keyIndex - 1;
	if (bytesAfter)
		memmove(outKeyLengths + keyIndex + 1,inKeyLengths + skip + keyIndex,in * sizeof(uint16));

	if (bytesBefore)
		memmove(outKeyValues,inKeyValues + skip,keys * sizeof(off_t));
	if (bytesAfter)
		memmove(outKeyValues + keyIndex + 1,inKeyValues + skip + keyIndex,in * sizeof(off_t));

	if (bytes) {
		// finally, copy the newly inserted key (don't overwrite anything)
		memcpy(inKeys + bytesBefore,key,bytes);
		outKeyLengths[keyIndex] = bytes + bytesBefore;
		outKeyValues[keyIndex] = *_value;
	}

	// prepare key to insert in the parent node
	
	uint16 length;
	uint8 *lastKey = other->KeyAt(other->all_key_count - 1,&length);
	memcpy(key,lastKey,length);
	*_keyLength = length;
	*_value = otherOffset;

	return B_OK;
}


status_t
BPlusTree::Insert(Transaction *transaction,uint8 *key,uint16 keyLength,off_t value)
{
	if (keyLength < BPLUSTREE_MIN_KEY_LENGTH || keyLength > BPLUSTREE_MAX_KEY_LENGTH)
		RETURN_ERROR(B_BAD_VALUE);

	// lock access to stream
	WriteLocked locked(fStream->Lock());

	Stack<node_and_key> stack;
	if (SeekDown(stack,key,keyLength) != B_OK)
		RETURN_ERROR(B_ERROR);

	uint8 keyBuffer[BPLUSTREE_MAX_KEY_LENGTH + 1];

	memcpy(keyBuffer,key,keyLength);
	keyBuffer[keyLength] = 0;

	// ToDo: update all tree iterators after the tree has changed!

	node_and_key nodeAndKey;
	bplustree_node *node;

	CachedNode cached(this);
	while (stack.Pop(&nodeAndKey) && (node = cached.SetTo(nodeAndKey.nodeOffset)) != NULL)
	{
		if (node->IsLeaf())	// first round, check for duplicate entries
		{
			status_t status = FindKey(node,key,keyLength,&nodeAndKey.keyIndex);

			// is this a duplicate entry?
			if (status == B_OK && node->overflow_link == BPLUSTREE_NULL)
			{
				if (fAllowDuplicates)
					return InsertDuplicate(node,nodeAndKey.keyIndex);
				else
					RETURN_ERROR(B_NAME_IN_USE);
			}
		}

		// is the node big enough to hold the pair?
		if (int32(round_up(sizeof(bplustree_node) + node->all_key_length + keyLength)
			+ (node->all_key_count + 1) * (sizeof(uint16) + sizeof(off_t))) < fNodeSize)
		{
			InsertKey(node,keyBuffer,keyLength,value,nodeAndKey.keyIndex);
			return cached.WriteBack(transaction);
		}
		else
		{
			CachedNode cachedNewRoot(this);
			CachedNode cachedOther(this);

			// do we need to allocate a new root node? if so, then do
			// it now
			off_t newRoot = BPLUSTREE_NULL;
			if (nodeAndKey.nodeOffset == fHeader->root_node_pointer) {
				if (cachedNewRoot.Allocate(transaction,&newRoot) == NULL) {
					// The tree is most likely corrupted!
					// But it's still sane at leaf level - we could set
					// a flag in the header that forces the tree to be
					// rebuild next time...
					// But since we will have journaling, that's not a big
					// problem anyway.
					RETURN_ERROR(B_NO_MEMORY);
				}
			}

			// reserve space for the other node
			off_t otherOffset;
			bplustree_node *other = cachedOther.Allocate(transaction,&otherOffset);
			if (other == NULL) {
				cachedNewRoot.Free(transaction,newRoot);
				RETURN_ERROR(B_NO_MEMORY);
			}

			if (SplitNode(node,nodeAndKey.nodeOffset,other,otherOffset,&nodeAndKey.keyIndex,keyBuffer,&keyLength,&value) < B_OK) {
				// free root node & other node here
				cachedNewRoot.Free(transaction,newRoot);
				cachedOther.Free(transaction,otherOffset);					

				RETURN_ERROR(B_ERROR);
			}

			// write the updated nodes back
		
			if (cached.WriteBack(transaction) < B_OK
				|| cachedOther.WriteBack(transaction) < B_OK)
				RETURN_ERROR(B_ERROR);

			// update the right link of the node in the left of the new node
			if ((other = cachedOther.SetTo(other->left_link)) != NULL) {
				other->right_link = otherOffset;
				if (cachedOther.WriteBack(transaction) < B_OK)
					RETURN_ERROR(B_ERROR);
			}

			// create a new root if necessary
			if (newRoot != BPLUSTREE_NULL) {
				bplustree_node *rootNode = cachedNewRoot.Node();

				InsertKey(rootNode,keyBuffer,keyLength,node->left_link,0);
				rootNode->overflow_link = nodeAndKey.nodeOffset;

				if (cachedNewRoot.WriteBack(transaction) < B_OK)
					RETURN_ERROR(B_ERROR);

				// finally, update header to point to the new root
				fHeader->root_node_pointer = newRoot;
				fHeader->max_number_of_levels++;

				return fCachedHeader.WriteBack(transaction);
			}
		}
	}
	RETURN_ERROR(B_ERROR);
}


/**	Searches the key in the tree, and stores the offset found in
 *	_value, if successful.
 *	It's very similar to BPlusTree::SeekDown(), but doesn't fill
 *	a stack while it descends the tree.
 *	Returns B_OK when the key could be found, B_ENTRY_NOT_FOUND
 *	if not. It can also return other errors to indicate that
 *	something went wrong.
 */

status_t
BPlusTree::Find(uint8 *key,uint16 keyLength,off_t *_value)
{
	if (keyLength < BPLUSTREE_MIN_KEY_LENGTH || keyLength > BPLUSTREE_MAX_KEY_LENGTH
		|| key == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	// lock access to stream
	ReadLocked locked(fStream->Lock());

	off_t nodeOffset = fHeader->root_node_pointer;
	CachedNode cached(this);
	bplustree_node *node;

	while ((node = cached.SetTo(nodeOffset)) != NULL) {
		uint16 keyIndex = 0;
		off_t nextOffset;
		status_t status = FindKey(node,key,keyLength,&keyIndex,&nextOffset);

		if (node->overflow_link == BPLUSTREE_NULL) {
			if (status == B_OK && _value != NULL)
				*_value = node->Values()[keyIndex];

			return status;
		} else if (nextOffset == nodeOffset)
			RETURN_ERROR(B_ERROR);

		nodeOffset = nextOffset;
	}
	RETURN_ERROR(B_ERROR);
}


//	#pragma mark -


TreeIterator::TreeIterator(BPlusTree *tree)
	:
	fTree(tree),
	fCurrentNodeOffset(BPLUSTREE_NULL)
{
}


TreeIterator::~TreeIterator()
{
}


status_t
TreeIterator::Goto(int8 to)
{
	if (fTree == NULL || fTree->fHeader == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	// lock access to stream
	ReadLocked locked(fTree->fStream->Lock());

	off_t nodeOffset = fTree->fHeader->root_node_pointer;
	CachedNode cached(fTree);
	bplustree_node *node;

	while ((node = cached.SetTo(nodeOffset)) != NULL) {
		// is the node a leaf node?
		if (node->overflow_link == BPLUSTREE_NULL) {
			fCurrentNodeOffset = nodeOffset;
			fCurrentKey = to == BPLUSTREE_BEGIN ? -1 : node->all_key_count;
			fDuplicateNode = 0;

			return B_OK;
		}

		// get the next node offset depending on the direction (and if there
		// are any keys in that node at all)
		off_t nextOffset = to == BPLUSTREE_END || node->all_key_count == 0 ?
				node->overflow_link : *node->Values();
		if (nextOffset == nodeOffset)
			break;

		nodeOffset = nextOffset;
	}
	FATAL(("%s fails\n",__PRETTY_FUNCTION__));
	RETURN_ERROR(B_ERROR);
}


status_t
TreeIterator::Traverse(int8 direction,void *key,uint16 *keyLength,uint16 maxLength,off_t *value)
{
	if (fCurrentNodeOffset == BPLUSTREE_NULL
		&& Goto(direction == BPLUSTREE_FORWARD ? BPLUSTREE_BEGIN : BPLUSTREE_END) < B_OK) 
		RETURN_ERROR(B_ERROR);

	// lock access to stream
	ReadLocked locked(fTree->fStream->Lock());

	CachedNode cached(fTree);
	bplustree_node *node;

	if (fDuplicateNode)
	{
		// regardless of traverse direction the duplicates are always presented in
		// the same order; since they are all considered as equal, this shouldn't
		// cause any problems

		if (fDuplicate < fNumDuplicates
			&& (node = cached.SetTo(bplustree_node::FragmentOffset(fDuplicateNode),false)) != NULL)
		{
			*value = node->DuplicateAt(fDuplicateNode,fIsFragment,fDuplicate++);
			if (!fIsFragment && fDuplicate == fNumDuplicates)
			{
				if (node->right_link != BPLUSTREE_NULL)
				{
					fDuplicateNode = node->right_link;
					if ((node = cached.SetTo(bplustree_node::FragmentOffset(fDuplicateNode),false)) != NULL)
					{
						fNumDuplicates = node->CountDuplicates(fDuplicateNode,false);
						fDuplicate = 0;
					}
//					dump_bplustree_node(node);
				}
			}
			return B_OK;
		}
		else
			fDuplicateNode = 0;
	}

	off_t savedNodeOffset = fCurrentNodeOffset;
	if ((node = cached.SetTo(fCurrentNodeOffset)) == NULL)
		RETURN_ERROR(B_ERROR);

	fDuplicateNode = 0LL;

	fCurrentKey += direction;
	
	// is the current key in the current node?
	while ((direction == BPLUSTREE_FORWARD && fCurrentKey >= node->all_key_count)
		   || (direction == BPLUSTREE_BACKWARD && fCurrentKey < 0))
	{
		fCurrentNodeOffset = direction == BPLUSTREE_FORWARD ? node->right_link : node->left_link;

		// are there any more nodes?
		if (fCurrentNodeOffset != BPLUSTREE_NULL)
		{
			node = cached.SetTo(fCurrentNodeOffset);
			if (!node)
				RETURN_ERROR(B_ERROR);

			// reset current key
			fCurrentKey = direction == BPLUSTREE_FORWARD ? 0 : node->all_key_count;
		}
		else
		{
			// there are no nodes left, so turn back to the last key
			fCurrentNodeOffset = savedNodeOffset;
			fCurrentKey = direction == BPLUSTREE_FORWARD ? node->all_key_count : -1;

			return B_ENTRY_NOT_FOUND;
		}
	}

	if (node->all_key_count == 0)
		RETURN_ERROR(B_ERROR);	// B_ENTRY_NOT_FOUND ?

	uint16 length;
	uint8 *keyStart = node->KeyAt(fCurrentKey,&length);

	length = min_c(length,maxLength);
	memcpy(key,keyStart,length);
	
	if (fTree->fHeader->data_type == BPLUSTREE_STRING_TYPE)	// terminate string type
	{
		if (length == maxLength)
			length--;
		((char *)key)[length] = '\0';
	}
	*keyLength = length;

	off_t offset = node->Values()[fCurrentKey];
	
	// duplicate fragments?
	uint8 type = bplustree_node::LinkType(offset);
	if (type == BPLUSTREE_DUPLICATE_FRAGMENT || type == BPLUSTREE_DUPLICATE_NODE)
	{
		fDuplicateNode = offset;

		node = cached.SetTo(bplustree_node::FragmentOffset(fDuplicateNode),false);
		if (node == NULL)
			RETURN_ERROR(B_ERROR);

		fIsFragment = type == BPLUSTREE_DUPLICATE_FRAGMENT;

		fNumDuplicates = node->CountDuplicates(offset,fIsFragment);
		if (fNumDuplicates)
		{
			fDuplicate = 0;
			offset = node->DuplicateAt(offset,fIsFragment,fDuplicate++);
		}
		else
		{
			// shouldn't happen, but we're dealing here with potentially corrupt disks...
			fDuplicateNode = 0;
			offset = 0;
		}
	}
	*value = offset;

	return B_OK;
}


/**	This is more or less a copy of BPlusTree::Find() - but it just
 *	sets the current position in the iterator, regardless of if the
 *	key could be found or not.
 */

status_t 
TreeIterator::Find(uint8 *key, uint16 keyLength)
{
	if (keyLength < BPLUSTREE_MIN_KEY_LENGTH || keyLength > BPLUSTREE_MAX_KEY_LENGTH
		|| key == NULL)
		RETURN_ERROR(B_BAD_VALUE);

	// lock access to stream
	ReadLocked locked(fTree->fStream->Lock());

	off_t nodeOffset = fTree->fHeader->root_node_pointer;

	CachedNode cached(fTree);
	bplustree_node *node;
	while ((node = cached.SetTo(nodeOffset)) != NULL) {
		uint16 keyIndex = 0;
		off_t nextOffset;
		status_t status = fTree->FindKey(node,key,keyLength,&keyIndex,&nextOffset);

		if (node->overflow_link == BPLUSTREE_NULL) {
			fCurrentNodeOffset = nodeOffset;
			fCurrentKey = keyIndex - 1;
			fDuplicateNode = 0;

			return status;
		} else if (nextOffset == nodeOffset)
			RETURN_ERROR(B_ERROR);

		nodeOffset = nextOffset;
	}
	RETURN_ERROR(B_ERROR);
}


//	#pragma mark -


uint8 *
bplustree_node::KeyAt(int32 index,uint16 *keyLength) const
{
	if (index < 0 || index > all_key_count)
		return NULL;

	uint8 *keyStart = Keys();
	uint16 *keyLengths = KeyLengths();

	*keyLength = keyLengths[index] - (index != 0 ? keyLengths[index - 1] : 0);
	if (index > 0)
		keyStart += keyLengths[index - 1];
	
	return keyStart;
}


uint8
bplustree_node::CountDuplicates(off_t offset,bool isFragment) const
{
	// the duplicate fragment handling is currently hard-coded to a node size
	// of 1024 bytes - with future versions of BFS, this may be a problem

	if (isFragment) {
		uint32 fragment = 8 * ((uint64)offset & 0x3ff);
	
		return ((off_t *)this)[fragment];
	}
	return overflow_link;
}


off_t
bplustree_node::DuplicateAt(off_t offset,bool isFragment,int8 index) const
{
	uint32 start;
	if (isFragment)
		start = 8 * ((uint64)offset & 0x3ff);
	else
		start = 2;

	return ((off_t *)this)[start + 1 + index];
}


//	#pragma mark -


int32
compareKeys(type_code type,const void *key1, int keyLength1, const void *key2, int keyLength2)
{
	switch (type)
	{
	    case B_STRING_TYPE:
    	{
			int len = min_c(keyLength1,keyLength2);
			int result = strncmp((const char *)key1,(const char *)key2,len);
			
			if (result == 0)
				result = keyLength1 - keyLength2;

			return result;
		}

		case B_INT32_TYPE:
			return *(int32 *)key1 - *(int32 *)key2;
			
		case B_UINT32_TYPE:
		{
			if (*(uint32 *)key1 == *(uint32 *)key2)
				return 0;
			else if (*(uint32 *)key1 > *(uint32 *)key2)
				return 1;

			return -1;
		}
			
		case B_INT64_TYPE:
		{
			if (*(int64 *)key1 == *(int64 *)key2)
				return 0;
			else if (*(int64 *)key1 > *(int64 *)key2)
				return 1;

			return -1;
		}

		case B_UINT64_TYPE:
		{
			if (*(uint64 *)key1 == *(uint64 *)key2)
				return 0;
			else if (*(uint64 *)key1 > *(uint64 *)key2)
				return 1;

			return -1;
		}

		case B_FLOAT_TYPE:
		{
			float result = *(float *)key1 - *(float *)key2;
			if (result == 0.0f)
				return 0;

			return (result < 0.0f) ? -1 : 1;
		}

		case B_DOUBLE_TYPE:
		{
			double result = *(double *)key1 - *(double *)key2;
			if (result == 0.0)
				return 0;

			return (result < 0.0) ? -1 : 1;
		}
	}
	return 0;
}


