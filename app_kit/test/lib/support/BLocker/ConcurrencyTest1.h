/*
	$Id$
	
	This file defines a class for performing one test of BLocker
	functionality.
	
	*/


#ifndef ConcurrencyTest1_H
#define ConcurrencyTest1_H


#include "LockerTestCase.h"
#include "ThreadedTestCaller.h"

	
template<class Locker> class ConcurrencyTest1 :
	public LockerTestCase<Locker> {
	
private:
	typedef ThreadedTestCaller <ConcurrencyTest1<Locker> >
		ConcurrencyTest1Caller;
	bool lockTestValue;

	bool AcquireLock(int, bool);
	
public:
	ConcurrencyTest1(std::string, bool);
	virtual ~ConcurrencyTest1();
	void setUp(void);
	void TestThread(void);
	static Test *suite(void);
	};
	
#endif