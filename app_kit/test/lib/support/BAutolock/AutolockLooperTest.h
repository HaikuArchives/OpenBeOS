/*
	$Id$
	
	This file defines the class for performing all BAutolock tests on a
	BLooper.
	
	*/


#ifndef AutolockLooperTest_H
#define AutolockLooperTest_H


#include "ThreadedTestCaller.h"
#include "TestCase.h"
	
template<class Autolock, class Looper> class AutolockLooperTest : public TestCase {
	
private:
	typedef ThreadedTestCaller <AutolockLooperTest<Autolock, Looper> >
		AutolockLooperTestCaller;
		
	Looper *theLooper;
	
public:
	static Test *suite(void);
	void TestThread1(void);
	AutolockLooperTest(std::string);
	virtual ~AutolockLooperTest();
	};
	
#endif