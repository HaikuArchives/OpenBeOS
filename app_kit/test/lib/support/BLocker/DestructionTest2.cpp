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


#include "DestructionTest2.h"
#include "TestSuite.h"
#include "ThreadedTestCaller.h"
#include <be/support/Locker.h>
#include "Locker.h"

// This constant is used to determine the number of microseconds to
// sleep during major steps of the test.

const bigtime_t SNOOZE_TIME = 200000;


/*
 *  Method:  DestructionTest2<Locker>::DestructionTest2()
 *   Descr:  This is the only constructor for this test class.
 */
		
template<class Locker>
	DestructionTest2<Locker>::DestructionTest2(std::string name,
											   bool isBenaphore) :
		LockerTestCase<Locker>(name, isBenaphore)
{
	}


/*
 *  Method:  DestructionTest2<Locker>::~DestructionTest2()
 *   Descr:  This is the only destructor for this test class.
 */

template<class Locker>
	DestructionTest2<Locker>::~DestructionTest2()
{
	}
	

/*
 *  Method:  DestructionTest2<Locker>::TestThread1()
 *   Descr:  This method immediately acquires the lock, sleeps
 *           for SNOOZE_TIME and then releases the lock.  It sleeps
 *           again for SNOOZE_TIME and then tries to re-acquire the
 *           lock.  By this time, the other thread should have
 *           deleted the lock.  This acquisition should fail.
 */

template<class Locker> void DestructionTest2<Locker>::TestThread1(void)
{
	assert(theLocker->LockWithTimeout(SNOOZE_TIME) == B_OK);	
	snooze(SNOOZE_TIME);
	theLocker->Unlock();
	snooze(SNOOZE_TIME);
	assert(theLocker->LockWithTimeout(SNOOZE_TIME * 10) == B_BAD_SEM_ID);
	}


/*
 *  Method:  DestructionTest2<Locker>::TestThread2()
 *   Descr:  This method sleeps for SNOOZE_TIME/10 and then attempts to acquire
 *           the lock for SNOOZE_TIME/10 seconds.  This acquisition will timeout
 *           because the other thread is holding the lock.  Then it acquires the
 *           lock by using a larger timeout. It sleeps again for 2*SNOOZE_TIME and
 *           then deletes the lock.  This should wake up the other thread.
 */

template<class Locker> void DestructionTest2<Locker>::TestThread2(void)
{
	Locker *tmpLock;
	
	snooze(SNOOZE_TIME/10);	
	assert(theLocker->LockWithTimeout(SNOOZE_TIME / 10) == B_TIMED_OUT);
	assert(theLocker->LockWithTimeout(SNOOZE_TIME * 10) == B_OK);
	snooze(SNOOZE_TIME);	
	snooze(SNOOZE_TIME);
	tmpLock = theLocker;
	theLocker = NULL;
	delete tmpLock;
}


/*
 *  Method:  DestructionTest2<Locker>::suite()
 *   Descr:  This static member function returns a test suite for performing 
 *           all combinations of "DestructionTest2".  The test suite contains
 *           two instances of the test.  One is performed on a benaphore,
 *           the other on a semaphore based BLocker.  Each individual test
 *           is created as a ThreadedTestCase (typedef'd as
 *           DestructionTest2Caller) with two independent threads.
 */

template<class Locker> Test *DestructionTest2<Locker>::suite(void)
{	
	TestSuite *testSuite = new TestSuite("DestructionTest2");
	
	// Make a benaphore based test object, create a ThreadedTestCase for it and add
	// two threads to it.
	DestructionTest2<Locker> *theTest = new DestructionTest2<Locker>("Benaphore", true);
	DestructionTest2Caller *threadedTest1 = new DestructionTest2Caller("", theTest);
	threadedTest1->addThread(":Thread1", &DestructionTest2<Locker>::TestThread1);
	threadedTest1->addThread(":Thread2", &DestructionTest2<Locker>::TestThread2);
				
	// Make a semaphore based test object, create a ThreadedTestCase for it and add
	// three threads to it.					 		
	theTest = new DestructionTest2<Locker>("Semaphore", false);
	DestructionTest2Caller *threadedTest2 = new DestructionTest2Caller("", theTest);
	threadedTest2->addThread(":Thread1", &DestructionTest2<Locker>::TestThread1);
	threadedTest2->addThread(":Thread2", &DestructionTest2<Locker>::TestThread2);
									 		
	testSuite->addTest(threadedTest1);	
	testSuite->addTest(threadedTest2);
	return(testSuite);
	}

template class DestructionTest2<BLocker>;
template class DestructionTest2<OpenBeOS::BLocker>;