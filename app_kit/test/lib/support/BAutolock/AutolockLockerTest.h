/*
	$Id$
	
	This file defines the class for performing all BAutolock tests on a
	BLocker.
	
	*/


#ifndef AutolockLockerTest_H
#define AutolockLockerTest_H


#include "ThreadedTestCaller.h"
#include "TestCase.h"
	
template<class Autolock, class Locker> class AutolockLockerTest : public TestCase {
	
private:
	typedef ThreadedTestCaller <AutolockLockerTest<Autolock, Locker> >
		AutolockLockerTestCaller;
		
	Locker *theLocker;
	
public:
	static Test *suite(void);
	void TestThread1(void);
	void TestThread2(void);
	void TestThread3(void);
	AutolockLockerTest(std::string);
	virtual ~AutolockLockerTest();
	};
	
#endif