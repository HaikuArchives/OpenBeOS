/*
	$Id$
	
	This file implements a test class for testing BLocker functionality.
	It tests use cases "Destruction" and "Locking 3".
	
	The test works like the following:
		- the main thread acquires the lock
		- it creates a new thread and sleeps
		- the new thread then attempts to acquire the lock
		- the first thread releases the lock
		- at this time, the new thread acquires the lock and goes to sleep
		- the first thread attempts to acquire the lock
		- the second thread deletes the lock
		- the first thread is woken up indicating that the lock wasn't acquired.
		
	*/


#include "Test3.h"
#include <stdio.h>
#include <be/kernel/OS.h>
#include <be/support/Locker.h>
#include "Locker.h"


const bigtime_t SNOOZE_TIME = 2000000;

		
template<class Locker>
	Test3<Locker>::Test3(Locker *lockerArg,
                         const char *nameArg) : TestLocker<Locker>(lockerArg, nameArg),
                         						testResult(true)
{
	}


template<class Locker>
	Test3<Locker>::~Test3()
{
	}
	

template<class Locker> bool Test3<Locker>::PerformTest(void)
{
	status_t exit_value;
	
	testResult = true;
	
	if (!theLocker->Lock()) {
		HandleTestFailure("First lock acquisition failed!");
	} else {	
		thread_id firstThread = CreateAndRunThread(
								reinterpret_cast<thread_func>(Test3<Locker>::ThreadEntry),
								"First Thread",
								static_cast<void *>(this));
								
		snooze(SNOOZE_TIME);
		theLocker->Unlock();
		snooze(SNOOZE_TIME);
		if (theLocker->Lock()) {
			HandleTestFailure("Expected the lock acquisition to fail!");
		}
		wait_for_thread(firstThread, &exit_value);
	}
	return(testResult);
	}
		
	
template<class Locker> int32 Test3<Locker>::ThreadEntry(Test3<Locker> *threadArg)
{
	threadArg->TestThread();
	return(0);
	}


template<class Locker> void Test3<Locker>::HandleTestFailure(const char *errorMsg)
{
	printf("%s\n", errorMsg);
	testResult = false;
}


template<class Locker> void Test3<Locker>::TestThread(void)
{
	Locker *tmpLock;
	
	if (!theLocker->Lock()) {
		HandleTestFailure("Lock acquisition in thread failed!");
		return;
	}	
	snooze(SNOOZE_TIME);	
	snooze(SNOOZE_TIME);
	tmpLock = theLocker;
	theLocker = NULL;
	delete tmpLock;
}

template class Test3<BLocker>;
template class Test3<OpenBeOS::BLocker>;