#ifndef INODE_H
#define INODE_H
/* Inode - inode access functions
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include <KernelExport.h>
#ifdef USER
#	include "myfs.h"
#	include <stdio.h>
#endif

#ifndef _IMPEXP_KERNEL
#	define _IMPEXP_KERNEL
#endif

extern "C" {
	#include <lock.h>
	#include <cache.h>
}

#include <string.h>

#include "Volume.h"
#include "Journal.h"
#include "Lock.h"


class BPlusTree;
class TreeIterator;


enum inode_type {
	S_DIR		= S_IFDIR,
	S_FILE		= S_IFREG,
	S_SYMLINK	= S_IFLNK
};


// The CachedBlock class is completely implemented as inlines.
// It should be used when cache single blocks to make sure they
// will be properly released after use (and it's also very
// convenient to use them).

class CachedBlock {
	public:
		CachedBlock(Volume *volume)
			:
			fVolume(volume),
			fBlock(NULL)
		{
		}

		CachedBlock(Volume *volume,off_t block)
			:
			fVolume(volume),
			fBlock(NULL)
		{
			SetTo(block);
		}
		
		CachedBlock(Volume *volume,block_run run)
			:
			fVolume(volume),
			fBlock(NULL)
		{
			SetTo(volume->ToBlock(run));
		}
		
		~CachedBlock()
		{
			Unset();
		}

		void Unset()
		{
			if (fBlock != NULL)
				release_block(fVolume->Device(),fBlockNumber);
		}

		uint8 *SetTo(off_t block)
		{
			Unset();
			fBlockNumber = block;
			return fBlock = (uint8 *)get_block(fVolume->Device(),block,fVolume->BlockSize());
		}
		
		uint8 *SetTo(block_run run)
		{
			return SetTo(fVolume->ToBlock(run));
		}

		status_t WriteBack(Transaction *transaction)
		{
			if (transaction == NULL || fBlock == NULL)
				RETURN_ERROR(B_BAD_VALUE);

			return transaction->WriteBlocks(fBlockNumber,fBlock);
		}

		uint8 *Block() const { return fBlock; }
		off_t BlockNumber() const { return fBlockNumber; }

	protected:
		Volume	*fVolume;
		off_t	fBlockNumber;
		uint8	*fBlock;
};


class Inode : public CachedBlock {
	public:
		Inode(Volume *volume,vnode_id id,uint8 reenter = 0);
		~Inode();

		ReadWriteLock &Lock() { return fLock; }
		bfs_inode *Node() const { return (bfs_inode *)fBlock; }
		vnode_id ID() const { return fVolume->ToVnode(fBlockNumber); }

		mode_t Mode() const { return Node()->mode; }
		int32 Flags() const { return Node()->flags; }
		bool IsDirectory() const { return S_ISDIR(Node()->mode); }
		bool IsSymLink() const { return S_ISLNK(Node()->mode); }
		
		off_t Size() const { return Node()->data.size; }

		block_run BlockRun() const { return Node()->inode_num; }
		block_run Parent() const { return Node()->parent; }
		block_run Attributes() const { return Node()->attributes; }
		Volume *GetVolume() const { return fVolume; }

		status_t InitCheck();

		bool CheckPermissions(int accessMode) const;

		status_t GetNextSmallData(small_data **smallData) const;
		small_data *FindSmallData(const char *name) const;
		const char *Name() const;

		Inode *GetAttribute(const char *name);
		void ReleaseAttribute(Inode *attribute);

		// for directories only:
		status_t GetTree(BPlusTree **);

		status_t FindBlockRun(off_t pos,block_run &run,off_t &offset);

		status_t ReadAt(off_t pos,void *buffer,size_t *length);
		status_t WriteAt(Transaction *transaction,off_t pos,void *buffer,size_t *length);
	
	private:
		BPlusTree		*fTree;
		Inode			*fAttributes;
		ReadWriteLock	fLock;
};


class AttributeIterator {
	public:
		AttributeIterator(Inode *inode);
		~AttributeIterator();
		
		status_t Rewind();
		status_t GetNext(char *name,size_t *length,uint32 *type,vnode_id *id);

	private:
		small_data	*fCurrentSmallData;
		Inode		*fInode, *fAttributes;
		TreeIterator *fIterator;
		void		*fBuffer;
};


/**	Converts the "omode", the open flags given to bfs_open(), into
 *	access modes, e.g. since O_RDONLY requires read access to the
 *	file, it will be converted to R_OK.
 */

inline int oModeToAccess(int omode)
{
	omode &= O_RWMASK;
	if (omode == O_RDONLY)
		return R_OK;
	else if (omode == O_WRONLY)
		return W_OK;
	
	return R_OK | W_OK;
}

#endif	/* INODE_H */
