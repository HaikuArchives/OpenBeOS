/*
	Test3.cpp
	
	This file implements a test class for testing BMessageQueue functionality.
	It tests use cases Destruction, Add Message 1, Add Message 3, Remove Message 1,
	Remove Message 2, Count Messages, Find Message 1, Next Message 1 and Next Message
	2.
	
	The test works like the following:
		- It does the test two times, one using an internal BList to keep track of
		  what should be in the queue and once without.  It does it without because
		  using the internal BList isn't a good test of mutual exclusion because an
		  explicit lock is used.  However, it is worth testing with the BList to show
		  that the result is what what expected.
	    - It starts three threads.
	    - In one thread, numAddMessages are added to the queue
	    - In the second thread, numNextMessages are removed from the queue using
	      NextMessage().
	    - In the third thread, numAddMessages are added to the queue however every
	      numRemovedMessagesPerAdd messages is removed using RemoveMessage().
	    - Once all three threads complete, the number of elements on the queue is
	      compared to what it expects.
	    - If the BList is being used internally, the queue is checked against the list.
	    - It checks that no BMessages have been deleted.
	    - It deletes the message queue.
	    - It checks that the expected number of BMessages have been deleted.
	    - It creates a new message queue and restarts the test again if necessary
	      (ie perform it without the internal list).
		
	*/


#include "Test3.h"
#include <stdio.h>
#include <OS.h>
#include <MessageQueue.h>
#include "MessageQueue.h"


const int numAddMessages = 5000;
const int numNextMessages = 100;
const int numRemoveMessagesPerAdd = 5;

		
template<class MessageQueue>
	Test3<MessageQueue>::Test3(MessageQueue *MessageQueueArg,
                         const char *nameArg) : TestMessageQueue<MessageQueue>(MessageQueueArg, nameArg),
                         						testResult(true)
{
	}


template<class MessageQueue>
	Test3<MessageQueue>::~Test3()
{
	}
	

template<class MessageQueue> bool Test3<MessageQueue>::PerformTest(void)
{
	status_t exit_value;
	
	testResult = true;
	useList = true;
	
	do {
		testMessageClass::messageDestructorCount = 0;
		
		thread_id firstThread = CreateAndRunThread(
					reinterpret_cast<thread_func>(Test3<MessageQueue>::ThreadEntry1),
					"First Thread",
					static_cast<void *>(this));
		thread_id secondThread = CreateAndRunThread(
					reinterpret_cast<thread_func>(Test3<MessageQueue>::ThreadEntry2),
					"Second Thread",
					static_cast<void *>(this));
		
		int i;
		for (i=0; i < numAddMessages; i++) {
			BMessage *theMessage = new testMessageClass(i);
			if (useList) {
				AddMessage(theMessage);
			} else {
				theMessageQueue->AddMessage(theMessage);
			}
		}
					
		wait_for_thread(firstThread, &exit_value);
		wait_for_thread(secondThread, &exit_value);
		
		if ((useList) && (!CheckQueueAgainstList())) {
			HandleTestFailure("List and queue do not match!");
		}
		
		int numMessages = (2*numAddMessages) -
							(numAddMessages / numRemoveMessagesPerAdd) -
							numNextMessages;
		if (numMessages != theMessageQueue->CountMessages()) {
			HandleTestFailure("Wrong number of messages found on queue!");
		}
		
		if (testMessageClass::messageDestructorCount != 0) {
			HandleTestFailure("Expected no messages deleted!");
		}
		
		delete theMessageQueue;
		
		if (testMessageClass::messageDestructorCount != numMessages) {
			printf("Expected %d messages deleted!\n", numMessages);
			testResult = false;
		}
		
		theMessageQueue = new MessageQueue;
		
		useList = !useList;
	} while (useList == false);

	return(testResult);
	}
		
	
template<class MessageQueue> int32 Test3<MessageQueue>::ThreadEntry1(Test3<MessageQueue> *threadArg)
{
	threadArg->TestRemove();
	return(0);
	}
	
	
template<class MessageQueue> int32 Test3<MessageQueue>::ThreadEntry2(Test3<MessageQueue> *threadArg)
{
	threadArg->TestNext();
	return(0);
	}

template<class MessageQueue> void Test3<MessageQueue>::HandleTestFailure(const char *errorMsg)
{
	printf("%s\n", errorMsg);
	testResult = false;
}


template<class MessageQueue> void Test3<MessageQueue>::TestNext(void)
{
	int i;
	for(i = 0; i < numNextMessages; i++) {
		snooze(5000);
		BMessage *theMessage;
		if (useList) {
			theMessage = NextMessage();
		} else {
			theMessage = theMessageQueue->NextMessage();
		}
		if (theMessage == NULL) {
			HandleTestFailure("Expected a non NULL message from NextMessage()!\n");
		}		
	}
}

template<class MessageQueue> void Test3<MessageQueue>::TestRemove(void)
{
	int i;
	BList messagesToRemove;
	
	for (i=0; i < numAddMessages; i++) {
		BMessage *theMessage = new testMessageClass(i);
		if (useList) {
			AddMessage(theMessage);
		} else {
			theMessageQueue->AddMessage(theMessage);
		}
		if ((i % numRemoveMessagesPerAdd) == numRemoveMessagesPerAdd - 1) {
			snooze(500);
			if (useList) {
				RemoveMessage(theMessage);
			} else {
				theMessageQueue->RemoveMessage(theMessage);
			}
		}
	}
}

template class Test3<BMessageQueue>;
template class Test3<OpenBeOS::BMessageQueue>;