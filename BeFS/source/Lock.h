#ifndef LOCK_H
#define LOCK_H
/* Lock - read/write lock implementation
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** Based on a sample code from Nathan Schrenk.
**
** This file may be used under the terms of the OpenBeOS License.
*/


#include <KernelExport.h>

#include "lock.h"


#define MAX_READERS 100000

// it may be a good idea to have timeouts for the WriteLocked class,
// in case something went wrong - we'll see if this is necessary,
// but it would be a somewhat poor work-around a deadlock...
// But the only real problem with timeouts could be for things like
// "chkbfs" - because such a tool may need to lock for some more time

class ReadWriteLock {
	public:
		ReadWriteLock()
		{
			fSem = create_sem(MAX_READERS, "bfs r/w lock");
		}

		~ReadWriteLock()
		{
			delete_sem(fSem);
		}

		status_t InitCheck()
		{
			if (fSem < B_OK)
				return fSem;
			
			return B_OK;
		}

	private:
		friend class ReadLocked;
		friend class WriteLocked;

		sem_id	fSem;
};


class ReadLocked {
	public:
		ReadLocked(ReadWriteLock &lock)
			: fSem(lock.fSem)
		{
			while (acquire_sem(fSem) == B_INTERRUPTED);
		}
		
		~ReadLocked()
		{
			release_sem(fSem);
		}
	
	private:
		sem_id	fSem;
};


class WriteLocked {
	public:
		WriteLocked(ReadWriteLock &lock)
			: fSem(lock.fSem)
		{
			status_t status;
			while ((status = acquire_sem_etc(fSem, MAX_READERS, B_ABSOLUTE_TIMEOUT,
						B_INFINITE_TIMEOUT)) == B_INTERRUPTED);

			// if something failed, we don't have write access
			// overwriting fSem saves us from using another variable
			if (status < B_OK)
				fSem = status;
		}
		
		~WriteLocked()
		{
			if (fSem >= B_OK)
				release_sem_etc(fSem, MAX_READERS, 0);
		}

		status_t IsLocked()
		{
			return fSem < B_OK ? fSem : B_OK;
		}

	private:
		sem_id	fSem;
};

#endif	/* LOCK_H */
