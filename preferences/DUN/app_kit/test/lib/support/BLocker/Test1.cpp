/*
	$Id$
	
	This file implements a test class for testing BLocker functionality.
	It tests use cases "Locking 1", "Locking 2", "Unlocking", "Is Locked",
	"Locking Thread" and "Count Locks".
	
	To perform the testing, the following method is used:
		- three concurrent threads are created all doing the same thing
		- without the thread acquired, the state of the lock is checked using
		  CheckLock()
		- the thread attempts to acquire the lock once
		- the thread checks that mutual exclusion is guaranteed by checking the
		  "lockTestValue"
		- if mutual exclusion is OK, then lockTestValue is set
		- the thread checks the state of the lock again
		- the lock is aquired a second time
		- the thread checks the state of the lock again
		- the lock is released once (it still needs to be released once more)
		- the thread checks the state of the lock again
		- the mutual exclusion value "lockTestValue" is reset
		- the thread releases the lock again, allowing other threads to acquire
		  the lock
		- the thread checks the state of the lock again
		- this is all performed MAXLOOP times per thread
		- the lock is acquired using Lock() or LockWithTimeout() so that all
		  combinations should be tested
	
	*/


#include "Test1.h"
#include <stdio.h>
#include <be/kernel/OS.h>
#include <be/support/Locker.h>
#include "Locker.h"


const int32 MAXLOOP = 100000;

		
template<class Locker>
	Test1<Locker>::Test1(Locker *lockerArg,
                         const char *nameArg) : TestLocker<Locker>(lockerArg, nameArg),
                         						testResult(true),
                         						lockTestValue(false)
{
	}


template<class Locker>
	Test1<Locker>::~Test1()
{
	}
	

template<class Locker> bool Test1<Locker>::PerformTest(void)
{
	status_t exit_value;
	
	testResult = true;

	lockTestValue = false;
	thread_id firstThread = CreateAndRunThread(
								reinterpret_cast<thread_func>(Test1<Locker>::ThreadEntry),
								"First Thread",
								static_cast<void *>(this));
	thread_id secondThread = CreateAndRunThread(
								reinterpret_cast<thread_func>(Test1<Locker>::ThreadEntry),
								"Second Thread",
								static_cast<void *>(this));
								
	TestThread();
	
	wait_for_thread(firstThread, &exit_value);
	wait_for_thread(secondThread, &exit_value);
	return(testResult);
	}
		
	
template<class Locker> int32 Test1<Locker>::ThreadEntry(Test1<Locker> *threadArg)
{
	threadArg->TestThread();
	return(0);
	}
	
	
template<class Locker> bool Test1<Locker>::AcquireLock(int lockAttempt,
                                                       bool firstAcquisition)
{
	bool timeoutLock;
	bool result;
	
	if (firstAcquisition) {
		timeoutLock = ((lockAttempt % 2) == 1);
	} else {
		timeoutLock = (((lockAttempt / 2) % 2) == 1);
	}
	if (timeoutLock) {
		result = (theLocker->LockWithTimeout(1000000) == B_OK);
	} else {
		result = theLocker->Lock();
	}
	return(result);
}


template<class Locker> void Test1<Locker>::HandleTestFailure(const char *errorMsg,
														     int iteration)
{
	printf("%s\nFailure at iteration %d\n", errorMsg, iteration);
	testResult = false;
	theLocker->Unlock();
	theLocker->Unlock();
}


template<class Locker> void Test1<Locker>::TestThread(void)
{
	int i;
	
	for (i = 0; i < MAXLOOP; i++) {
		if (!CheckLock(0)) {
			HandleTestFailure("Lock is already acquired in TestThread!", i);
			return;
		}
		if (!AcquireLock(i, 1)) {
			HandleTestFailure("Lock returned false in TestThread!", i);
			return;
		}
		
		if (lockTestValue) {
			HandleTestFailure("Mutual exclusion failed!", i);
			return;
		}
		lockTestValue = true;
		
		if (!CheckLock(1)) {
			HandleTestFailure("First CheckLock failed in TestThread!", i);
			return;
		}
		
		if (!AcquireLock(i, 2)) {
			HandleTestFailure("Second lock returned false in TestThread!", i);
			return;
		}
		
		if (!CheckLock(2)) {
			HandleTestFailure("Second CheckLock failed in TestThread!", i);
			return;
		}
		
		theLocker->Unlock();
		if (!CheckLock(1)) {
			HandleTestFailure("Third CheckLock failed in TestThread!", i);
			return;
		}
		
		lockTestValue = false;
		theLocker->Unlock();
		if (!CheckLock(0)) {
			HandleTestFailure("Lock is already acquired in TestThread!", i);
			return;
		}
	}
}

template class Test1<BLocker>;
template class Test1<OpenBeOS::BLocker>;