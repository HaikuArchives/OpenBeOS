/*
	$Id$
	
	This file implements a test class for testing BLocker functionality.
	It tests use cases "Locking 1", "Locking 2", "Unlocking", "Is Locked",
	"Locking Thread" and "Count Locks".  It is essentially the same as Test1.cpp
	except it makes the first LockWithTimeout inside the threads timeout.  The
	reason for this is because the implementation of BLocker by Be and with OpenBeOS
	is such that after one timeout occurs on a benaphore style BLocker, the lock
	effectively becomes a semaphore style BLocker.  This test tests that condition.
	
	*/


#include "Test5.h"
#include <stdio.h>
#include <be/kernel/OS.h>
#include <be/support/Locker.h>
#include "Locker.h"


const int32 MAXLOOP = 100000;

		
template<class Locker>
	Test5<Locker>::Test5(Locker *lockerArg,
                         const char *nameArg) : TestLocker<Locker>(lockerArg, nameArg),
                         						testResult(true),
                         						lockTestValue(false)
{
	}


template<class Locker>
	Test5<Locker>::~Test5()
{
	}
	

template<class Locker> bool Test5<Locker>::PerformTest(void)
{
	status_t exit_value;
	
	testResult = true;

	lockTestValue = false;
	
	if (!theLocker->Lock()) {
		printf("Unable to acquire the lock to start with!\n");
		return(false);
	}
	
	thread_id firstThread = CreateAndRunThread(
								reinterpret_cast<thread_func>(Test5<Locker>::ThreadEntry),
								"First Thread",
								static_cast<void *>(this));
	thread_id secondThread = CreateAndRunThread(
								reinterpret_cast<thread_func>(Test5<Locker>::ThreadEntry),
								"Second Thread",
								static_cast<void *>(this));
	snooze(1000000);					
	theLocker->Unlock();
	LockingLoop();
	
	wait_for_thread(firstThread, &exit_value);
	wait_for_thread(secondThread, &exit_value);
	return(testResult);
	}
		
	
template<class Locker> int32 Test5<Locker>::ThreadEntry(Test5<Locker> *threadArg)
{
	threadArg->TestThread();
	return(0);
	}
	
	
template<class Locker> bool Test5<Locker>::AcquireLock(int lockAttempt,
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


template<class Locker> void Test5<Locker>::HandleTestFailure(const char *errorMsg,
														     int iteration)
{
	printf("%s\nFailure at iteration %d\n", errorMsg, iteration);
	testResult = false;
	theLocker->Unlock();
	theLocker->Unlock();
}


template<class Locker> void Test5<Locker>::TestThread(void)
{
	if (theLocker->LockWithTimeout(10000) != B_TIMED_OUT) {
		printf("Expected lock acquisition to time out!\n");
		testResult = false;
		return;
	}
	LockingLoop();
}
	
	
template<class Locker> void Test5<Locker>::LockingLoop(void)
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

template class Test5<BLocker>;
template class Test5<OpenBeOS::BLocker>;