/*
	$Id$
	
	This file implements a base class for testing BLocker functionality.
	
	*/


#include "TestLocker.h"
#include <stdio.h>
#include <be/kernel/OS.h>
#include <be/support/Locker.h>
#include "Locker.h"

		
template<class Locker>
	TestLocker<Locker>::TestLocker(Locker *lockerArg,
                                   const char *nameArg) : testName(nameArg), 
                                                          theLocker(lockerArg)
{
	}


template<class Locker>
	TestLocker<Locker>::~TestLocker()
{
	delete theLocker;
	theLocker = NULL;
	}
		
	
template<class Locker> bool TestLocker<Locker>::CheckLock(int32 expectedCount)
{
	bool isLocked = theLocker->IsLocked();
	thread_id actualThread = theLocker->LockingThread();
	thread_id expectedThread = find_thread(NULL);
	int32 actualCount = theLocker->CountLocks();
	
	if (expectedCount > 0) {
		if (!isLocked) {
			printf("CheckLock() found lock unlocked!\n");
			return(false);
		}
		if (expectedThread != actualThread) {
			printf("CheckLock() found lock held by wrong thread!\n");
			printf("actualThread = %d  expectedThread %d\n",
			             static_cast<int>(actualThread),
			             static_cast<int>(expectedThread));
			return(false);
		}
		if (expectedCount != actualCount) {
			printf("CheckLock() found wrong lock count!\n");
			printf("actualCount = %d  expectedCount = %d\n", 
			             static_cast<int>(actualCount),
			             static_cast<int>(expectedCount));
			return(false);
		}
	} else {
		if ((isLocked) && (actualThread == expectedThread)) {
	    	printf("CheckLock() found lock locked!\n");
		}
	}
	return(true);
}

	
template<class Locker>
	thread_id TestLocker<Locker>::CreateAndRunThread(thread_func func,
													 const char *threadName, 
													 void *data)
{
	thread_id theThread = spawn_thread(func, threadName, B_NORMAL_PRIORITY, data);
	resume_thread(theThread);
	return(theThread);
	}
	
	
template class TestLocker<BLocker>;
template class TestLocker<OpenBeOS::BLocker>;