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


#include "BenaphoreLockCountTest1.h"
#include "TestSuite.h"
#include "ThreadedTestCaller.h"
#include <be/support/Locker.h>
#include "Locker.h"


// This constant is used to determine the number of microseconds to
// sleep during major steps of the test.

const bigtime_t SNOOZE_TIME = 100000;


/*
 *  Method:  BenaphoreLockCountTest1<Locker>::BenaphoreLockCountTest1()
 *   Descr:  This is the constructor for this test class.
 */
		
template<class Locker>
	BenaphoreLockCountTest1<Locker>::BenaphoreLockCountTest1(std::string name) :
		LockerTestCase<Locker>(name, true)
{
	}


/*
 *  Method:  BenaphoreLockCountTest1<Locker>::~BenaphoreLockTestCountTest1()
 *   Descr:  This is the destructor for this test class.
 */

template<class Locker>
	BenaphoreLockCountTest1<Locker>::~BenaphoreLockCountTest1()
{
	}


/*
 *  Method:  BenaphoreLockCountTest1<Locker>::CheckLockRequests()
 *   Descr:  This member function checks the actual number of lock requests
 *           that the BLocker thinks are outstanding versus the number
 *           passed in.  If they match, true is returned.
 */
	
template<class Locker> bool BenaphoreLockCountTest1<Locker>::CheckLockRequests(int expected)
{
	int actual = theLocker->CountLockRequests();
	return(actual == expected);
}


/*
 *  Method:  BenaphoreLockCountTest1<Locker>::TestThread1()
 *   Descr:  This member function performs the main portion of the test.
 *           It first acquires thread2Lock and thread3Lock.  This ensures
 *           that thread2 and thread3 will block until this thread wants
 *           them to start running.  It then checks the lock count, acquires
 *           the lock and checks the lock count again.  It unlocks each
 *           of the other two threads in turn and rechecks the lock count.
 *           Finally, it releases the lock and sleeps for a short while
 *           for the other two threads to finish.  At the end, it checks
 *           the lock count on final time.
 */

template<class Locker> void BenaphoreLockCountTest1<Locker>::TestThread1(void)
{
	SafetyLock<Locker> theSafetyLock1(theLocker);
	SafetyLock<Locker> theSafetyLock2(&thread2Lock);
	SafetyLock<Locker> theSafetyLock3(&thread3Lock);
	
	assert(thread2Lock.Lock());
	assert(thread3Lock.Lock());

	assert(CheckLockRequests(0));
	assert(theLocker->Lock());
	
	assert(CheckLockRequests(1));
	
	thread2Lock.Unlock();
	snooze(SNOOZE_TIME);
	assert(CheckLockRequests(3));
	
	thread3Lock.Unlock();
	snooze(SNOOZE_TIME);
	assert(CheckLockRequests(5));
	
	theLocker->Unlock();
	snooze(SNOOZE_TIME);
	assert(CheckLockRequests(2));
	}


/*
 *  Method:  BenaphoreLockCountTest1<Locker>::TestThread2()
 *   Descr:  This member function defines the actions of the second thread of
 *           the test.  First it sleeps for a short while and then blocks on
 *           the thread2Lock.  When the first thread releases it, this thread
 *           begins its testing.  It times out attempting to acquire the main
 *           lock and then blocks to acquire the lock.  Once that lock is
 *           acquired, the lock count is checked before finishing this thread.
 */

template<class Locker> void BenaphoreLockCountTest1<Locker>::TestThread2(void)
{
	SafetyLock<Locker> theSafetyLock1(theLocker);
	
	snooze(SNOOZE_TIME / 10);
	assert(thread2Lock.Lock());
	
	assert(theLocker->LockWithTimeout(SNOOZE_TIME / 10) == B_TIMED_OUT);
	assert(theLocker->Lock());
	int actual = theLocker->CountLockRequests();
	assert((actual == 3) || (actual == 4));
	theLocker->Unlock();
}


/*
 *  Method:  BenaphoreLockCountTest1<Locker>::TestThread3()
 *   Descr:  This member function defines the actions of the second thread of
 *           the test.  First it sleeps for a short while and then blocks on
 *           the thread3Lock.  When the first thread releases it, this thread
 *           begins its testing.  It times out attempting to acquire the main
 *           lock and then blocks to acquire the lock.  Once that lock is
 *           acquired, the lock count is checked before finishing this thread.
 */

template<class Locker> void BenaphoreLockCountTest1<Locker>::TestThread3(void)
{
	SafetyLock<Locker> theSafetyLock1(theLocker);
	
	snooze(SNOOZE_TIME / 10);
	assert(thread3Lock.Lock());
	
	assert(theLocker->LockWithTimeout(SNOOZE_TIME / 10) == B_TIMED_OUT);
	assert(theLocker->Lock());
	int actual = theLocker->CountLockRequests();
	assert((actual == 3) || (actual == 4));
	theLocker->Unlock();
}


/*
 *  Method:  BenaphoreLockCountTest1<Locker>::suite()
 *   Descr:  This static member function returns a test caller for performing 
 *           the "BenaphoreLockCountTest1" test.  The test caller
 *           is created as a ThreadedTestCaller (typedef'd as
 *           BenaphoreLockCountTest1Caller) with three independent threads.
 */

template<class Locker> Test *BenaphoreLockCountTest1<Locker>::suite(void)
{	
	BenaphoreLockCountTest1<Locker> *theTest = new BenaphoreLockCountTest1<Locker>("");
	BenaphoreLockCountTest1Caller *threadedTest = new BenaphoreLockCountTest1Caller("", theTest);
	threadedTest->addThread(":Thread1", &BenaphoreLockCountTest1<Locker>::TestThread1);
	threadedTest->addThread(":Thread2", &BenaphoreLockCountTest1<Locker>::TestThread2);
	threadedTest->addThread(":Thread3", &BenaphoreLockCountTest1<Locker>::TestThread3);
	return(threadedTest);
	}


template class BenaphoreLockCountTest1<BLocker>;
template class BenaphoreLockCountTest1<OpenBeOS::BLocker>;