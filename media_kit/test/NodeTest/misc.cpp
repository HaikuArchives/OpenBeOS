#include <string.h>
#include "misc.h"

void val(void *p)
{
	if (p)
		out("OK\n");
	else 
		out("failed\n");
}

void val(status_t status)
{
	if (status == B_OK)
		out("OK\n");
	else 
		out("failed, 0x%08x, %s\n",status,strerror(status));
}

void wait()
{
	out("press enter to continue\n");
	getchar();
}

void out(const char *format,...)
{
	va_list ap;
	va_start(ap,format);
	vfprintf(stdout,format,ap);
	va_end(ap);
	fflush(0);
}
