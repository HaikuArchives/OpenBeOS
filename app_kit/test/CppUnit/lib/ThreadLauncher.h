#ifndef THREADLAUNCHER_H
#define THREADLAUNCHER_H

#include <string>
#include <signal.h>
#include <kernel/OS.h>
#include "CppUnitException.h"
#include "TestResult.h"


template <class Fixture> class ThreadLauncher {
private:
    typedef void             (Fixture::*TestMethod)();
	std::string threadName;
	Fixture *theTest;
	TestMethod theMethod;
	thread_id theThread;
	TestResult *theResult;
	
	static long entry(ThreadLauncher *);
	void LaunchFunc(void);
	
protected:

public:
	ThreadLauncher(std::string aName, Fixture *aClass, TestMethod arg);
	~ThreadLauncher();
	void run(TestResult *result);
	int32 Stop(void);
	int32 WaitForThread(void);
	bool IsRunning(void);
	};
	
	
template <class Fixture> ThreadLauncher<Fixture>::ThreadLauncher(std::string aName, Fixture *aClass,
	TestMethod arg) : threadName(aClass->name() + aName), theTest(aClass), theMethod(arg),
	theThread(0), theResult(NULL)
{
	}
	
	
template <class Fixture> ThreadLauncher<Fixture>::~ThreadLauncher()
{
	Stop();
	}
	
	
template <class Fixture> int32 ThreadLauncher<Fixture>::WaitForThread(void)
{
	int32 result = 0;
	if (find_thread(NULL) != theThread) {
		wait_for_thread(theThread, &result);
		}
	return(result);
	}
		
	
template <class Fixture> int32 ThreadLauncher<Fixture>::Stop(void)
{
	int32 result = 0;
	if (find_thread(NULL) != theThread) {
		while (IsRunning()) {
			kill(theThread, SIGINT);
			snooze(1000000);
			}
		result = WaitForThread();
		}
	return(result);
	}
	
	
template <class Fixture> bool ThreadLauncher<Fixture>::IsRunning(void)
{
	if (theThread != 0) {
		thread_info myThreadInfo;
		if (get_thread_info(theThread, &myThreadInfo) == B_OK)
			return(true);
		theThread = 0;
		}
	return(false);
	}
	
	
template <class Fixture> void ThreadLauncher<Fixture>::run(TestResult *result)
{
	if (IsRunning())
		return;

	theResult = result;
	theThread = spawn_thread((thread_entry)(ThreadLauncher::entry),
					threadName.c_str(), B_NORMAL_PRIORITY, this);
	if ((theThread == B_NO_MORE_THREADS) || (theThread == B_NO_MEMORY)) {
		theThread = 0;
		}
	else {
		resume_thread(theThread);
		}

	return;
	}
	
	
template <class Fixture> void ThreadLauncher<Fixture>::LaunchFunc(void)
{
	try {
		(theTest->*theMethod)();
    }
    catch (CppUnitException e) {
        CppUnitException *copy = new CppUnitException (e);
        theResult->addFailure (theTest, copy);
    }
    catch (exception e) {
        theResult->addError (theTest, new CppUnitException (e.what ()));
    }
    catch (...) {
        CppUnitException *e = new CppUnitException ("unknown exception");
        theResult->addError (theTest, e);
    }
}
	
	
template <class Fixture> long ThreadLauncher<Fixture>::entry(ThreadLauncher *arg)
{
	arg->LaunchFunc();
	return(0);
	}
	
#endif