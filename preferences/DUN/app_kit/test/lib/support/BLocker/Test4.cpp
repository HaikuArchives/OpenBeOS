/*
	$Id$
	
	This file implements a test class for testing BLocker functionality.
	It tests use cases "Destruction" and "Locking 4".
	
	The test works like the following:
		- the main thread acquires the lock
		- it creates a new thread and sleeps
		- the new thread attempts to acquire the lock but times out
		- the new thread then attempts to acquire the lock again
		- before the new thread times out a second time, the first thread releases
		  the lock
		- at this time, the new thread acquires the lock and goes to sleep
		- the first thread attempts to acquire the lock
		- the second thread deletes the lock
		- the first thread is woken up indicating that the lock wasn't acquired.
	
	*/


#include "Test4.h"
#include <stdio.h>
#include <be/kernel/OS.h>
#include <be/support/Locker.h>
#include "Locker.h"


const bigtime_t SNOOZE_TIME = 2000000;

		
template<class Locker>
	Test4<Locker>::Test4(Locker *lockerArg,
                         const char *nameArg) : TestLocker<Locker>(lockerArg, nameArg),
                         						testResult(true)
{
	}


template<class Locker>
	Test4<Locker>::~Test4()
{
	}
	

template<class Locker> bool Test4<Locker>::PerformTest(void)
{
	status_t exit_value;
	
	testResult = true;
	
	if (theLocker->LockWithTimeout(SNOOZE_TIME) != B_OK) {
		HandleTestFailure("First lock acquisition failed!");
	} else {	
		thread_id firstThread = CreateAndRunThread(
								reinterpret_cast<thread_func>(Test4<Locker>::ThreadEntry),
								"First Thread",
								static_cast<void *>(this));
								
		snooze(SNOOZE_TIME);
		theLocker->Unlock();
		snooze(SNOOZE_TIME);
		if (theLocker->LockWithTimeout(SNOOZE_TIME * 10) != B_BAD_SEM_ID) {
			HandleTestFailure("Expected the lock acquisition to fail!");
		}
		wait_for_thread(firstThread, &exit_value);
	}
	return(testResult);
	}
		
	
template<class Locker> int32 Test4<Locker>::ThreadEntry(Test4<Locker> *threadArg)
{
	threadArg->TestThread();
	return(0);
	}


template<class Locker> void Test4<Locker>::HandleTestFailure(const char *errorMsg)
{
	printf("%s\n", errorMsg);
	testResult = false;
}


template<class Locker> void Test4<Locker>::TestThread(void)
{
	Locker *tmpLock;
	
	if (theLocker->LockWithTimeout(SNOOZE_TIME / 10) != B_TIMED_OUT) {
		HandleTestFailure("Expected LockWithTimeout() to timeout!");
		return;
	}
	
	if (theLocker->LockWithTimeout(SNOOZE_TIME * 10) != B_OK) {
		HandleTestFailure("Lock acquisition in thread failed!");
		return;
	}	
	snooze(SNOOZE_TIME);
	snooze(SNOOZE_TIME);
	tmpLock = theLocker;
	theLocker = NULL;
	delete tmpLock;
}

template class Test4<BLocker>;
template class Test4<OpenBeOS::BLocker>;