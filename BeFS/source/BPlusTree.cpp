/* BPlusTree - BFS B+Tree implementation
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** Roughly based on 'btlib' written by Marcus J. Ranum
**
** Copyright (c) 2001-2002 pinc Software. All Rights Reserved.
*/


#include "cpp.h"
#include "BPlusTree.h"
#include "Inode.h"
#include "Stack.h"

#include <Debug.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#define MAX_NODES_IN_CACHE 20


Cached::Cached(off_t _offset,bplustree_node *_node)
	:
	prev(NULL),
	next(NULL),
	locked(0L),
	isDirty(false),
	offset(_offset),
	node(_node)
{
}


Cached::~Cached()
{
	if (node)
		free(node);
}


//	#pragma mark -


TreeCache::TreeCache(int32 max)
	:
	fMaxInQueue(max),
	fCount(0),
	fMostRecentlyUsed(NULL),
	fLeastRecentlyUsed(NULL)
{
}


TreeCache::~TreeCache()
{
	Flush(NULL,0,true);
}


void 
TreeCache::Flush(BPlusTree *tree,int32 targetCount, bool force)
{
	Cached *entry = fLeastRecentlyUsed;
	while (entry)
	{
		Cached *prev = entry->prev;
		if (entry->locked <= 0 || force)
		{
			if (tree && entry->isDirty)
			{
				if (tree->WriteNode(entry->offset,entry->node) < B_OK && !force)
					// don't remove the node, if it couldn't be written
					continue;
			}

			if (entry->next)
				entry->next->prev = prev;
			if (prev)
				prev->next = entry->next;

			if (fLeastRecentlyUsed == entry)
				fLeastRecentlyUsed = prev;
			if (fMostRecentlyUsed == entry)
				fMostRecentlyUsed = entry->next;

			delete entry;
			
			if (--fCount <= targetCount)
				break;
		}

		entry = prev;
	}
}


status_t 
TreeCache::Release(Cached *entry)
{
	entry->locked--;
}


Cached *
TreeCache::Get(BPlusTree *tree,off_t offset)
{
	Cached *entry = GetFromCache(offset);
	if (entry)
	{
		entry->locked++;

		if (fMostRecentlyUsed == entry)
			return entry;

		// remove entry from cache (to insert it at top of the MRU list)
		if (entry->prev)
			entry->prev->next = entry->next;
		if (!entry->next)
			fLeastRecentlyUsed = entry->prev;
	}
	else
	{
		// ToDo: find empty (unused) entry here
		// if none was found, do:
		bplustree_node *node = (bplustree_node *)malloc(tree->fNodeSize);
		if (node == NULL)
			return NULL;

		entry = new Cached(offset,node);
		fCount++;

		if (tree->ReadNode(offset,entry->node,false) != B_OK)
		{
			delete entry;
			return NULL;
		}
	}

	if (entry)
	{
		// insert entry at the top of the MRU list
		entry->next = fMostRecentlyUsed;
		
		if (fMostRecentlyUsed)
			fMostRecentlyUsed->prev = entry;
		else
			fLeastRecentlyUsed = entry;
		fMostRecentlyUsed = entry;
		
		// remove old nodes from of the cache (if possible and necessary)
		if (fCount > fMaxInQueue
			&& fLeastRecentlyUsed)
			Flush(tree,fMaxInQueue);
	}
	return entry;
}


Cached *
TreeCache::GetFromCache(off_t offset)
{
	Cached *entry = fMostRecentlyUsed;
	while (entry)
	{
		if (entry->offset == offset)
			return entry;
		entry = entry->next;
	}
	return NULL;
}


//	#pragma mark -


BPlusTree::BPlusTree(int32 keyType,int32 nodeSize,bool allowDuplicates)
	:
	fStream(NULL),
	fHeader(NULL),
	fCache(MAX_NODES_IN_CACHE)
{
	SetTo(keyType,nodeSize,allowDuplicates);
}


BPlusTree::BPlusTree(Inode *stream,bool allowDuplicates)
	:
	fStream(NULL),
	fHeader(NULL),
	fCache(MAX_NODES_IN_CACHE)
{
	SetTo(stream,allowDuplicates);
}


BPlusTree::BPlusTree()
	:
	fStream(NULL),
	fHeader(NULL),
	fNodeSize(BPLUSTREE_NODE_SIZE),
	fAllowDuplicates(true),
	fStatus(B_NO_INIT),
	fCache(MAX_NODES_IN_CACHE)
{
}


