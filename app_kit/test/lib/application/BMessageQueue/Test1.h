/*
	Test1.h
	
	This file defines a classes for performing one test of BMessageQueue
	functionality.
	
	*/


#ifndef TEST1_H
#define TEST1_H


#include "TestMessageQueue.h"

	
template<class MessageQueue> class Test1 : public TestMessageQueue<MessageQueue> {
	
private:
	
protected:
	virtual bool PerformTest(void);
	
public:
	Test1(MessageQueue *, const char *);
	virtual ~Test1();
	};
	
#endif