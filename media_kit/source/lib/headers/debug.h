
#include <stdio.h>

#ifndef NDEBUG

	#define UNIMPLEMENTED() \
		printf("libmedia.so: UNIMPLEMENTED %s\n",__PRETTY_FUNCTION__)

	#define BROKEN() \
		printf("libmedia.so: BROKEN %s\n",__PRETTY_FUNCTION__)

	#define CALLED() \
		printf("libmedia.so: CALLED %s\n",__PRETTY_FUNCTION__)
		
	#define TRACE \
		printf

#else

	#define UNIMPLEMENTED() 	((void)0)
	#define BROKEN()			((void)0)
	#define CALLED()			((void)0)
	#define TRACE \
		if (1) {} else printf

#endif
