/* Index - index access functions
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include "Debug.h"
#include "cpp.h"
#include "Index.h"
#include "Volume.h"
#include "Inode.h"
#include "BPlusTree.h"

#include <TypeConstants.h>


Index::Index(Volume *volume)
	:
	fVolume(volume),
	fNode(NULL)
{
}


Index::~Index()
{
	if (fNode == NULL)
		return;

	put_vnode(fVolume->ID(),fNode->ID());
}


void
Index::Unset()
{
	if (fNode == NULL)
		return;

	put_vnode(fVolume->ID(),fNode->ID());
	fNode = NULL;
}


status_t 
Index::SetTo(const char *name)
{
	// remove the old node, if the index is set for the second time
	Unset();

	Inode *indices = fVolume->IndicesNode();
	if (indices == NULL)
		return B_ENTRY_NOT_FOUND;

	BPlusTree *tree;
	if (indices->GetTree(&tree) != B_OK)
		return B_BAD_VALUE;

	vnode_id id;
	status_t status = tree->Find((uint8 *)name,(uint16)strlen(name),&id);
	if (status != B_OK)
		return status;

	if (get_vnode(fVolume->ID(),id,(void **)&fNode) != B_OK)
		return B_ENTRY_NOT_FOUND;

	if (fNode == NULL) {
		FATAL(("fatal error at Index::InitCheck(), get_vnode() returned NULL pointer\n"));
		put_vnode(fVolume->ID(),id);
		return B_ERROR;
	}

	return B_OK;
}


uint32 
Index::Type()
{
	if (fNode == NULL)
		return 0;

	switch (fNode->Mode() & (S_STR_INDEX | S_INT_INDEX | S_UINT_INDEX | S_LONG_LONG_INDEX |
							 S_ULONG_LONG_INDEX | S_FLOAT_INDEX | S_DOUBLE_INDEX)) {
		case S_INT_INDEX:
			return B_INT32_TYPE;
		case S_UINT_INDEX:
			return B_UINT32_TYPE;
		case S_LONG_LONG_INDEX:
			return B_INT64_TYPE;
		case S_ULONG_LONG_INDEX:
			return B_UINT64_TYPE;
		case S_FLOAT_INDEX:
			return B_FLOAT_TYPE;
		case S_DOUBLE_INDEX:
			return B_DOUBLE_TYPE;
		case S_STR_INDEX:
			return B_STRING_TYPE;
	}
	FATAL(("index has unknown type!\n"));
	return 0;
}


size_t
Index::KeySize()
{
	if (fNode == NULL)
		return 0;
	
	int32 mode = fNode->Mode() & (S_STR_INDEX | S_INT_INDEX | S_UINT_INDEX | S_LONG_LONG_INDEX |
								  S_ULONG_LONG_INDEX | S_FLOAT_INDEX | S_DOUBLE_INDEX);

	if (mode == S_STR_INDEX)
		// string indices don't have a fixed key size
		return 0;

	switch (mode) {
		case S_INT_INDEX:
		case S_UINT_INDEX:
			return sizeof(int32);
		case S_LONG_LONG_INDEX:
		case S_ULONG_LONG_INDEX:
			return sizeof(int64);
		case S_FLOAT_INDEX:
			return sizeof(float);
		case S_DOUBLE_INDEX:
			return sizeof(double);
	}
	FATAL(("index has unknown type!\n"));
	return 0;
}


status_t 
Index::InsertName(Transaction *transaction,const char *name,off_t id)
{
	return UpdateName(transaction,NULL,name,id);
}


status_t 
Index::RemoveName(Transaction *transaction,const char *name,off_t id)
{
	return UpdateName(transaction,name,NULL,id);
}


/**	Updates the "name" index, the oldName will be removed from, the newName
 *	inserted into the tree.
 *	If the method returns B_BAD_INDEX, it means the index couldn't be found -
 *	the most common reason will be that the index doesn't exist.
 *	You may not want to let the whole transaction fail because of that.
 */

status_t 
Index::UpdateName(Transaction *transaction,const char *oldName, const char *newName,off_t id)
{
	if (oldName == NULL && newName == NULL)
		return B_BAD_VALUE;

	status_t status = SetTo("name");
	if (status < B_OK)
		return B_BAD_INDEX;

	BPlusTree *tree;
	if ((status = Node()->GetTree(&tree)) < B_OK)
		return status;

	// remove the old name
	
	if (oldName != NULL) {
		status = tree->Remove(transaction,(const uint8 *)oldName,strlen(oldName),id);
		if (status == B_ENTRY_NOT_FOUND) {
			FATAL(("Name \"%s\" should be in index, but was not found (inode at %Ld)!\n",oldName,id));
		} else if (status < B_OK)
			return status;
	}
	
	// add the new name
	
	if (newName != NULL)
		status = tree->Insert(transaction,(const uint8 *)newName,strlen(newName),id);

	return status;
}

