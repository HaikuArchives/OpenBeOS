/*
	$Id$
	
	This file implements the first test for the OpenBeOS BMessageQueue code.
	It tests the Construction, Destruction, Add Message 1, Count Messages,
	Is Empty and Find Message 1 use cases.  It does so by doing the following:
		- checks that the queue is empty according to IsEmpty() and CountMessages()
		- adds 10000 messages to the queue and a BList
		- as each is added, checks that IsEmpty() is false and CountMessages()
		  returns what is expected
		- the BList contents is compared to the queue contents (uses FindMessage())
		- the number of BMessages destructed is checked, no messages should have been
		  deleted so far.
		- the queue is deleted
		- the number of BMessages destructed should now be 10000
	
	*/


#include "Test1.h"
#include <stdio.h>
#include <Message.h>
#include <MessageQueue.h>
#include "MessageQueue.h"

		
template<class MessageQueue>
	Test1<MessageQueue>::Test1(MessageQueue *MessageQueueArg, const char *nameArg) :
					TestMessageQueue<MessageQueue>(MessageQueueArg, nameArg)
{
	}


template<class MessageQueue>
	Test1<MessageQueue>::~Test1()
{
	}
	

template<class MessageQueue> bool Test1<MessageQueue>::PerformTest(void)
{
	if (!theMessageQueue->IsEmpty()) {
		printf("Expected empty message queue!\n");
		return(false);
	}
	if (theMessageQueue->CountMessages() != 0) {
		printf("Expected to find no messages on queue!\n");
		return(false);
	}
	
	int i;
	for(i = 0; i < 10000; i++) {
		BMessage *theMessage = new testMessageClass(i);
		AddMessage(theMessage);
		if (theMessageQueue->IsEmpty()) {
			printf("Expected a non-empty message queue!\n");
			return(false);
		}
		if (theMessageQueue->CountMessages() != i + 1) {
			printf("Expected %d messages on the queue!\n", i+1);
			return(false);
		}
	}

	if (!CheckQueueAgainstList()) {
		printf("The message queue doesn't match the list!\n");
		return(false);
	}
	
	if (testMessageClass::messageDestructorCount != 0) {
		printf("Expected no messages to be destructed!\n");
		return(false);
	}
	delete theMessageQueue;
	theMessageQueue = NULL;
		
	if (testMessageClass::messageDestructorCount != 10000) {
		printf("Expected 10000 messages to be destructed!\n");
		return(false);
	}
	
	return(true);
}

template class Test1<BMessageQueue>;
template class Test1<OpenBeOS::BMessageQueue>;