/*
	$Id$
	
	This file implements a test class for testing BLocker construction functionality.
	It checks the "Construction 1", "Construction 2" and "Sem" use cases.  It does
	so by testing all the documented constructors and uses the Sem() member function
	to confirm that the name and style were set correctly.
	
	*/


#include "Test2.h"
#include <stdio.h>
#include <be/kernel/OS.h>
#include <be/support/Locker.h>
#include "Locker.h"

		
template<class Locker>
	Test2<Locker>::Test2(const char *nameArg) : TestLocker<Locker>(new Locker, nameArg)
{
	}


template<class Locker>
	Test2<Locker>::~Test2()
{
	}
	
	
template<class Locker> bool Test2<Locker>::NameMatches(const char *name, 
                                                       Locker *lockerArg)
{
	sem_info theSemInfo;
	
	if (get_sem_info(lockerArg->Sem(), &theSemInfo) != B_OK) {
		printf("Error in get_sem_info()!\n");
		return(false);
	}
	return(strcmp(name, theSemInfo.name) == 0);
	}
	
	
template<class Locker> bool Test2<Locker>::IsBenaphore(Locker *lockerArg)
{
	int32 semCount;
	
	if (get_sem_count(lockerArg->Sem(), &semCount) != B_OK) {
		printf("Error in get_sem_count()!\n");
		return(false);
	}
	switch (semCount) {
		case 0: return(true);
				break;
		case 1: return(false);
				break;
		default:
				printf("Bad semaphore count, cannot determine if it is a benaphore!\n");
				break;
		}
	return(false);
	}
	

template<class Locker> bool Test2<Locker>::PerformTest(void)
{
	bool result = true;
	
	if (!NameMatches("some BLocker", theLocker)) {
		printf("Default name not set correctly!\n");
		result = false;
	}
	if (!IsBenaphore(theLocker)) {
		printf("Default was semaphore but should be benaphore!\n");
		result = false;
	}
	
	Locker locker1("test string");
	if (!NameMatches("test string", &locker1)) {
		printf("Name for locker1 is wrong!\n");
		result = false;
	}
	if (!IsBenaphore(&locker1)) {
		printf("Default was semaphore but should be benaphore for locker1!\n");
		result = false;
	}
	
	Locker locker2(false);
	if (!NameMatches("some BLocker", &locker2)) {
		printf("Name for locker2 is wrong!\n");
		result = false;
	}
	if (IsBenaphore(&locker2)) {
		printf("BLocker was benaphore and should be semaphore for locker2!\n");
		result = false;
	}
	
	Locker locker3(true);
	if (!NameMatches("some BLocker", &locker3)) {
		printf("Name for locker3 is wrong!\n");
		result = false;
	}
	if (!IsBenaphore(&locker3)) {
		printf("BLocker was semaphore and should be benaphore for locker3!\n");
		result = false;
	}
	
	Locker locker4("test string", false);
	if (!NameMatches("test string", &locker4)) {
		printf("Name for locker4 is wrong!\n");
		result = false;
	}
	if (IsBenaphore(&locker4)) {
		printf("BLocker was benaphore and should be semaphore for locker4!\n");
		result = false;
	}
	
	Locker locker5("test string", true);
	if (!NameMatches("test string", &locker5)) {
		printf("Name for locker5 is wrong!\n");
		result = false;
	}
	if (!IsBenaphore(&locker5)) {
		printf("BLocker was semaphore and should be benaphore for locker5!\n");
		result = false;
	}
	
	return(result);
}


template class Test2<BLocker>;
template class Test2<OpenBeOS::BLocker>;