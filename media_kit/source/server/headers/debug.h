
#include <stdio.h>

#ifndef NDEBUG

	#define UNIMPLEMENTED() \
		printf("UNIMPLEMENTED %s\n",__PRETTY_FUNCTION__)

	#define BROKEN() \
		printf("BROKEN %s\n",__PRETTY_FUNCTION__)

	#define CALLED() \
		((void)0)
//		printf("CALLED %s\n",__PRETTY_FUNCTION__)

	#undef TRACE		
	#define TRACE \
		printf

#else

	#define UNIMPLEMENTED() 	((void)0)
	#define BROKEN()			((void)0)
	#define CALLED()			((void)0)
	#define TRACE \
		if (1) {} else printf

#endif
