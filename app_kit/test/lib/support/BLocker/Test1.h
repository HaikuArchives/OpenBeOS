/*
	Test1.h
	
	This file defines a classes for performing one test of BLocker
	functionality.
	
	*/


#ifndef TEST1_H
#define TEST1_H


#include "TestLocker.h"

	
template<class Locker> class Test1 : public TestLocker<Locker> {
	
private:
	bool testResult;
	bool lockTestValue;

	static int32 ThreadEntry(Test1<Locker> *);
	void TestThread(void);
	bool AcquireLock(int, bool);
	void HandleTestFailure(const char *, int);
	
protected:
	virtual bool PerformTest(void);
	
public:
	Test1(Locker *, const char *);
	virtual ~Test1();
	};
	
#endif