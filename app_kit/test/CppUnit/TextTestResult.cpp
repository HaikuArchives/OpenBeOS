
#include "Test.h"
#include "TextTestResult.h"
#include "CppUnitException.h"
#include "estring.h"

using namespace std;

void TextTestResult::addError (Test *test, CppUnitException *e)
{
    TestResult::addError (test, e);
    cerr << " Error!";

}

void TextTestResult::addFailure (Test *test, CppUnitException *e)
{
    TestResult::addFailure (test, e);
    cerr << " Failed!";

}

void TextTestResult::startTest (Test *test)
{
    TestResult::startTest (test);
    cerr << "Running test: " << test->toString();
    testStartTime = system_time();
}


void TextTestResult::endTest (Test *test)
{
	bigtime_t testRunTime = system_time() - testStartTime;
    TestResult::endTest (test);
    cerr << " (" << testRunTime << " us)" << endl;
}

void TextTestResult::printErrors (ostream& stream)
{
    if (testErrors () != 0) {

        if (testErrors () == 1)
            stream << "There was " << testErrors () << " error: " << endl;
        else
            stream << "There were " << testErrors () << " errors: " << endl;

        int i = 1;

        for (vector<TestFailure *>::iterator it = errors ().begin (); it != errors ().end (); ++it) {
            TestFailure             *failure    = *it;
            CppUnitException        *e          = failure->thrownException ();

            stream << i 
                   << ") "
                   << "line: " << (e ? estring (e->lineNumber ()) : "") << " "
                   << (e ? e->fileName () : "") << " "
                   << "\"" << failure->thrownException ()->what () << "\""
                   << endl;
            i++;
        }
    }

}

void TextTestResult::printFailures (ostream& stream) 
{
    if (testFailures () != 0) {
        if (testFailures () == 1)
            stream << "There was " << testFailures () << " failure: " << endl;
        else
            stream << "There were " << testFailures () << " failures: " << endl;

        int i = 1;

        for (vector<TestFailure *>::iterator it = failures ().begin (); it != failures ().end (); ++it) {
            TestFailure             *failure    = *it;
            CppUnitException        *e          = failure->thrownException ();

            stream << i 
                   << ") "
                   << "line: " << (e ? estring (e->lineNumber ()) : "") << " "
                   << (e ? e->fileName () : "") << " "
                   << "\"" << failure->thrownException ()->what () << "\""
                   << endl;
            i++;
        }
    }

}


void TextTestResult::print (ostream& stream) 
{
    printHeader (stream);
    printErrors (stream);
    printFailures (stream);

}


void TextTestResult::printHeader (ostream& stream)
{
    if (wasSuccessful ())
        cout << endl << "OK (" << runTests () << " tests)" << endl;
    else
        cout << endl
             << "!!!FAILURES!!!" << endl
             << "Test Results:" << endl
             << "Run:  "
             << runTests ()
             << "   Failures: "
             << testFailures ()
             << "   Errors: "
             << testErrors ()
             << endl;

}
