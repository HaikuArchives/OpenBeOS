/* mbuf.c 
 * network buffer implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <kernel/OS.h>

#include "mbuf.h"
#include "pools.h"

#define MBUF_ALLOCSIZE		4096

void dump_freelist(void)
{
	pool_debug_walk(mbpool);
}

struct mbuf *m_get(int type)
{
	struct mbuf *mnew;
	MGET(mnew, type);
	return mnew;
}

struct mbuf *m_free(struct mbuf *mfree)
{
	struct mbuf *succ; /* successor if there is one! */
	MFREE(mfree, succ);
	return succ;
}
	
/* init the mbuf data structures */		
void mbinit(void)
{
	pool_init(&mbpool, 256);
	pool_init(&clpool, MCLBYTES);

/*
	printf("mbuf is %ld bytes, cluster is %d bytes\n", 
			sizeof(struct mbuf),
			MCLBYTES);
*/
}
