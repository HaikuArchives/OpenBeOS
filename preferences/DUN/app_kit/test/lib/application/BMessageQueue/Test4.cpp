/*
	$Id$
	
	This file implements the second test for the OpenBeOS BMessageQueue code.
	It tests the Add Message 1 and Find Message 2 use cases.  It does so by
	doing the following:
		- creates numPerWhatCode * numWhatCodes messages and puts them on
		  the queue and the list
		- searches for the xth instance of a what code using FindMessage()
		  and by searching the list
		- compares the result of the two
		- there are many cases where FindMessage() returns NULL and those are
		  checked also
		- it does all searches for all the what codes used plus two not
		  actually used
	
	*/


#include "Test4.h"
#include <stdio.h>
#include <Message.h>
#include <MessageQueue.h>
#include "MessageQueue.h"


const int numWhatCodes = 20;
const int numPerWhatCode = 50;

		
template<class MessageQueue>
	Test4<MessageQueue>::Test4(MessageQueue *MessageQueueArg, const char *nameArg) :
					TestMessageQueue<MessageQueue>(MessageQueueArg, nameArg)
{
	}


template<class MessageQueue>
	Test4<MessageQueue>::~Test4()
{
	}
	

template<class MessageQueue> bool Test4<MessageQueue>::PerformTest(void)
{
	int whatCode;
	int i;
	BMessage *theMessage;
	BMessage *listMessage;
	
	for (i = 0; i < numPerWhatCode; i++) {
		for (whatCode = 1; whatCode <= numWhatCodes; whatCode++) {
			AddMessage(new testMessageClass(whatCode));
		}
	}
	
	for (whatCode = 0; whatCode <= numWhatCodes + 1; whatCode++) {
		int index = 0;
		while ((theMessage = theMessageQueue->FindMessage(whatCode,
														   index)) != NULL) {
			listMessage = FindMessage(whatCode, index);
			if (listMessage != theMessage) {
				printf("Message pointers do not match!\n");
				return(false);
			}
			index++;
		}
		listMessage = FindMessage(whatCode, index);
		if (listMessage != theMessage) {
			printf("Message pointers do not match!\n");
			return(false);
		}
	}	
	
	return(true);
}

template class Test4<BMessageQueue>;
template class Test4<OpenBeOS::BMessageQueue>;