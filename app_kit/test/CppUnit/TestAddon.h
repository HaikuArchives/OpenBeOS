#include "Test.h"

#ifndef TESTADDON_H
#define TESTADDON_H

extern const char *addonTestName;

extern "C" {
	Test *addonTestFunc(void);
	}

#endif