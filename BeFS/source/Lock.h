#ifndef LOCK_H
#define LOCK_H
/* Lock - read/write lock implementation
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** Roughly based on a sample code from Nathan Schrenk.
**
** This file may be used under the terms of the OpenBeOS License.
*/


#include <KernelExport.h>


// These is a "fast" implementation of a sinlge writer/many reader
// locking scheme. It's fast because it uses the benaphore idea
// to do lazy semaphore locking - in most cases it will only do
// simple integer arithmetic.

#define MAX_READERS 100000

// Timeouts:
// It may be a good idea to have timeouts for the WriteLocked class,
// in case something went wrong - we'll see if this is necessary,
// but it would be a somewhat poor work-around a deadlock...
// But the only real problem with timeouts could be for things like
// "chkbfs" - because such a tool may need to lock for some more time


class ReadWriteLock {
	public:
		ReadWriteLock()
		{
			fSemaphore = create_sem(0, "bfs r/w lock");
			fCount = MAX_READERS;
		}

		~ReadWriteLock()
		{
			delete_sem(fSemaphore);
		}

		status_t InitCheck()
		{
			if (fSemaphore < B_OK)
				return fSemaphore;
			
			return B_OK;
		}

	private:
		friend class ReadLocked;
		friend class WriteLocked;

		sem_id	fSemaphore;
		int32	fCount;
};


class ReadLocked {
	public:
		ReadLocked(ReadWriteLock &lock)
			: fLock(lock)
		{
			if (atomic_add(&lock.fCount, -1) <= 0)
				acquire_sem(lock.fSemaphore);
		}
		
		~ReadLocked()
		{
			if (atomic_add(&fLock.fCount, 1) < 0)
				release_sem(fLock.fSemaphore);
		}
	
	private:
		ReadWriteLock	&fLock;
};


class WriteLocked {
	public:
		WriteLocked(ReadWriteLock &lock)
			: fLock(lock)
		{
			int32 readers = atomic_add(&lock.fCount, -MAX_READERS);
			if (readers < MAX_READERS) {
				// Acquire sem for all readers currently not using a semaphore.
				// But if we are not the only write lock in the queue, just get
				// the one for us
				if (readers <= 0)
					readers = 1;
				else
					readers = MAX_READERS - readers;

				fStatus = acquire_sem_etc(lock.fSemaphore,readers,0,0);
			} else
				fStatus = B_OK;
		}

		~WriteLocked()
		{
			int32 readers = atomic_add(&fLock.fCount,MAX_READERS);
			if (readers < 0) {
				// release sem for all readers only when we were the only writer
				if (readers <= -MAX_READERS)
					readers = -1;
				release_sem_etc(fLock.fSemaphore,-readers,0);
			}
		}

		status_t IsLocked()
		{
			return fStatus;
		}

	private:
		ReadWriteLock	&fLock;
		status_t		fStatus;
};

#endif	/* LOCK_H */
