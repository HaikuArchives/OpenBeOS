/*
	$Id$
	
	This file defines a set of classes for testing BMessageQueue
	functionality.
	
	*/


#ifndef TESTMessageQueue_H
#define TESTMessageQueue_H


#include <OS.h>
#include <List.h>
#include <Locker.h>
#include <Message.h>
#include <stdio.h>


class testMessageClass : public BMessage
{
public:
	static int messageDestructorCount;
	virtual ~testMessageClass() { messageDestructorCount++; };
	testMessageClass(int what) : BMessage(what) { };
};

	
template<class MessageQueue> class TestMessageQueue {
	
private:	
	const char *testName;
	BList messageList;

protected:
	MessageQueue *theMessageQueue;
	
	void AddMessage(BMessage *message);
	void RemoveMessage(BMessage *message);
	BMessage *NextMessage(void);
	BMessage *FindMessage(uint32 what, int index);
	
	bool CheckQueueAgainstList(void);
	
	virtual bool PerformTest(void) = 0;
	thread_id CreateAndRunThread(thread_func, const char *, void *);
	
public:
	TestMessageQueue(MessageQueue *, const char *);
	virtual ~TestMessageQueue();
	bool RunTest(void) { printf("Running %s test\n", testName);
						 testMessageClass::messageDestructorCount = 0;
	                     return(PerformTest());
	                   };
	};
	
#endif