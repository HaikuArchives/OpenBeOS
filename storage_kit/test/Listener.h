#ifndef __sk_listener_h__
#define __sk_listener_h__

#include <cppunit/TestListener.h>

namespace StorageKit {

// Handles printing of test progress info
/* Receives notification of the beginning and end of each test,
	as well as all failures and errors for each test. Prints out
	said test information in a standard format to standard output.
*/
class TestListener : public ::CppUnit::TestListener {
protected:
	bool ok;
	
public:

    virtual void startTest( CppUnit::Test *test ) {
    	ok = true;
    	cout << test->getName() << flush;
    }
    
    virtual void addError( CppUnit::Test *test, CppUnit::Exception *e ) {
    	ok = false;
    	cout << endl << "  - ERROR -- " << e->what() << flush;
    }

    virtual void addFailure( CppUnit::Test *test, CppUnit::Exception *e ) {
    	ok = false;
    	cout << endl <<  "  - FAILURE -- " << e->what() << flush;
    }

    virtual void endTest( CppUnit::Test *test )  {
    	if (ok)
    		cout << endl << "  + PASSED";
    	cout << endl << endl;
    }
 

};


};	// namespace StorageKit

#endif // __sk_listener_h__