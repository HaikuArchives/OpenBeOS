
#ifndef CPPUNIT_THREADEDTESTCALLER_H
#define CPPUNIT_THREADEDTESTCALLER_H

#include <memory>
#include <vector>

#ifndef CPPUNIT_GUARDS_H
#include "Guards.h"
#endif

#ifndef CPPUNIT_TESTCASE_H
#include "TestCase.h"
#endif

#include "ThreadLauncher.h"


class TestResult;


template <class Fixture> class ThreadedTestCaller : public TestCase
{ 
	REFERENCEOBJECT (ThreadedTestCaller)

	typedef void             (Fixture::*TestMethod)();
	typedef class ThreadLauncher<Fixture> FixtureThreadLauncher;
   
public:
	ThreadedTestCaller (std::string name, Fixture *theFixture)
		: TestCase (theFixture->name() + name), m_fixture (theFixture)
		{}
	
	virtual ~ThreadedTestCaller();
    void run(TestResult *result); 
    void addThread(std::string threadName, TestMethod theMethod);
    virtual std::string toString(void);

protected:

    void                    setUp ()
                            { m_fixture->setUp (); }

    void                    tearDown ()
                            { m_fixture->tearDown (); }

private:
   Fixture   *m_fixture;
   std::vector<FixtureThreadLauncher *> m_threads;

};


// Returns the name of the test case instance
template <class Fixture> std::string ThreadedTestCaller<Fixture>::toString (void)
{ 
	const type_info& thisClass = typeid (*m_fixture);return std::string (thisClass.name()) + "." + name ();
	}

template <class Fixture> ThreadedTestCaller<Fixture>::~ThreadedTestCaller()
{
	for (std::vector<FixtureThreadLauncher *>::iterator it = m_threads.begin();
		it != m_threads.end ();
        ++it) {
		delete *it;
		}
	delete m_fixture;
	}


template <class Fixture> void ThreadedTestCaller<Fixture>::addThread(std::string threadName,
	TestMethod theMethod)
{
	m_threads.push_back(new ThreadLauncher<Fixture>(threadName, m_fixture, theMethod));
	}


template <class Fixture> void ThreadedTestCaller<Fixture>::run(TestResult *result)
{
    result->startTest (this);

    setUp ();

	for (std::vector<FixtureThreadLauncher *>::iterator it = m_threads.begin();
		it != m_threads.end ();
        ++it) {
		(*it)->run(result);
	}
	
	for (std::vector<FixtureThreadLauncher *>::iterator it = m_threads.begin();
		it != m_threads.end ();
        ++it) {
		(*it)->WaitForThread();
		delete (*it);
	}
	m_threads.clear();

    tearDown ();

    result->endTest (this);
}

#endif
