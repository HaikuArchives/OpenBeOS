/* Journal - transaction and logging
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include "Journal.h"
#include "Inode.h"
#include "Debug.h"
#include "cpp.h"


int32 gLogEntries = 0;


Journal::Journal(Volume *volume)
	:
	fVolume(volume),
	fLock("bfs journal"),
	fOwner(NULL),
	fArray(volume->BlockSize()),
	fLogSize(volume->Log().length)
{
}


Journal::~Journal()
{
}


status_t
Journal::Lock(Transaction *owner)
{
	if (owner == fOwner)
		return B_OK;

	status_t status = fLock.Lock();
	if (status == B_OK)
		fOwner = owner;
	
	return B_OK;
}


void
Journal::Unlock(Transaction *owner,bool success)
{
	if (owner != fOwner)
		return;

	fOwner = NULL;
	fLock.Unlock();

	TransactionDone(success);
}


void
Journal::blockNotify(off_t blockNumber,size_t numBlocks,void *arg)
{
	log_entry *logEntry = (log_entry *)arg;
	PRINT(("block info called with: block %Ld, %lu blocks, log_entry = %p\n",blockNumber,numBlocks,arg));

	logEntry->num_blocks -= numBlocks;
	if (logEntry->num_blocks <= 0) {
		Journal *journal = logEntry->journal;

		PRINT(("numBlocks = %ld, transaction completed, %ld left!\n",logEntry->num_blocks,--gLogEntries));

		// Set log_start pointer if possible...
		// ToDo: locking before changing the log_entry list
		disk_super_block &superBlock = journal->fVolume->SuperBlock();

		if (logEntry == journal->fEntries.head) {
			int32 length;
			if (logEntry->Next() != NULL) {
				length = logEntry->next->start - logEntry->start;
			} else
				length = logEntry->length;

			superBlock.log_start = (superBlock.log_start + length) % journal->fLogSize;
			PRINT(("log start bumped to %ld\n",superBlock.log_start));
		} else if (logEntry == journal->fEntries.last) {
			// since we're not the first entry, there has to be an entry before us
			superBlock.log_end = logEntry->start;
			PRINT(("log end bumped to %ld\n",superBlock.log_end));
		}
		logEntry->Remove();
		free(logEntry);
	}
}


status_t
Journal::WriteLogEntry()
{
	sorted_array *array = fArray.Array();
	if (array == NULL || array->count == 0)
		return B_OK;

	off_t logOffset = fVolume->ToBlock(fVolume->Log());
	off_t logStart = fVolume->SuperBlock().log_end;
	int32 logPosition = logStart % fLogSize;

	PRINT(("write log entry, start = %Ld\n",logStart));

	// Write disk block array

	uint8 *arrayBlock = (uint8 *)array;
	int32 size = fArray.Size();

	while (size > 0) {
		cached_write(fVolume->Device(),logOffset + logPosition,arrayBlock,1,fVolume->BlockSize());

		logPosition = (logPosition + 1) % fLogSize;
		arrayBlock += fVolume->BlockSize();
		size -= fVolume->BlockSize();
	}

	// Write logged blocks into the log

	CachedBlock cached(fVolume);
	for (int32 i = 0;i < array->count;i++) {
		uint8 *block = cached.SetTo(array->values[i]);
		if (block == NULL)
			return B_IO_ERROR;

		cached_write(fVolume->Device(),logOffset + logPosition,block,1,fVolume->BlockSize());
		logPosition = (logPosition + 1) % fLogSize;
	}
	
	log_entry *logEntry = (log_entry *)malloc(sizeof(log_entry));
	if (logEntry != NULL) {
		logEntry->start = logStart;
		logEntry->length = array->count + fArray.Size() / fVolume->BlockSize();
		logEntry->num_blocks = array->count;
		logEntry->journal = this;

		fEntries.Add(logEntry);
		fCurrent = logEntry;

		// ToDo: remove counter
		gLogEntries++;
		set_blocks_info(fVolume->Device(),&array->values[0],array->count,blockNotify,logEntry);
	}

	// Flush blocks in log

	if (logPosition > fLogSize)
		flush_blocks(fVolume->Device(),logOffset + logStart,logPosition);
	else {
		// log "overflow"
		flush_blocks(fVolume->Device(),logOffset + logStart,fLogSize - logStart);
		flush_blocks(fVolume->Device(),logOffset,logPosition);
	}
	fArray.MakeEmpty();
	
	// ToDo: set log_end pointer...
	fVolume->SuperBlock().log_end = logPosition;
	PRINT(("log end moved to %ld\n",logPosition));
	
}


status_t
Journal::TransactionDone(bool success)
{
	// ToDo: batch up transactions here...

	return WriteLogEntry();
}


status_t
Journal::LogBlocks(off_t blockNumber,const uint8 *buffer,size_t numBlocks)
{
	for (;numBlocks-- > 0;blockNumber++,buffer += fVolume->BlockSize()) {
		if (fArray.Find(blockNumber) >= 0)
			continue;

		// ToDo: do something if the log is full
		// ... like flushing older transactions

		fArray.Insert(blockNumber);
		status_t status = cached_write_locked(fVolume->Device(),blockNumber,buffer,1,fVolume->BlockSize());
		if (status < B_OK)
			return status;
	}
	return B_OK;
}


//	#pragma mark -


Transaction::Transaction(Volume *volume, off_t refBlock)
{
	Start(volume,refBlock);
}


Transaction::Transaction(Volume *volume, block_run refRun)
{
	Start(volume,volume->ToBlock(refRun));
}


Transaction::Transaction()
	:
	fVolume(NULL),
	fJournal(NULL)
{
}


Transaction::~Transaction()
{
	PRINT(("######################################### transaction done!\n"));
	if (fJournal)
		fJournal->Unlock(this,false);
}


void 
Transaction::Start(Volume *volume,off_t refBlock)
{
	fVolume = volume;
	fJournal = volume->GetJournal(refBlock);
	fJournal->Lock(this);
	PRINT(("***************************************** transaction started!\n"));
}


void
Transaction::Done()
{
	fJournal->Unlock(this,true);
}


status_t
Transaction::WriteBlocks(off_t blockNumber,const uint8 *buffer,size_t numBlocks)
{
	if (fVolume == NULL)
		return B_NO_INIT;

	return fJournal->LogBlocks(blockNumber,buffer,numBlocks);
	//status_t status = cached_write/*_locked*/(fVolume->Device(),blockNumber,buffer,numBlocks,fVolume->BlockSize());
	//return status;
}

