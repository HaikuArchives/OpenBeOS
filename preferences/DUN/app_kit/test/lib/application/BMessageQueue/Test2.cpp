/*
	$Id$
	
	This file implements the second test for the OpenBeOS BMessageQueue code.
	It tests the Add Message 2 use case.  It does so by doing the following:
		- creates 4 BMessages
		- it adds them to the queue and a list
		- it checks the queue against the list
		- it adds the second one to the queue and the list again
		- it checks the queue against the list again (expecting an error)
	
	*/


#include "Test2.h"
#include <stdio.h>
#include <Message.h>
#include <MessageQueue.h>
#include "MessageQueue.h"

		
template<class MessageQueue>
	Test2<MessageQueue>::Test2(MessageQueue *MessageQueueArg, const char *nameArg) :
					TestMessageQueue<MessageQueue>(MessageQueueArg, nameArg)
{
	}


template<class MessageQueue>
	Test2<MessageQueue>::~Test2()
{
	}
	

template<class MessageQueue> bool Test2<MessageQueue>::PerformTest(void)
{
	BMessage *firstMessage = new BMessage(1);
	BMessage *secondMessage = new BMessage(2);
	BMessage *thirdMessage = new BMessage(3);
	BMessage *fourthMessage = new BMessage(4);
	
	AddMessage(firstMessage);
	AddMessage(secondMessage);
	AddMessage(thirdMessage);
	AddMessage(fourthMessage);
	
	if (!CheckQueueAgainstList()) {
		printf("The message queue doesn't match the list!\n");
		return(false);
	}
	
	AddMessage(secondMessage);
	printf("The following message is normal: ");
	if (CheckQueueAgainstList()) {
		printf("Expected the queue to not match the list!\n");
		return(false);
	}
	
	if (theMessageQueue->FindMessage((int32)0) != firstMessage) {
		printf("Unexpected first message!\n");
		return(false);
	}
	if (theMessageQueue->FindMessage((int32)1) != secondMessage) {
		printf("Unexpected second message!\n");
		return(false);
	}
	if (theMessageQueue->FindMessage((int32)2) != NULL) {
		printf("Unexpected third message!\n");
		return(false);
	}
	if (theMessageQueue->FindMessage((int32)3) != NULL) {
		printf("Unexpected fourth message!\n");
		return(false);
	}
	if (theMessageQueue->FindMessage((int32)4) != NULL) {
		printf("Unexpected fifth message!\n");
		return(false);
	}
	
	return(true);
}

template class Test2<BMessageQueue>;
template class Test2<OpenBeOS::BMessageQueue>;