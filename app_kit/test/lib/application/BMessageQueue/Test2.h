/*
	Test2.h
	
	This file defines a classes for performing one test of BMessageQueue
	functionality.
	
	*/


#ifndef Test2_H
#define Test2_H


#include "TestMessageQueue.h"

	
template<class MessageQueue> class Test2 : public TestMessageQueue<MessageQueue> {
	
private:
	
protected:
	virtual bool PerformTest(void);
	
public:
	Test2(MessageQueue *, const char *);
	virtual ~Test2();
	};
	
#endif