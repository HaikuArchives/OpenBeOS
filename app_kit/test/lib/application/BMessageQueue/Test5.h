/*
	$Id$
	
	This file defines a classes for performing one test of BMessageQueue
	functionality.
	
	*/


#ifndef Test5_H
#define Test5_H


#include "TestMessageQueue.h"

	
template<class MessageQueue> class Test5 : public TestMessageQueue<MessageQueue> {
	
private:
	bool testResult;
	bool unlockTest;
	bool isLocked;
	BMessage *removeMessage;
	
    void HandleTestFailure(const char *);
	static int32 ThreadEntry1(Test5<MessageQueue> *);
	static int32 ThreadEntry2(Test5<MessageQueue> *);
	static int32 ThreadEntry3(Test5<MessageQueue> *);
	static int32 ThreadEntry4(Test5<MessageQueue> *);
	void TestNext(void);
	void TestRemove(void);
	void TestAdd(void);
	void TestLock(void);
	
protected:
	virtual bool PerformTest(void);
	
public:
	Test5(MessageQueue *, const char *);
	virtual ~Test5();
	};
	
#endif