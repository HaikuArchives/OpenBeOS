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
			fVolume(volume),
			fCount(0)
		{
		}

		Transaction(Volume *volume,block_run refRun)
			:
			fVolume(volume),
			fCount(0)
		{
		}

		~Transaction();

		status_t WriteBlocks(off_t blockNumber,const uint8 *buffer,size_t numBlocks = 1);
		void Done();

	//protected:
		Volume	*fVolume;
		int32	fCount;
};

#endif	/* JOURNAL_H */
