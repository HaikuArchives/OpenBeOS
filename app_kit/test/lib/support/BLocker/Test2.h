/*
	Test2.h
	
	This file defines a classes for performing one test of BLocker
	functionality.
	
	*/


#ifndef TEST2_H
#define TEST2_H


#include "TestLocker.h"

	
template<class Locker> class Test2 : public TestLocker<Locker> {
	
private:
	bool NameMatches(const char *, Locker *);
	bool IsBenaphore(Locker *);
	
protected:
	virtual bool PerformTest(void);
	
public:
	Test2(const char *);
	virtual ~Test2();
	};
	
#endif