BPlusTree::~BPlusTree()
{
	fCache.Flush(this,0,true);
}


void
BPlusTree::Initialize(int32 nodeSize)
{
	// free old data
	fCache.Flush(0,true);

	if (fHeader)
		free(fHeader);

	fStream = NULL;

	fNodeSize = nodeSize;
	fHeader = (bplustree_header *)malloc(fNodeSize);
 	memset(fHeader,0,fNodeSize);
 	
	//fCurrentNodeOffset = BPLUSTREE_NULL;
}


status_t
BPlusTree::SetTo(int32 keyType,int32 nodeSize,bool allowDuplicates)
{
	// initializes in-memory B+Tree

	Initialize(nodeSize);

	fAllowDuplicates = allowDuplicates;

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
	// initializes on-disk B+Tree

	bplustree_header header;
	
	size_t read = sizeof(bplustree_header);
	if (stream->ReadAt(0,&header,&read) < B_OK || read < sizeof(bplustree_header))
		return fStatus = read;

	// is header valid?

	if (header.magic != BPLUSTREE_MAGIC
		|| header.maximum_size != stream->Node()->data.size
		|| (header.root_node_pointer % header.node_size) != 0
		|| !header.IsValidLink(header.root_node_pointer)
		|| !header.IsValidLink(header.free_node_pointer))
		return fStatus = B_BAD_DATA;

	fAllowDuplicates = allowDuplicates;

	//if (DataStream *dataStream = dynamic_cast<DataStream *>(stream))
	{
		uint32 toMode[] = {S_STR_INDEX, S_INT_INDEX, S_UINT_INDEX, S_LONG_LONG_INDEX,
						   S_ULONG_LONG_INDEX, S_FLOAT_INDEX, S_DOUBLE_INDEX};
		uint32 mode = stream->Mode() & (S_STR_INDEX | S_INT_INDEX | S_UINT_INDEX | S_LONG_LONG_INDEX
						   | S_ULONG_LONG_INDEX | S_FLOAT_INDEX | S_DOUBLE_INDEX);
	
		if (header.data_type > BPLUSTREE_DOUBLE_TYPE
			|| toMode[header.data_type] != mode)
			return fStatus = B_BAD_TYPE;

		 // although it's in stat.h, the S_ALLOW_DUPS flag is obviously unused
		fAllowDuplicates = (stream->Mode() & (S_INDEX_DIR | 0777)) == S_INDEX_DIR;

		//printf("allows duplicates? %s\n",fAllowDuplicates ? "yes" : "no");
	}

	Initialize(header.node_size);
	fStream = stream;

	memcpy(fHeader,&header,sizeof(bplustree_header));

	CachedNode cached(this,header.root_node_pointer);
	return fStatus = cached.Node() ? B_OK : B_BAD_DATA;
}


