#include <cppunit/Exception.h>
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestSuite.h>
#include <set>
#include <map>
#include <string>
#include <vector>

#include "Listener.h"

// ##### Include your test header here #####
#include "NodeTest.h"
#include "PathTest.h"

// Typedef for a function that takes no parameters
// and returns a CppUnit::Test pointer
typedef CppUnit::Test* (*SuiteFunction)(void);

enum VerbosityLevel { v0, v1, v2 };

// Contains all the system-wide parameters that need to
// be configured (by command-line args) and then interpereted
// (to produce the proper output).
class TestParameters {
public:
	TestParameters() : fVerbosityLevel(v2) {};
	
	VerbosityLevel fVerbosityLevel;
	std::set<std::string> fTestsToRun;
	std::map<std::string, SuiteFunction> fTests;
	CppUnit::TestResult fTestResults;
};

// Prints a brief description of the program and a guess as to
// which Storage Kit library the app was linked with based on
// the filename of the app
void PrintDescription(std::string AppName);

// Prints out command line argument instructions
void PrintHelp();

// Handles command line arguments; returns true if everything goes
// okay, false if not (or if the program just needs to terminate without
// running any tests). Modifies settings in "settings" as necessary.
bool ProcessArguments(int argc, char *argv[], TestParameters &settings);

// Makes any necessary pre-test preparations
void InitOutput(TestParameters &parms);

// Prints out the test results in the proper format per
// the specified verbosity level.
void PrintResults(TestParameters &parms);

int main(int argc, char *argv[]) {
	TestParameters settings;

	// ##### Add your test suite to the list of available suites here #####
	settings.fTests["BNode"] = &NodeTest::Suite;
	settings.fTests["BPath"] = &PathTest::Suite;	

	// Parse the command line args
	if (!ProcessArguments(argc, argv, settings))
		return 0;		

	// Add the proper tests to our suite
	CppUnit::TestSuite suite;	
	if (settings.fTestsToRun.empty()) {
		// None specified, so run them all
		std::map<std::string, SuiteFunction>::iterator i;
		for (i = settings.fTests.begin(); i != settings.fTests.end(); ++i)
			suite.addTest( i->second() );
	}
	else {
		// One or more specified, so only run those
		std::set<std::string>::const_iterator i;
		for (i = settings.fTestsToRun.begin(); i != settings.fTestsToRun.end(); ++i) 
			suite.addTest( settings.fTests[*i]() );
	}

	InitOutput(settings);
	suite.run(&settings.fTestResults);
	PrintResults(settings);

	return 0;
}

void PrintDescription(std::string AppName) {
	cout << endl;
	cout << "This program is the central testing framework for the purpose" << endl;
	cout << "of testing and verifying the behavior of the OpenBeOS Storage" << endl;
	cout << "Kit." << endl;
	
	if (AppName.rfind("Test.StorageKit.R5") != std::string::npos) {
		cout << endl;
		cout << "Judging by its name (Test.StorageKit.R5), this copy was" << endl;
		cout << "probably linked again Be Inc.'s R5 Storage Kit for the sake" << endl;
		cout << "of comparison against our own implementation." << endl;
	}
	else if (AppName.rfind("Test.StorageKit.OpenBeOS") != std::string::npos) {
		cout << endl;
		cout << "Judging by its name (Test.StorageKit.OpenBeOS), this copy" << endl;
		cout << "was probably linked against the OpenBeOS implementation of" << endl;
		cout << "the Storage Kit." << endl;
	}
}

void PrintHelp() {
	const char indent[] = "  ";
	cout << endl;
	cout << "VALID ARGUMENTS:     " << endl;
	cout << indent << "--help     Displays this help text plus some other garbage" << endl;
	cout << indent << "--list     Lists the names of classes with installed tests" << endl;
	cout << indent << "-v0        Sets verbosity level to 0 (concise summary only)" << endl;
	cout << indent << "-v1        Sets verbosity level to 1 (complete summary only)" << endl;
	cout << indent << "-v2        Sets verbosity level to 2 (*default* -- per-test results plus" << endl;
	cout << indent << "           complete summary)" << endl;
	cout << indent << "CLASSNAME  Instructs the program to run the test for the given class; if" << endl;
	cout << indent << "           no classes are specified, all tests are run" << endl;
	cout << endl;
	
}

bool ProcessArguments(int argc, char *argv[], TestParameters &settings) {
	// If we're given no parameters, go with the defaults
	if (argc < 2)
		return true;

	// Handle each command line argument (skipping the first
	// which is just the app name)
	for (int i = 1; i < argc; i++) {
		std::string str(argv[i]);
		
		if (str == "--help") {
			PrintDescription(argv[0]);
			PrintHelp();
			return false;
		}
		else if (str == "--list") {
			// Print out the list of installed tests
			cout << "------------------------------------------------------------------------------" << endl;
			cout << "Available Tests:" << endl;
			cout << "------------------------------------------------------------------------------" << endl;
			map<std::string, SuiteFunction>::const_iterator i;			
			for (i = settings.fTests.begin(); i != settings.fTests.end(); ++i)
				cout << i->first << endl;
			cout << endl;
			return false;
		}
		else if (str == "-v0") {
			settings.fVerbosityLevel = v0;
		}
		else if (str == "-v1") {
			settings.fVerbosityLevel = v1;
		}
		else if (str == "-v2") {
			settings.fVerbosityLevel = v2;
		}
		else if	(settings.fTests.find(str) != settings.fTests.end()) {
			settings.fTestsToRun.insert(str);
		}
		else {
			cout << endl << "ERROR: Invalid argument \"" << str << "\"" << endl;
			PrintHelp();
			return false;
		}
			
	}
	
	return true;
}


void InitOutput(TestParameters &parms) {
	// For vebosity level 2, we output info about each test
	// as we go. This involves a custom CppUnit::TestListener
	// class.
	if (parms.fVerbosityLevel == v2) {
		cout << "------------------------------------------------------------------------------" << endl;
		cout << "Tests" << endl;
		cout << "------------------------------------------------------------------------------" << endl;
		parms.fTestResults.addListener(new StorageKit::TestListener);
	}
}

void PrintResults(TestParameters &parms) {

	// Aliases to save typing :-)
	CppUnit::TestResult &result = parms.fTestResults;
	VerbosityLevel level = parms.fVerbosityLevel;

	if (level > v0) {
		// Print out detailed results for verbosity levels > 0
		cout << "------------------------------------------------------------------------------" << endl;
		cout << "Results " << endl;
		cout << "------------------------------------------------------------------------------" << endl;

		// Print failures and errors if there are any, otherwise just say "PASSED"
		vector<CppUnit::TestFailure*>::const_iterator iFailure;
		if (result.failures().size() > 0 || result.errors().size() > 0) {
			if (result.failures().size() > 0) {
				cout << "- FAILURES: " << result.testFailures() << endl;
				for (iFailure = result.failures().begin(); iFailure != result.failures().end(); ++iFailure)
					cout << "    " << (*iFailure)->toString() << endl;
			}
			if (result.errors().size() > 0) {
				cout << "- ERRORS: " << result.testErrors() << endl;
				for (iFailure = result.errors().begin(); iFailure != result.errors().end(); ++iFailure)
					cout << "    " << (*iFailure)->toString() << endl;
			}
			
		}
		else
			cout << "+ PASSED" << endl;
			
		cout << endl;
			
	}
	else {
		// Print out concise results for verbosity level == 0
		if (result.failures().size() > 0 || result.errors().size() > 0)
			cout << "- FAILED" << endl;
		else
			cout << "+ PASSED" << endl;
	}
}
