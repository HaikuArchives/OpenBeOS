/*
	$Id$
	
	This file implements a test class for testing BLocker functionality.
	It tests use cases "Count Lock Requests" for a benaphore style BLocker.
	
	The test works by:
		- checking the lock requests
		- acquiring the lock
		- checking the lock requests
		- staring a thread which times out acquiring the lock and then blocks
		  again waiting for the lock
		- checking the lock requests
		- start a second thread which times out acquiring the lock and then blocks
		  again waiting for the lock
		- checking the lock requests
		- release the lock
		- each blocked thread acquires the lock, checks the lock requests and releases
		  the lock before terminating
		- the main thread checks the lock requests one last time
		
	*/


#include "Test6.h"
#include <stdio.h>
#include <be/kernel/OS.h>
#include <be/support/Locker.h>
#include "Locker.h"


const int32 MAXLOOP = 100000;

		
template<class Locker>
	Test6<Locker>::Test6(const char *nameArg) : TestLocker<Locker>(new Locker(true),
																   nameArg),
                         						testResult(true)
{
	}


template<class Locker>
	Test6<Locker>::~Test6()
{
	}
	
	
template<class Locker> bool Test6<Locker>::CheckLockRequests(int expected)
{
	int actual = theLocker->CountLockRequests();
	
	if (actual != expected) {
		printf("CheckLockRequests() failed, actual=%d, expected=%d\n", actual, expected);
		testResult = false;
	}
	return(actual == expected);
}


template<class Locker> bool Test6<Locker>::PerformTest(void)
{
	status_t exit_value;
	
	testResult = true;
	
	CheckLockRequests(0);
	
	if (!theLocker->Lock()) {
		printf("Unable to acquire the lock to start with!\n");
		return(false);
	}
	
	CheckLockRequests(1);
	
	thread_id firstThread = CreateAndRunThread(
							reinterpret_cast<thread_func>(Test6<Locker>::ThreadEntry),
							"First Thread",
							static_cast<void *>(this));
	snooze(1000000);
	
	CheckLockRequests(3);
	
	thread_id secondThread = CreateAndRunThread(
							reinterpret_cast<thread_func>(Test6<Locker>::ThreadEntry),
							"Second Thread",
							static_cast<void *>(this));		
	snooze(1000000);
	
	CheckLockRequests(5);
	theLocker->Unlock();
	wait_for_thread(secondThread, &exit_value);
	wait_for_thread(firstThread, &exit_value);
	
	CheckLockRequests(2);
	return(testResult);
	}
		
	
template<class Locker> int32 Test6<Locker>::ThreadEntry(Test6<Locker> *threadArg)
{
	threadArg->TestThread();
	return(0);
	}


template<class Locker> void Test6<Locker>::TestThread(void)
{
	if (theLocker->LockWithTimeout(10000) != B_TIMED_OUT) {
		printf("Expected lock acquisition to time out!\n");
		testResult = false;
		return;
	}
	if (!theLocker->Lock()) {
		printf("Error acquiring the lock within the thread!\n");
		testResult = false;
	}
	int actual = theLocker->CountLockRequests();
	
	switch (actual) {
		case 3:
		case 4:
			break;
		default:
			printf("Expected the lock requests to be 1 or 2, actual=%d\n", actual);
			testResult = false;
			break;
	}
	theLocker->Unlock();
}

template class Test6<BLocker>;
template class Test6<OpenBeOS::BLocker>;