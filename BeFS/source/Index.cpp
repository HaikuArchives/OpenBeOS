/* Index - index access functions
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
*/


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


status_t 
Index::SetTo(const char *name)
{
	// doesn't support to be set to different indices right now
	if (fNode != NULL)
		return B_OK;

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
		dprintf("bfs: fatal error at Index::InitCheck(), get_vnode() returned NULL pointer\n");
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
	dprintf("bfs: index has unknown type!\n");
	return 0;
}

