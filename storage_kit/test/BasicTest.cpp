// BasicTest.cpp

#include <stdio.h>
#include <string>

#include "BasicTest.h"

// constructor
BasicTest::BasicTest()
		 : StorageKit::TestCase(),
		   fSubTestNumber(0)
{
}

// setUp
void
BasicTest::setUp()
{
	SaveCWD();
	fSubTestNumber = 0;
}

// tearDown
void
BasicTest::tearDown()
{
	RestoreCWD();
	nextSubTestBlock();
}

// nextSubTest
void
BasicTest::nextSubTest()
{
	if (shell.BeVerbose()) {
		printf("[%ld]", fSubTestNumber++);
		fflush(stdout);
	}
}

// nextSubTestBlock
void
BasicTest::nextSubTestBlock()
{
	if (shell.BeVerbose())
		printf("\n");
	fSubTestNumber = 0;
}

// execCommand
//
// Calls system() with the supplied string.
void
BasicTest::execCommand(const string &cmdLine)
{
	system(cmdLine.c_str());
}

