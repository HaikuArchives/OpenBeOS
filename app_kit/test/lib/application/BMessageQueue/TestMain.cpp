/*
	$Id$
	
	This file implements the main control for the BMessageQueue tests.
	
	*/
	

#include "TestMessageQueue.h"	
#include "Test1.h"	
#include "Test2.h"
#include "Test3.h"
#include "Test4.h"
#include <stdio.h>
#include <OS.h>
#include <MessageQueue.h>
#include "MessageQueue.h"

int main(void)
{
	int failedTests = 0;
	int testsRun = 0;
	
	TestMessageQueue<BMessageQueue> *testListOrig[] = {
						  new Test1<BMessageQueue>(new BMessageQueue,
							                 "Test1"),
						  new Test2<BMessageQueue>(new BMessageQueue,
							                 "Test2"),
						  new Test3<BMessageQueue>(new BMessageQueue,
							                 "Test3"),
						  new Test4<BMessageQueue>(new BMessageQueue,
							                 "Test4"),
						  NULL
						  };
	TestMessageQueue<OpenBeOS::BMessageQueue> *testListNew[] = {
						  new Test1<OpenBeOS::BMessageQueue>(
						  		new OpenBeOS::BMessageQueue,
							    "Test1"),
						  new Test2<OpenBeOS::BMessageQueue>(
						  		new OpenBeOS::BMessageQueue,
							    "Test2"),
						  new Test3<OpenBeOS::BMessageQueue>(
						  		new OpenBeOS::BMessageQueue,
							    "Test3"),
						  new Test4<OpenBeOS::BMessageQueue>(
						  		new OpenBeOS::BMessageQueue,
							    "Test4"),
						  NULL
						  };
	bigtime_t startTime;
	
	printf("Testing Be's BMessageQueue implementation:\n");						
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
	
	printf("\nTesting OpenBeOS BMessageQueue implementation:\n");						
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
