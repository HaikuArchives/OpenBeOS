/*
	$Id$
	
	This file defines a classes for performing one test of BMessageQueue
	functionality.
	
	*/


#ifndef Test4_H
#define Test4_H


#include "TestMessageQueue.h"

	
template<class MessageQueue> class Test4 : public TestMessageQueue<MessageQueue> {
	
private:
	
protected:
	virtual bool PerformTest(void);
	
public:
	Test4(MessageQueue *, const char *);
	virtual ~Test4();
	};
	
#endif