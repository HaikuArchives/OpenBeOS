#ifndef JOURNAL_H
#define JOURNAL_H
/* Journal - transaction and logging
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

#include "Volume.h"


// For now, that's only a dumb class that does more or less nothing
// else than writing the blocks directly to the real location.
// It doesn't yet use logging.

class Transaction {
	public:
		Transaction(Volume *volume,off_t refBlock)
			:
			fVolume(volume)
		{
		}

		~Transaction()
		{
		}

		void Done()
		{
		}

		status_t WriteBlocks(off_t blockNumber,const uint8 *buffer,size_t numBlocks = 1)
		{
			return cached_write(fVolume->Device(),blockNumber,buffer,numBlocks,fVolume->BlockSize());
		}

	protected:
		Volume	*fVolume;
};

#endif	/* JOURNAL_H */
