/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <ktypes.h>
#include <endian.h>

#undef ntohs

uint16 ntohs(uint16 x)
{
#if BYTE_ORDER == LITTLE_ENDIAN
	u_char *s = (u_char *) &x;
	return (uint16)(s[0] << 8 | s[1]);
#else
	return x;
#endif
}
