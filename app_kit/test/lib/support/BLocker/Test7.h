/*
	$Id$
	
	This file defines a classes for performing one test of BLocker
	functionality.
	
	*/


#ifndef Test7_H
#define Test7_H


#include "TestLocker.h"

	
template<class Locker> class Test7 : public TestLocker<Locker> {
	
private:
	bool testResult;

	static int32 ThreadEntry(Test7<Locker> *);
	void TestThread(void);
	bool CheckLockRequests(int);
	
protected:
	virtual bool PerformTest(void);
	
public:
	Test7(const char *);
	virtual ~Test7();
	};
	
#endif