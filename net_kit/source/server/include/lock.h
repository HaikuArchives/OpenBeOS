#ifndef LOCK_H
#define LOCK_H
/* lock.h - benaphore locking, "faster semaphores", read/write locks
**		sorry for all those macros; I wish C would support C++-style
**		inline functions.
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
**
** This file may be used under the terms of the OpenBeOS License.
*/


#include <SupportDefs.h>

// for read/write locks
#define MAX_READERS 100000


struct benaphore {
	sem_id	sem;
	int32	count;
};

typedef struct benaphore benaphore;

#define INIT_BENAPHORE(lock,name) \
	{ \
		(lock).sem = create_sem(0, name); \
		(lock).count = 1; \
	}

#define CHECK_BENAPHORE(lock) \
	((lock).sem)

#define UNINIT_BENAPHORE(lock) \
	delete_sem((lock).sem);

#define ACQUIRE_BENAPHORE(lock) \
	(atomic_add(&((lock).count), -1) <= 0 ? acquire_sem((lock).sem) : B_OK)

#define RELEASE_BENAPHORE(lock) \
	{ \
		if (atomic_add(&((lock).count), 1) < 0) \
			release_sem((lock).sem); \
	}

/* read/write lock */

#define CREATE_RW_LOCK(name) \
	create_sem(MAX_READERS, name)

#define DELETE_RW_LOCK(sem) \
	delete_sem(sem)

#define ACQUIRE_READ_LOCK(sem) \
	while (acquire_sem(sem) == B_INTERRUPTED);

#define RELEASE_READ_LOCK(sem) \
	release_sem(sem);

#define ACQUIRE_WRITE_LOCK(sem) \
	while (acquire_sem_etc(sem, MAX_READERS, B_ABSOLUTE_TIMEOUT, \
				B_INFINITE_TIMEOUT) == B_INTERRUPTED);

#define RELEASE_WRITE_LOCK(sem) \
	release_sem_etc(sem, MAX_READERS, 0);

#endif	/* LOCK_H */
