/*
	$Id$
	
	This file implements a test class for testing BMessageQueue functionality.
	It tests use cases Destruction, Add Message 3, Remove Message 2,
	Next Message 2, Lock 1, Lock 2, Unlock.
	
	The test works like the following:
		- It does the test two times, one unlocking using Unlock() and the other
		  unlocking using delete.
		- It populates the queue with numAddMessages messages to start.
		- The queue is locked
	    - It starts four threads.
	    - In one thread, a NextMessage() blocks
	    - In the second thread, a RemoveMessage() blocks
	    - In the third thread, an AddMessage() blocks
	    - In the fourth thread, a Lock() blocks
	    - After a short snooze, the queue is released using Unlock() or delete.
	    - Each of the four threads wake up and each checks as best it can that it
	      was successful and did not violate mutual exclusion.
		
	*/


#include "Test5.h"
#include <stdio.h>
#include <OS.h>
#include <MessageQueue.h>
#include "MessageQueue.h"


const int numAddMessages = 50;

		
template<class MessageQueue>
	Test5<MessageQueue>::Test5(MessageQueue *MessageQueueArg,
                         const char *nameArg) : TestMessageQueue<MessageQueue>(MessageQueueArg, nameArg),
                         						testResult(true)
{
	}


template<class MessageQueue>
	Test5<MessageQueue>::~Test5()
{
	}
	

template<class MessageQueue> bool Test5<MessageQueue>::PerformTest(void)
{
	status_t exit_value;
	
	testResult = true;
	unlockTest = true;
	isLocked = false;
	
	do {
		testMessageClass::messageDestructorCount = 0;
		
		int i;
		BMessage *theMessage;
		for (i=0; i < numAddMessages; i++) {
			theMessage = new testMessageClass(i);
			theMessageQueue->AddMessage(theMessage);
		}
		removeMessage = theMessage;
		
		theMessageQueue->Lock();
		isLocked = true;
		
		thread_id firstThread = CreateAndRunThread(
					reinterpret_cast<thread_func>(Test5<MessageQueue>::ThreadEntry1),
					"First Thread",
					static_cast<void *>(this));
		thread_id secondThread = CreateAndRunThread(
					reinterpret_cast<thread_func>(Test5<MessageQueue>::ThreadEntry2),
					"Second Thread",
					static_cast<void *>(this));
		thread_id thirdThread = CreateAndRunThread(
					reinterpret_cast<thread_func>(Test5<MessageQueue>::ThreadEntry3),
					"Third Thread",
					static_cast<void *>(this));
		thread_id fourthThread = CreateAndRunThread(
					reinterpret_cast<thread_func>(Test5<MessageQueue>::ThreadEntry4),
					"Fourth Thread",
					static_cast<void *>(this));
		
		snooze(500000);
		
		isLocked = false;
		if (unlockTest) {
			theMessageQueue->Unlock();
		} else {
			MessageQueue *tmpMessageQueue = theMessageQueue;
			theMessageQueue = NULL;
			delete tmpMessageQueue;
		}
					
		wait_for_thread(firstThread, &exit_value);
		wait_for_thread(secondThread, &exit_value);
		wait_for_thread(thirdThread, &exit_value);
		wait_for_thread(fourthThread, &exit_value);
		
		delete theMessageQueue;
		
		theMessageQueue = new MessageQueue;
		
		unlockTest = !unlockTest;
	} while (unlockTest == false);

	return(testResult);
	}
		
	
template<class MessageQueue> int32 Test5<MessageQueue>::ThreadEntry1(Test5<MessageQueue> *threadArg)
{
	threadArg->TestRemove();
	return(0);
	}
	
	
template<class MessageQueue> int32 Test5<MessageQueue>::ThreadEntry2(Test5<MessageQueue> *threadArg)
{
	threadArg->TestNext();
	return(0);
	}
	
	
template<class MessageQueue> int32 Test5<MessageQueue>::ThreadEntry3(Test5<MessageQueue> *threadArg)
{
	threadArg->TestAdd();
	return(0);
	}
	
	
