/*
	$Id$
	
	This file defines a classes for performing one test of BMessageQueue
	functionality.
	
	*/


#ifndef TEST3_H
#define TEST3_H


#include "TestMessageQueue.h"

	
template<class MessageQueue> class Test3 : public TestMessageQueue<MessageQueue> {
	
private:
	bool testResult;
	bool useList;
	
    void HandleTestFailure(const char *);
	static int32 ThreadEntry1(Test3<MessageQueue> *);
	static int32 ThreadEntry2(Test3<MessageQueue> *);
	void TestNext(void);
	void TestRemove(void);
	
protected:
	virtual bool PerformTest(void);
	
public:
	Test3(MessageQueue *, const char *);
	virtual ~Test3();
	};
	
#endif