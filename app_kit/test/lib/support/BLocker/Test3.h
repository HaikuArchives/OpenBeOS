/*
	$Id$
	
	This file defines a classes for performing one test of BLocker
	functionality.
	
	*/


#ifndef TEST3_H
#define TEST3_H


#include "TestLocker.h"

	
template<class Locker> class Test3 : public TestLocker<Locker> {
	
private:
	bool testResult;

    void HandleTestFailure(const char *);
	static int32 ThreadEntry(Test3<Locker> *);
	void TestThread(void);
	
protected:
	virtual bool PerformTest(void);
	
public:
	Test3(Locker *, const char *);
	virtual ~Test3();
	};
	
#endif