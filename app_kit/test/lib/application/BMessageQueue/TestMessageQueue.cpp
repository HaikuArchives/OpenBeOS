/*
	TestMessageQueue.cpp
	
	This file implements a base class for testing BMessageQueue functionality.
	
	*/


#include "TestMessageQueue.h"
#include <stdio.h>
#include <OS.h>
#include <MessageQueue.h>
#include "MessageQueue.h"


int testMessageClass::messageDestructorCount = 0;

		
template<class MessageQueue>
	TestMessageQueue<MessageQueue>::TestMessageQueue(MessageQueue *MessageQueueArg,
                                   const char *nameArg) : testName(nameArg), 
                                                          theMessageQueue(MessageQueueArg)
{
	}


template<class MessageQueue>
	TestMessageQueue<MessageQueue>::~TestMessageQueue()
{
	delete theMessageQueue;
	theMessageQueue = NULL;
	}
	
	
template<class MessageQueue>
	thread_id TestMessageQueue<MessageQueue>::CreateAndRunThread(thread_func func,
													 const char *threadName, 
													 void *data)
{
	thread_id theThread = spawn_thread(func, threadName, B_NORMAL_PRIORITY, data);
	resume_thread(theThread);
	return(theThread);
	}
	
template<class MessageQueue>
	bool TestMessageQueue<MessageQueue>::CheckQueueAgainstList(void)
{
	if (theMessageQueue->Lock()) {
		if (theMessageQueue->CountMessages() != messageList.CountItems()) {
			printf("Queue has different number of elements than list!\n");
			return(false);
		}
		int i;
		for (i = 0; i < theMessageQueue->CountMessages(); i++) {
			if (theMessageQueue->FindMessage((int32)i) != messageList.ItemAt(i)) {
				printf("Contents of queue and list are different!\n");
				return(false);
			}
		}
		theMessageQueue->Unlock();
	}
	return(true);
}


template<class MessageQueue>
	void TestMessageQueue<MessageQueue>::AddMessage(BMessage *message)
{
	if (theMessageQueue->Lock()) {
		theMessageQueue->AddMessage(message);
		messageList.AddItem(message);
		theMessageQueue->Unlock();
	}
}
	
template<class MessageQueue>
	void TestMessageQueue<MessageQueue>::RemoveMessage(BMessage *message)
{
	if (theMessageQueue->Lock()) {
		theMessageQueue->RemoveMessage(message);
		messageList.RemoveItem((void *)message);
		theMessageQueue->Unlock();
	}
}
	
template<class MessageQueue>
	BMessage *TestMessageQueue<MessageQueue>::NextMessage(void)
{
	BMessage *result = NULL;
	if (theMessageQueue->Lock()) {
		result = theMessageQueue->NextMessage();
		if (result != messageList.RemoveItem((int32)0)) {
			printf("Message removed from queue doesn't match list!\n");
		}
		theMessageQueue->Unlock();
	}
	return(result);
}
	

template class TestMessageQueue<BMessageQueue>;
template class TestMessageQueue<OpenBeOS::BMessageQueue>;