/*
	$Id$
	
	This file defines a classes for performing one test of BLocker
	functionality.
	
	*/


#ifndef Test6_H
#define Test6_H


#include "TestLocker.h"

	
template<class Locker> class Test6 : public TestLocker<Locker> {
	
private:
	bool testResult;

	static int32 ThreadEntry(Test6<Locker> *);
	void TestThread(void);
	bool CheckLockRequests(int);
	
protected:
	virtual bool PerformTest(void);
	
public:
	Test6(const char *);
	virtual ~Test6();
	};
	
#endif