/* Journal - transaction and logging
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/


#include "Journal.h"
#include "Inode.h"
#include "Debug.h"
#include "cpp.h"


// ToDo: remove this counter
int32 gLogEntries = 0;


Journal::Journal(Volume *volume)
	:
	fVolume(volume),
	fLock("bfs journal"),
	fOwner(NULL),
	fOwningThread(-1),
	fArray(volume->BlockSize()),
	fLogSize(volume->Log().length),
	fMaxTransactionSize(128), //fLogSize / 4),
	fUsed(0),
	fTransactionsInEntry(0)
{
}


Journal::~Journal()
{
	FlushLog();
}


status_t
Journal::InitCheck()
{
	if (fVolume->LogStart() != fVolume->LogEnd()) {
		if (fVolume->SuperBlock().flags != SUPER_BLOCK_DISK_DIRTY)
			FATAL(("log_start and log_end differ, but disk is marked clean - trying to replay log...\n"));

		return ReplayLog();
	}

	return B_OK;
}


status_t
Journal::Lock(Transaction *owner)
{
	if (owner == fOwner)
		return B_OK;

	status_t status = fLock.Lock();
	if (status == B_OK) {
		fOwner = owner;
		fOwningThread = find_thread(NULL);
	}
	return B_OK;
}


void
Journal::Unlock(Transaction *owner,bool success)
{
	if (owner != fOwner)
		return;

	TransactionDone(success);

	fOwner = NULL;
	fOwningThread = -1;
	fLock.Unlock();
}


status_t
Journal::CheckLogEntry(int32 count,off_t *array)
{
	// ToDo: check log entry integrity (block numbers and entry size)
	PRINT(("Log entry has %ld entries (%Ld)\n",count));
	return B_OK;
}


status_t
Journal::ReplayLogEntry(int32 *_start)
{
	PRINT(("ReplayLogEntry(start = %u)\n",*_start));

	off_t logOffset = fVolume->ToBlock(fVolume->Log());
	off_t arrayBlock = (*_start % fLogSize) + fVolume->ToBlock(fVolume->Log());
	int32 blockSize = fVolume->BlockSize();
	int32 count = 1,valuesInBlock = blockSize / sizeof(off_t);
	int32 numArrayBlocks;
	off_t blockNumber;
	bool first = true;

	CachedBlock cached(fVolume);
	while (count > 0) {
		off_t *array = (off_t *)cached.SetTo(arrayBlock);
		if (array == NULL)
			return B_IO_ERROR;

		int32 index = 0;
		if (first) {
			count = array[0];
			if (count < 1 || count >= fLogSize)
				return B_BAD_DATA;

			first = false;
			
			numArrayBlocks = ((count + 1) * sizeof(off_t) + blockSize - 1) / blockSize;
			blockNumber = (*_start + numArrayBlocks) % fLogSize;
				// first real block in this log entry
			*_start += count;
			index++;
				// the first entry in the first block is the number
				// of blocks in that log entry
		}
		(*_start)++;

		if (CheckLogEntry(count,array + 1) < B_OK)
			return B_BAD_DATA;

		CachedBlock cachedCopy(fVolume);
		for (;index < valuesInBlock && count-- > 0;index++) {
			PRINT(("replay block %Ld in log at %Ld!\n",array[index],blockNumber));

			uint8 *copy = cachedCopy.SetTo(logOffset + blockNumber);
			if (copy == NULL)
				RETURN_ERROR(B_IO_ERROR);

			ssize_t written = write_pos(fVolume->Device(),array[index] * blockSize,copy,blockSize);
			if (written != blockSize)
				RETURN_ERROR(B_IO_ERROR);

			blockNumber = (blockNumber + 1) % fLogSize;
		}
		arrayBlock++;
		if (arrayBlock > fVolume->ToBlock(fVolume->Log()) + fLogSize)
			arrayBlock = fVolume->ToBlock(fVolume->Log());
	}
	return B_OK;
}


/**	Replays all log entries - this will put the disk into a
 *	consistent and clean state, if it was not correctly unmounted
 *	before.
 *	This method is called by Journal::InitCheck() if the log start
 *	and end pointer don't match.
 */

