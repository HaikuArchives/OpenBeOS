/*
	Test4.h
	
	This file defines a classes for performing one test of BLocker
	functionality.
	
	*/


#ifndef TEST4_H
#define TEST4_H


#include "TestLocker.h"

	
template<class Locker> class Test4 : public TestLocker<Locker> {
	
private:
	bool testResult;

    void HandleTestFailure(const char *);
	static int32 ThreadEntry(Test4<Locker> *);
	void TestThread(void);
	
protected:
	virtual bool PerformTest(void);
	
public:
	Test4(Locker *, const char *);
	virtual ~Test4();
	};
	
#endif