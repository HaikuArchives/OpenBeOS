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
#include "Chain.h"
#include "Utility.h"


struct log_entry : node<log_entry> {
	uint16		start;
	uint16		length;
	uint32		num_blocks;
	Journal		*journal;
};


class Journal {
	public:
		Journal(Volume *);
		~Journal();
		
		status_t Lock(Transaction *owner);
		void Unlock(Transaction *owner,bool success);

		status_t WriteLogEntry();
		status_t LogBlocks(off_t blockNumber,const uint8 *buffer, size_t numBlocks);
		
		Transaction *CurrentTransaction() const { return fOwner; }

	private:
		friend log_entry;

		static void blockNotify(off_t blockNumber, size_t numBlocks, void *arg);
		status_t TransactionDone(bool success);

		Volume		*fVolume;
		Benaphore	fLock;
		Transaction *fOwner;
		BlockArray	fArray;
		uint32		fLogSize;
		list<log_entry>	fEntries;
		log_entry	*fCurrent;
};


// For now, that's only a dumb class that does more or less nothing
// else than writing the blocks directly to the real location.
// It doesn't yet use logging.

class Transaction {
	public:
		Transaction(Volume *volume,off_t refBlock);
		Transaction(Volume *volume,block_run refRun);
		Transaction();
		~Transaction();

		void Start(Volume *volume,off_t refBlock);
		void Done();

		status_t WriteBlocks(off_t blockNumber,const uint8 *buffer,size_t numBlocks = 1);

		Volume	*GetVolume() { return fVolume; }

	protected:
		Volume	*fVolume;
		Journal	*fJournal;
};

#endif	/* JOURNAL_H */