status_t 
BPlusTree::SetStream(Inode *stream)
{
	fStream = stream;
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
	switch (fHeader->data_type)
	{
	    case BPLUSTREE_STRING_TYPE:
    	{
			int len = min_c(keyLength1,keyLength2);
			int result = strncmp((const char *)key1,(const char *)key2,len);
			
			if (result == 0)
				result = keyLength1 - keyLength2;

			return result;
		}

		case BPLUSTREE_INT32_TYPE:
			return *(int32 *)key1 - *(int32 *)key2;
			
		case BPLUSTREE_UINT32_TYPE:
		{
			if (*(uint32 *)key1 == *(uint32 *)key2)
				return 0;
			else if (*(uint32 *)key1 > *(uint32 *)key2)
				return 1;

			return -1;
		}
			
		case BPLUSTREE_INT64_TYPE:
		{
			if (*(int64 *)key1 == *(int64 *)key2)
				return 0;
			else if (*(int64 *)key1 > *(int64 *)key2)
				return 1;

			return -1;
		}

		case BPLUSTREE_UINT64_TYPE:
		{
			if (*(uint64 *)key1 == *(uint64 *)key2)
				return 0;
			else if (*(uint64 *)key1 > *(uint64 *)key2)
				return 1;

			return -1;
		}

		case BPLUSTREE_FLOAT_TYPE:
		{
			float result = *(float *)key1 - *(float *)key2;
			if (result == 0.0f)
				return 0;

			return (result < 0.0f) ? -1 : 1;
		}

		case BPLUSTREE_DOUBLE_TYPE:
		{
			double result = *(double *)key1 - *(double *)key2;
			if (result == 0.0)
				return 0;

			return (result < 0.0) ? -1 : 1;
		}
	}
	return 0;
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


status_t
BPlusTree::SeekDown(Stack<node_and_key> &stack,uint8 *key,uint16 keyLength)
{
	node_and_key nodeAndKey;
	nodeAndKey.nodeOffset = fHeader->root_node_pointer;
	nodeAndKey.keyIndex = 0;

	stack.Push(nodeAndKey);

	CachedNode cached(this);
	bplustree_node *node;
	while (stack.Pop(&nodeAndKey) && (node = cached.SetTo(nodeAndKey.nodeOffset)) != NULL)
	{
		if (node->overflow_link == BPLUSTREE_NULL)
		{
			// put the old node back on the stack
			stack.Push(nodeAndKey);
			return B_OK;
		}

		off_t pos;
		status_t status = FindKey(node,key,keyLength,&nodeAndKey.keyIndex,&pos);
		if (status == B_ERROR)
			return B_ERROR;

		// put the old node back on the stack
		stack.Push(nodeAndKey);
		
		nodeAndKey.nodeOffset = pos;
		nodeAndKey.keyIndex = 0;

		stack.Push(nodeAndKey);
	}
	return B_ERROR;
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
BPlusTree::Insert(uint8 *key,uint16 keyLength,off_t value)
{
	if (keyLength < BPLUSTREE_MIN_KEY_LENGTH || keyLength > BPLUSTREE_MAX_KEY_LENGTH)
		return B_BAD_VALUE;

	Stack<node_and_key> stack;
	if (SeekDown(stack,key,keyLength) != B_OK)
		return B_ERROR;

	CachedNode freeNode(this,BPLUSTREE_NULL);
	uint8 *key1 = (uint8 *)freeNode.Node();
//	uint8 *key2 = key1 + (fNodeSize >> 1);

	memcpy(key1,key,keyLength);

	off_t valueToInsert = value;

	// ToDo: update all tree iterators!
	//fCurrentNodeOffset = BPLUSTREE_NULL;

	node_and_key nodeAndKey;
	bplustree_node *node;
	uint32 count = 0;

	CachedNode cached(this);
	while (stack.Pop(&nodeAndKey) && (node = cached.SetTo(nodeAndKey.nodeOffset)) != NULL)
	{
		if (count++ == 0)	// first round, check for duplicate entries
		{
			status_t status = FindKey(node,key,keyLength,&nodeAndKey.keyIndex);
			if (status == B_ERROR)
				return B_ERROR;

			// is this a duplicate entry?
			if (status == B_OK && node->overflow_link == BPLUSTREE_NULL)
			{
				if (fAllowDuplicates)
					return InsertDuplicate(node,nodeAndKey.keyIndex);
				else
					return B_NAME_IN_USE;
			}
		}

		// is the node big enough to hold the pair?
		if (node->Used() + keyLength + int32(sizeof(uint16) + sizeof(off_t)) < fNodeSize)
		{
			InsertKey(node,key1,keyLength,valueToInsert,nodeAndKey.keyIndex);
			cached.SetDirty(true);

			return B_OK;
		}
		else
		{
			//puts("split!");
		}
	}

	return B_ERROR;
}


status_t
BPlusTree::Find(uint8 *key,uint16 keyLength,off_t *value)
{
	if (keyLength < BPLUSTREE_MIN_KEY_LENGTH || keyLength > BPLUSTREE_MAX_KEY_LENGTH)
		return B_BAD_VALUE;

	Stack<node_and_key> stack;
	if (SeekDown(stack,key,keyLength) != B_OK)
		return B_ERROR;

	//fCurrentNodeOffset = BPLUSTREE_NULL;

	node_and_key nodeAndKey;
	bplustree_node *node;

	CachedNode cached(this);
	if (stack.Pop(&nodeAndKey) && (node = cached.SetTo(nodeAndKey.nodeOffset)) != NULL)
	{
		status_t status = FindKey(node,key,keyLength,&nodeAndKey.keyIndex);
		if (status == B_ERROR)
			return B_ERROR;
		
		if (status == B_OK && node->overflow_link == BPLUSTREE_NULL)
		{
			*value = node->Values()[nodeAndKey.keyIndex];
			//SetCurrentNode(node,nodeAndKey.nodeOffset);
			return B_OK;
		}
	}
	return B_ENTRY_NOT_FOUND;
}


//	#pragma mark -


status_t
BPlusTree::ReadNode(off_t nodeOffset,bplustree_node *node,bool check)
{
/*printf("1: %d,%d,%d\n",
	nodeOffset > fHeader->maximum_size - fNodeSize,
	nodeOffset < 0 && nodeOffset != BPLUSTREE_NULL,
	(nodeOffset % fNodeSize) != 0);
*/
	if (nodeOffset > fHeader->maximum_size - fNodeSize
		|| nodeOffset < 0 //&& nodeOffset != BPLUSTREE_NULL
		|| (nodeOffset % fNodeSize) != 0
		|| node == NULL)
		return B_BAD_VALUE;

	size_t length = fNodeSize;
	if (fStream && fStream->ReadAt(nodeOffset,node,&length) < B_OK || length < fNodeSize)
		return B_IO_ERROR;

	if (!fStream)	// initialize node
	{
	 	node->left_link = BPLUSTREE_NULL;
	 	node->right_link = BPLUSTREE_NULL;
	 	node->overflow_link = BPLUSTREE_NULL;
	 	node->all_key_count = 0;
	 	node->all_key_length = 0;
	}

	if (check && node != NULL)
	{
		// sanity checks (links, all_key_count)
		if (!fHeader->IsValidLink(node->left_link)
			|| !fHeader->IsValidLink(node->right_link)
			|| !fHeader->IsValidLink(node->overflow_link)
			|| (int8 *)node->Values() + node->all_key_count * sizeof(off_t) > (int8 *)node + fNodeSize)
			return B_BAD_DATA;
	}
	return B_OK;
}


status_t
BPlusTree::WriteNode(off_t offset,bplustree_node *node)
{
	// not yet implemented
	return B_ERROR;
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


void
TreeIterator::SetCurrentNode(bplustree_node *node,off_t offset,int8 to)
{
	fCurrentNodeOffset = offset;
	fCurrentKey = to == BPLUSTREE_BEGIN ? -1 : node->all_key_count;
	fDuplicateNode = 0;
}


status_t
TreeIterator::Goto(int8 to)
{
	Stack<off_t> stack;
	stack.Push(fTree->fHeader->root_node_pointer);

	CachedNode cached(fTree);
	bplustree_node *node;
	off_t pos;
	while (stack.Pop(&pos) && (node = cached.SetTo(pos)) != NULL)
	{
		// is the node a leaf node?
		if (node->overflow_link == BPLUSTREE_NULL)
		{
			SetCurrentNode(node,pos,to);

			return B_OK;
		}

		stack.Push(to == BPLUSTREE_END || node->all_key_count == 0 ? node->overflow_link : *node->Values());
	}
	return B_ERROR;
}


status_t
TreeIterator::Traverse(int8 direction,void *key,uint16 *keyLength,uint16 maxLength,off_t *value)
{
	if (fCurrentNodeOffset == BPLUSTREE_NULL
		&& Goto(direction == BPLUSTREE_FORWARD ? BPLUSTREE_BEGIN : BPLUSTREE_END) < B_OK) 
		return B_ERROR;

	CachedNode cached(fTree);
	bplustree_node *node;

	if (fDuplicateNode)
	{
		// regardless of traverse direction the duplicates are always presented in
		// the same order; since they are all considered as equal, this shouldn't
		// cause any problems

		if (fDuplicate < fNumDuplicates
			&& (node = cached.SetTo(bplustree_node::FragmentOffset(fDuplicateNode))) != NULL)
		{
			*value = node->DuplicateAt(fDuplicateNode,fIsFragment,fDuplicate++);
			if (!fIsFragment && fDuplicate == fNumDuplicates)
			{
				if (node->right_link != BPLUSTREE_NULL)
				{
					fDuplicateNode = node->right_link;
					if ((node = cached.SetTo(bplustree_node::FragmentOffset(fDuplicateNode))) != NULL)
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
		return B_ERROR;

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
				return B_ERROR;

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
		return B_ERROR; //B_ENTRY_NOT_FOUND;

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

		node = cached.SetTo(bplustree_node::FragmentOffset(fDuplicateNode));
		if (node == NULL)
			return B_ERROR;

		fIsFragment = type == BPLUSTREE_DUPLICATE_FRAGMENT;

		fNumDuplicates = node->CountDuplicates(offset,fIsFragment);
		if (fNumDuplicates)
		{
			fDuplicate = 0;
			offset = node->DuplicateAt(offset,fIsFragment,fDuplicate++);
		}
		else
		{
			// shouldn't happen, but we're dealing here with corrupt disks...
			fDuplicateNode = 0;
			offset = 0;
		}
	}
	*value = offset;

	return B_OK;
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