status_t
Journal::ReplayLog()
{
	INFORM(("Replay log, disk was not correctly unmounted...\n"));

	int32 start = fVolume->LogStart();
	int32 lastStart = -1;
	while (true) {

		// stop if the log is completely flushed
		if (start == fVolume->LogEnd())
			break;

		if (start == lastStart) {
			// strange, flushing the log hasn't changed the log_start pointer
			return B_ERROR;
		}
		lastStart = start;

		status_t status = ReplayLogEntry(&start);
		if (status < B_OK) {
			FATAL(("replaying log entry from %u failed: %s\n",start,status));
			return B_ERROR;
		}
		start = start % fLogSize;
	}
	
	PRINT(("replaying worked fine!\n"));
	fVolume->SuperBlock().log_start = fVolume->LogEnd();
	fVolume->SuperBlock().flags = SUPER_BLOCK_DISK_CLEAN;
	return fVolume->WriteSuperBlock();
}


void
Journal::blockNotify(off_t blockNumber,size_t numBlocks,void *arg)
{
	log_entry *logEntry = (log_entry *)arg;

	logEntry->cached_blocks -= numBlocks;
	if (logEntry->cached_blocks <= 0) {
		Journal *journal = logEntry->journal;
		disk_super_block &superBlock = journal->fVolume->SuperBlock();

		// Set log_start pointer if possible...

		if (logEntry == journal->fEntries.head) {
			int32 length;
			if (logEntry->Next() != NULL) {
				length = logEntry->next->start - logEntry->start;
			} else
				length = logEntry->length;

			superBlock.log_start = (superBlock.log_start + length) % journal->fLogSize;
		} else if (logEntry == journal->fEntries.last) {
			// since we're not the first entry, there has to be an entry before us
			superBlock.log_end = logEntry->start;
		}
		journal->fUsed -= logEntry->length;
			
		journal->fEntriesLock.Lock();
		logEntry->Remove();
		journal->fEntriesLock.Unlock();

		free(logEntry);

		// update the super block, and change the disk's state, if necessary

		if (superBlock.log_start == superBlock.log_end)
			superBlock.flags = SUPER_BLOCK_DISK_CLEAN;

		journal->fVolume->WriteSuperBlock();
	}
}


status_t
Journal::WriteLogEntry()
{
	sorted_array *array = fArray.Array();
	if (array == NULL || array->count == 0)
		return B_OK;

	off_t logOffset = fVolume->ToBlock(fVolume->Log());
	off_t logStart = fVolume->LogEnd();
	int32 logPosition = logStart % fLogSize;

	// Write disk block array

	uint8 *arrayBlock = (uint8 *)array;

	for (int32 size = fArray.BlocksUsed();size-- > 0;) {
		cached_write(fVolume->Device(),logOffset + logPosition,arrayBlock,1,fVolume->BlockSize());

		logPosition = (logPosition + 1) % fLogSize;
		arrayBlock += fVolume->BlockSize();
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
		logEntry->length = TransactionSize();
		logEntry->cached_blocks = array->count;
		logEntry->journal = this;

		fEntriesLock.Lock();
		fEntries.Add(logEntry);
		fEntriesLock.Unlock();

		fCurrent = logEntry;
		fUsed += logEntry->length;

		// ToDo: remove counter
		gLogEntries++;
		set_blocks_info(fVolume->Device(),&array->values[0],array->count,blockNotify,logEntry);
	}

	// Flush blocks in log (writes the log entry to disk)

	if (logPosition > fLogSize)
		flush_blocks(fVolume->Device(),logOffset + logStart,logPosition);
	else {
		// log "overflow"
		flush_blocks(fVolume->Device(),logOffset + logStart,fLogSize - logStart);
		flush_blocks(fVolume->Device(),logOffset,logPosition);
	}
	fArray.MakeEmpty();

	// Update the log end pointer in the super block
	fVolume->SuperBlock().flags = SUPER_BLOCK_DISK_DIRTY;
	fVolume->SuperBlock().log_end = logPosition;
	fVolume->WriteSuperBlock();
}


