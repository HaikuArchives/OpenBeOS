#ifndef INODE_H
#define INODE_H
/* Inode - inode access functions
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
*/


#include <KernelExport.h>
#ifdef USER
#	define dprintf printf
#	include "myfs.h"
#	include <stdio.h>
#endif

extern "C" {
	#define _IMPEXP_KERNEL
	#include <lock.h>
	#include <cache.h>
}

#include "Volume.h"


class BPlusTree;


class CachedBlock
{
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

		uint8 *Block() const { return fBlock; }

	protected:
		Volume	*fVolume;
		off_t	fBlockNumber;
		uint8	*fBlock;
};


class Inode : public CachedBlock
{
	public:
		Inode(Volume *volume,vnode_id id,uint8 reenter = 0);
		~Inode();

		mode_t Mode() const { return Node()->mode; }
		bool IsDirectory() const { return S_ISDIR(Node()->mode); }
		bool IsSymLink() const { return S_ISLNK(Node()->mode); }

		bfs_inode *Node() const { return (bfs_inode *)fBlock; }
		vnode_id VnodeID() const { return fVolume->ToVnode(fBlockNumber); }

		status_t InitCheck();

		// for directories only:
		status_t GetTree(BPlusTree **);

		status_t FindBlockRun(off_t pos,block_run &run,off_t &offset);

		status_t ReadAt(off_t pos,void *buffer,size_t *length);
		status_t WriteAt(off_t pos,void *buffer,size_t *length);
	
	private:
		BPlusTree *fTree;
};

#endif	/* INODE_H */
