#ifndef CPP_H
#define CPP_H
/* cpp - C++ in the kernel
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
*/

#include <new>
#include <stdlib.h>


inline void *operator new(size_t size, const nothrow_t&) throw()
{
	return malloc(size);
} 

inline void *operator new[](size_t size, const nothrow_t&) throw()
{
	return malloc(size);
}
 
inline void operator delete(void *ptr)
{
	free(ptr);
} 

inline void operator delete[](void *ptr)
{
	free(ptr);
}

// we're not using virtuals
//extern "C" void pure_virtual();

extern nothrow_t _dontthrow;
#define new new (_dontthrow)


#endif	/* CPP_H */
