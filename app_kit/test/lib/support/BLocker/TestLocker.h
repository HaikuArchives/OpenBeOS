/*
	TestLocker.h
	
	This file defines a set of classes for testing BLocker
	functionality.
	
	*/


#ifndef TESTLOCKER_H
#define TESTLOCKER_H


#include <stdio.h>
#include <be/kernel/OS.h>

	
template<class Locker> class TestLocker {
	
private:	
	const char *testName;

protected:
	Locker *theLocker;
	
	virtual bool PerformTest(void) = 0;
	bool CheckLock(int32);
	thread_id CreateAndRunThread(thread_func, const char *, void *);
	
public:
	TestLocker(Locker *, const char *);
	virtual ~TestLocker();
	bool RunTest(void) { printf("Running %s test\n", testName);
	                     return(PerformTest());
	                   };
	};
	
#endif