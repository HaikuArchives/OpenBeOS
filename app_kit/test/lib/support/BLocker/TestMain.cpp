/*
	TestMain.cpp
	
	This file implements the main control for the BLocker tests.
	
	*/
	

#include "TestLocker.h"	
#include "Test1.h"
#include "Test2.h"
#include "Test3.h"
#include "Test4.h"
#include "Test5.h"
#include "Test6.h"
#include "Test7.h"
#include <stdio.h>
#include <OS.h>
#include <Locker.h>
#include "Locker.h"

int main(void)
{
	int failedTests = 0;
	int testsRun = 0;
	
	TestLocker<BLocker> *testListOrig[] = {
						  new Test1<BLocker>(new BLocker(true),
							                 "Test1 Benaphore"),
						  new Test1<BLocker>(new BLocker(false),
							                 "Test1 Semaphore"),
						  new Test2<BLocker>("Test2 Construction Tests"),
						  new Test3<BLocker>(new BLocker(true),
							                 "Test3 Benaphore"),
						  new Test3<BLocker>(new BLocker(false),
							                 "Test3 Semaphore"),
						  new Test4<BLocker>(new BLocker(true),
							                 "Test4 Benaphore"),
						  new Test4<BLocker>(new BLocker(false),
							                 "Test4 Semaphore"),
						  new Test5<BLocker>(new BLocker(true),
							                 "Test5 Benaphore"),
						  new Test5<BLocker>(new BLocker(false),
							                 "Test5 Semaphore"),
						  new Test6<BLocker>("Test6 Benaphore"),
						  new Test7<BLocker>("Test7 Semaphore"),
						  NULL
						  };
	TestLocker<OpenBeOS::BLocker> *testListNew[] = {
						  new Test1<OpenBeOS::BLocker>(
						  		new OpenBeOS::BLocker(true),
							    "Test1 Benaphore"),
						  new Test1<OpenBeOS::BLocker>(
						  		new OpenBeOS::BLocker(false),
							    "Test1 Semaphore"),
						  new Test2<OpenBeOS::BLocker>(
						  		"Test2 Construction Tests"),
						  new Test3<OpenBeOS::BLocker>(
						  		new OpenBeOS::BLocker(true),
							    "Test3 Benaphore"),
						  new Test3<OpenBeOS::BLocker>(
						  		new OpenBeOS::BLocker(false),
							    "Test3 Semaphore"),
						  new Test4<OpenBeOS::BLocker>(
						  		new OpenBeOS::BLocker(true),
							    "Test4 Benaphore"),
						  new Test4<OpenBeOS::BLocker>(
						  		new OpenBeOS::BLocker(false),
							    "Test4 Semaphore"),
						  new Test5<OpenBeOS::BLocker>(
						  		new OpenBeOS::BLocker(true),
							    "Test5 Benaphore"),
						  new Test5<OpenBeOS::BLocker>(
						  		new OpenBeOS::BLocker(false),
							    "Test5 Semaphore"),
						  new Test6<OpenBeOS::BLocker>(
						  		"Test6 Benaphore"),
						  new Test7<OpenBeOS::BLocker>(
						  		"Test7 Semaphore"),
						  NULL
						  };
	bigtime_t startTime;
	
	printf("Testing Be's BLocker implementation:\n");						
	for(int32 i; testListOrig[i] != NULL; i++) {
		testsRun++;
		startTime = system_time();
		if (!testListOrig[i]->RunTest()) {
			printf(">>>Test number %ld failed!\n", i);
			failedTests++;
		}
		printf("        Test time = %lld us\n",
					static_cast<long long>(system_time() - startTime));
		delete testListOrig[i];
	}
	
	printf("\nTesting OpenBeOS BLocker implementation:\n");						
	for(int32 i; testListNew[i] != NULL; i++) {
		testsRun++;
		startTime = system_time();
		if (!testListNew[i]->RunTest()) {
			printf(">>>Test number %ld failed!\n", i);
			failedTests++;
		}
		printf("        Test time = %lld us\n",
					static_cast<long long>(system_time() - startTime));
		delete testListNew[i];
	}

	if (testsRun > 0) {
		printf("\nTests Run = %d  Tests Failed = %d  Pass Rate = %d%%\n",
				testsRun, failedTests,
				((testsRun - failedTests) * 100) / testsRun);
	}
	
	exit(failedTests == 0 ? 0 : 1);
}
