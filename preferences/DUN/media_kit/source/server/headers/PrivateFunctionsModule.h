#ifndef PRIVATE_FUNCTION_MODULE
#define PRIVATE_FUNCTION_MODULE

#include <String.h>

namespace Private
	{
	bool StringFitsRule( char *Input, char *Rule);
	int OKBox (const char *Output);
	int YesNoBox (const char *Output);
	int CopyLine( char *source, char *target, int limit = 1000);
	BString IntegerToString(int what);
	};

#endif