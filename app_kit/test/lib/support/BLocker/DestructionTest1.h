/*
	$Id$
	
	This file defines a class for performing one test of BLocker
	functionality.
	
	*/


#ifndef DestructionTest1_H
#define DestructionTest1_H


#include "LockerTestCase.h"
#include "ThreadedTestCaller.h"

	
template<class Locker> class DestructionTest1 : public LockerTestCase<Locker> {
	
private:
	typedef ThreadedTestCaller <DestructionTest1<Locker> >
		DestructionTest1Caller;

protected:
	
public:
	void TestThread1(void);
	void TestThread2(void);
	DestructionTest1(std::string name, bool isBenaphore);
	virtual ~DestructionTest1();
	static Test *suite(void);
	};
	
#endif