status_t
Journal::FlushLogEntry(uint32 start)
{
	CachedBlock cached(fVolume);
	off_t arrayBlock = (start % fLogSize) + fVolume->ToBlock(fVolume->Log());
	int32 count = 1,valuesInBlock = fVolume->BlockSize() / sizeof(off_t);
	bool first = true;

	while (count > 0) {
		off_t *array = (off_t *)cached.SetTo(arrayBlock);
		if (array == NULL)
			return B_IO_ERROR;

		int32 index = 0;
		if (first) {
			count = array[0];
			if (count < 1 || count >= fLogSize)
				return B_BAD_DATA;

			first = false;
			index++;
				// the first entry in the first block is the number
				// of blocks in that log entry
		}

		for (;index < valuesInBlock && count-- > 0;index++) {
			// batch following near blocks together
			off_t first = array[index];
			int32 numBlocks = 1;
			while (count > 0 && index < valuesInBlock - 1
				&& array[index + 1] - array[index] < 16) {
				numBlocks += array[index + 1] - array[index];
				index++;
				count--;
			}

			flush_blocks(fVolume->Device(),first,numBlocks);
		}
		arrayBlock++;
		if (arrayBlock > fVolume->ToBlock(fVolume->Log()) + fLogSize)
			arrayBlock = fVolume->ToBlock(fVolume->Log());
	}
	return B_OK;
}


status_t 
Journal::FlushLog()
{
	// if the current transaction can be flushed (i.e. there are
	// no new blocks in the current transaction), write the log
	// entry to disk
	if (!fHasChangedBlocks) {
		fTransactionsInEntry = 0;
		fHasChangedBlocks = false;

		status_t status = WriteLogEntry();
		if (status < B_OK)
			FATAL(("writing current log entry failed: %s\n",status));
	}

	// flush all log entries - this will put the disk
	// in a consistent and clean state

	int32 lastStart = -1;
	while (true) {
		int32 start = fVolume->LogStart();

		// stop if the log is completely flushed
		if (start == fVolume->LogEnd())
			return B_OK;

		if (start == lastStart) {
			// strange, flushing the log hasn't changed the log_start pointer
			return B_ERROR;
		}
		lastStart = start;

		status_t status = FlushLogEntry(start);
		if (status < B_OK) {
			FATAL(("flushing log entry from %u failed: %s\n",start,status));
			return B_ERROR;
		}
	}
}


status_t
Journal::TransactionDone(bool success)
{
	if (!success && fTransactionsInEntry == 0) {
		// we can safely abort the transaction
		// ToDo: abort the transaction
		PRINT(("should abort transaction...\n"));
	}

	// Up to a maximum size, we will just batch several
	// transactions together to improve speed
	if (TransactionSize() < fMaxTransactionSize) {
		fTransactionsInEntry++;
		fHasChangedBlocks = false;
		return B_OK;
	}

	fTransactionsInEntry = 0;
	fHasChangedBlocks = false;

	return WriteLogEntry();
}


status_t
Journal::LogBlocks(off_t blockNumber,const uint8 *buffer,size_t numBlocks)
{
	fHasChangedBlocks = true;
	int32 blockSize = fVolume->BlockSize();

	for (;numBlocks-- > 0;blockNumber++,buffer += blockSize) {
		if (fArray.Find(blockNumber) >= 0)
			continue;

		// Insert the block into the transaction's array, and write the changes
		// back into the locked cache buffer
		fArray.Insert(blockNumber);
		status_t status = cached_write_locked(fVolume->Device(),blockNumber,buffer,1,blockSize);
		if (status < B_OK)
			return status;
	}

	// If necessary, flush the log, so that we have enough space for this transaction

	int32 sizeNeeded = TransactionSize();
	while (fVolume->LogEnd() < fVolume->LogStart() && fVolume->LogEnd() + sizeNeeded >= fVolume->LogStart()
			|| fVolume->LogEnd() > fVolume->LogStart() && (fVolume->LogEnd() + sizeNeeded) % fLogSize >= fVolume->LogStart()) {
		// flush older transactions if the log is full
		off_t start = fVolume->LogStart();
		FlushLogEntry(start);

		if (fVolume->LogStart() == fVolume->LogEnd())
			break;

		if (start == fVolume->LogStart()) {
			PRINT(("used = %ld blocks, transaction size = %ld, numBlocks = %ld\n",fUsed,TransactionSize(),numBlocks));
			PRINT(("log_start = %Ld, log_end = %Ld\n",fVolume->SuperBlock().log_start,fVolume->SuperBlock().log_end));
			return B_DEVICE_FULL;
		}	
	}

	return B_OK;
}


//	#pragma mark -


status_t 
Transaction::Start(Volume *volume,off_t refBlock)
{
	// has it already been started?
	if (fJournal != NULL)
		return B_OK;

	fJournal = volume->GetJournal(refBlock);
	if (fJournal != NULL && fJournal->Lock(this) == B_OK)
		return B_OK;

	fJournal = NULL;
	return B_ERROR;
}

