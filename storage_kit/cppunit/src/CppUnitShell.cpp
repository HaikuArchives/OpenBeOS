//----------------------------------------------------------------------
//	CppUnitShell.cpp - Copyright 2002 Tyler Dauwalder	
//	This software is release under the GNU Lesser GPL
//	See the accompanying CppUnitShell.LICENSE file, or wander
//	over to: http://www.gnu.org/copyleft/lesser.html
//----------------------------------------------------------------------
#include "CppUnitShell.h"

#include <cppunit/Exception.h>
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestSuite.h>
#include <set>
#include <map>
#include <string>
#include <vector>

CppUnitShell::CppUnitShell(const std::string &description) : fVerbosityLevel(v2),
fDescription(description) {
};

CppUnitShell::~CppUnitShell()
{
}

void
CppUnitShell::AddSuite(const std::string &name, const SuiteFunction suite) {
	if (suite != NULL)
		fTests[name] = suite;
}

int
CppUnitShell::Run(int argc, char *argv[]) {
	// Parse the command line args
	if (!ProcessArguments(argc, argv))
		return 0;		

	// Add the proper tests to our suite (or exit if there
	// are no tests installed).
	CppUnit::TestSuite suite;	
	if (fTests.empty()) {
	
		// No installed tests whatsoever, so bail
		cout << "ERROR: No installed tests to run!" << endl;
		return 0;
		
	} else if (fTestsToRun.empty()) {
	
		// None specified, so run them all
		std::map<std::string, SuiteFunction>::iterator i;
		for (i = fTests.begin(); i != fTests.end(); ++i)
			suite.addTest( i->second() );
			
	} else {
	
		// One or more specified, so only run those
		std::set<std::string>::const_iterator i;
		for (i = fTestsToRun.begin(); i != fTestsToRun.end(); ++i) 
			suite.addTest( fTests[*i]() );
			
	}
	
	// Run all the tests
	InitOutput();
	suite.run(&fTestResults);
	PrintResults();

	return 0;
}

CppUnitShell::VerbosityLevel
CppUnitShell::Verbosity() const {
	return fVerbosityLevel;
} 

void
CppUnitShell::PrintDescription(int argc, char *argv[]) {
	cout << endl << fDescription;
}

void
CppUnitShell::PrintHelp() {
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

bool
CppUnitShell::ProcessArguments(int argc, char *argv[]) {
	// If we're given no parameters, the default settings
	// will do just fine
	if (argc < 2)
		return true;

	// Handle each command line argument (skipping the first
	// which is just the app name)
	for (int i = 1; i < argc; i++) {
		std::string str(argv[i]);
		
		if (str == "--help") {
			PrintDescription(argc, argv);
			PrintHelp();
			return false;
		}
		else if (str == "--list") {
			// Print out the list of installed tests
			cout << "------------------------------------------------------------------------------" << endl;
			cout << "Available Tests:" << endl;
			cout << "------------------------------------------------------------------------------" << endl;
			map<std::string, SuiteFunction>::const_iterator i;			
			for (i = fTests.begin(); i != fTests.end(); ++i)
				cout << i->first << endl;
			cout << endl;
			return false;
		}
		else if (str == "-v0") {
			fVerbosityLevel = v0;
		}
		else if (str == "-v1") {
			fVerbosityLevel = v1;
		}
		else if (str == "-v2") {
			fVerbosityLevel = v2;
		}
		else if	(fTests.find(str) != fTests.end()) {
			fTestsToRun.insert(str);
		}
		else {
			cout << endl << "ERROR: Invalid argument \"" << str << "\"" << endl;
			PrintHelp();
			return false;
		}
			
	}
	
	return true;
}

void
CppUnitShell::InitOutput() {
	// For vebosity level 2, we output info about each test
	// as we go. This involves a custom CppUnit::TestListener
	// class.
	if (fVerbosityLevel == v2) {
		cout << "------------------------------------------------------------------------------" << endl;
		cout << "Tests" << endl;
		cout << "------------------------------------------------------------------------------" << endl;
		fTestResults.addListener(new CppUnitShell::TestListener);
	}
}

void
CppUnitShell::PrintResults() {

	if (fVerbosityLevel > v0) {
		// Print out detailed results for verbosity levels > 0
		cout << "------------------------------------------------------------------------------" << endl;
		cout << "Results " << endl;
		cout << "------------------------------------------------------------------------------" << endl;

		// Print failures and errors if there are any, otherwise just say "PASSED"
		vector<CppUnit::TestFailure*>::const_iterator iFailure;
		if (fTestResults.failures().size() > 0 || fTestResults.errors().size() > 0) {
			if (fTestResults.failures().size() > 0) {
				cout << "- FAILURES: " << fTestResults.testFailures() << endl;
				for (iFailure = fTestResults.failures().begin(); iFailure != fTestResults.failures().end(); ++iFailure)
					cout << "    " << (*iFailure)->toString() << endl;
			}
			if (fTestResults.errors().size() > 0) {
				cout << "- ERRORS: " << fTestResults.testErrors() << endl;
				for (iFailure = fTestResults.errors().begin(); iFailure != fTestResults.errors().end(); ++iFailure)
					cout << "    " << (*iFailure)->toString() << endl;
			}
			
		}
		else
			cout << "+ PASSED" << endl;
			
		cout << endl;
			
	}
	else {
		// Print out concise results for verbosity level == 0
		if (fTestResults.failures().size() > 0 || fTestResults.errors().size() > 0)
			cout << "- FAILED" << endl;
		else
			cout << "+ PASSED" << endl;
	}
	
}
