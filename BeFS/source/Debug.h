#ifndef DEBUG_H
#define DEBUG_H
/* Debug - debug stuff
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
** This file may be used under the terms of the OpenBeOS License.
*/

#ifdef USER
#	define __out printf
#else
#	define __out dprintf
#endif

#ifdef DEBUG
	#define PRINT(x) __out x;
	#define INFORM(x) __out x;
	#define FUNCTION() __out("%s\n",__PRETTY_FUNCTION__);
	#define FUNCTION_START(x) __out x;
#else
	#define PRINT(x) ;
	#define INFORM(x) __out x;
	#define FUNCTION() ;
	#define FUNCTION_START(x) ;
#endif

#define FATAL(x) __out x;

#endif	/* DEBUG_H */