template<class MessageQueue> int32 Test5<MessageQueue>::ThreadEntry4(Test5<MessageQueue> *threadArg)
{
	threadArg->TestLock();
	return(0);
	}
	

template<class MessageQueue> void Test5<MessageQueue>::HandleTestFailure(const char *errorMsg)
{
	printf("%s\n", errorMsg);
	testResult = false;
}


template<class MessageQueue> void Test5<MessageQueue>::TestNext(void)
{
	if (!isLocked) {
		HandleTestFailure("Expected the queue to be locked!");
	} else {
		if (!unlockTest) {
			// Be's implementation can cause a segv when NextMessage() is in
			// progress when a delete occurs.  The OpenBeOS implementation
			// does not segv, but it won't be tested here because Be's fails.
			return;
		}
		BMessage *theMessage = theMessageQueue->NextMessage();
		if (isLocked) {
			HandleTestFailure("Expected the queue to be unlocked!");
		} else if ((unlockTest) && (theMessage == NULL)) {
			HandleTestFailure("Expected NextMessage() to return non-NULL!");
		}
		if ((unlockTest) && (theMessage != NULL) && (theMessage->what != 0)) {
			HandleTestFailure("Expected NextMessage() to return 0 what code!");
		}
		// The following test passes for the OpenBeOS implementation but
		// fails for the Be implementation.  If the BMessageQueue is deleted
		// while another thread is blocking waiting for NextMessage(), the
		// OpenBeOS implementation detects that the message queue is deleted
		// and returns NULL.  The Be implementation actually returns a message.
		// It must be doing so from freed memory since the queue has been
		// deleted.  The OpenBeOS implementation will not emulate the Be
		// implementation since I consider it a bug.
		//
		// if ((!unlockTest) && (theMessage != NULL)) {
		// 	HandleTestFailure("Expected NextMessage() to return NULL!");
		// }
	}
}


template<class MessageQueue> void Test5<MessageQueue>::TestRemove(void)
{
	if (!isLocked) {
		HandleTestFailure("Expected the queue to be locked!");
	} else {
		if (!unlockTest) {
			// Be's implementation causes a segv when RemoveMessage() is in
			// progress when a delete occurs.  The OpenBeOS implementation
			// does not segv, but it won't be tested here because Be's fails.
			return;
		}
		theMessageQueue->RemoveMessage(removeMessage);
		if (isLocked) {
			HandleTestFailure("Expected the queue to be unlocked!");
		} else if ((unlockTest) &&
					(theMessageQueue->FindMessage(removeMessage->what, 0) != NULL)) {
			HandleTestFailure("Expected FindMessage() to return NULL!");
		}
	}
}


template<class MessageQueue> void Test5<MessageQueue>::TestAdd(void)
{
	if (!isLocked) {
		HandleTestFailure("Expected the queue to be locked!");
	} else {
		if (!unlockTest) {
			// Be's implementation can cause a segv when AddMessage() is in
			// progress when a delete occurs.  The OpenBeOS implementation
			// does not segv, but it won't be tested here because Be's fails.
			return;
		}
		theMessageQueue->AddMessage(new testMessageClass(numAddMessages));
		if (isLocked) {
			HandleTestFailure("Expected the queue to be unlocked!");
		} else if ((unlockTest) &&
					(theMessageQueue->FindMessage(numAddMessages, 0) == NULL)) {
			HandleTestFailure("Expected FindMessage() to return non-NULL!");
		}
	}
}


template<class MessageQueue> void Test5<MessageQueue>::TestLock(void)
{
	if (!isLocked) {
		HandleTestFailure("Expected the queue to be locked!");
	} else {
		bool result = theMessageQueue->Lock();
		if (isLocked) {
			HandleTestFailure("Expected the queue to be unlocked!");
		} else if ((unlockTest) && (!result)) {
			HandleTestFailure("Expected to be able to acquire the lock!");
		} else if ((!unlockTest) && (result)) {
			HandleTestFailure("Expected not to be able to acquire the lock!");
		}
		if (unlockTest) {
			theMessageQueue->Unlock();
		}
	}
}


template class Test5<BMessageQueue>;
template class Test5<OpenBeOS::BMessageQueue>;