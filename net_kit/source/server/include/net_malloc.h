/* net_malloc.h
 *
 * Are we useing the system amlloc or our own...
 */
 
#ifndef NET_MALLOC_H
#define NET_MALLOC_H

#include <malloc.h>
#include "net_misc.h"

#if USE_DEBUG_MALLOC

/* use the area malloc code from marcus Overhagen */
void *dbg_malloc(char *file, int line, size_t size);
void  dbg_free  (char *file, int line, void *ptr);

#define malloc(s) dbg_malloc(__FILE__, __LINE__, s)
#define free(s)   do { dbg_free(__FILE__, __LINE__, s); s = NULL; } while (0)
#endif /* USE_DEBUG_MALLOC */

#endif /* NET_MALLOC_H */

