/*
	$Id$
	
	This file defines a classes for performing one test of BLocker
	functionality.
	
	*/


#ifndef TEST5_H
#define TEST5_H


#include "TestLocker.h"

	
template<class Locker> class Test5 : public TestLocker<Locker> {
	
private:
	bool testResult;
	bool lockTestValue;

	static int32 ThreadEntry(Test5<Locker> *);
	void TestThread(void);
	bool AcquireLock(int, bool);
	void HandleTestFailure(const char *, int);
	void LockingLoop(void);
	
protected:
	virtual bool PerformTest(void);
	
public:
	Test5(Locker *, const char *);
	virtual ~Test5();
	};
	
#endif