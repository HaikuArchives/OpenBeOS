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

/* init the mbuf data structures */
void mbinit(void)
{
        pool_init(&mbpool, 256);
        pool_init(&clpool, MCLBYTES);

}

struct mbuf *m_get(int type)
{
	struct mbuf *mnew;
	MGET(mnew, type);
	return mnew;
}

struct mbuf *m_getclr(int type)
{
	struct mbuf *mnew;
	MGET(mnew, type);
	if (!mnew)
		return NULL;
	memset(mtod(mnew, char *), 0, MLEN);
	return mnew;
}

struct mbuf *m_gethdr(int type)
{
        struct mbuf *mnew;
        MGETHDR(mnew, type);
        return mnew;
}

struct mbuf *m_free(struct mbuf *mfree)
{
	struct mbuf *succ; /* successor if there is one! */
	MFREE(mfree, succ);
	return succ;
}

/* Free the entire chain */
void m_freem(struct mbuf *m)
{
	struct mbuf *n;

	if (!m)
		return;
	do {
		MFREE(m, n);
	} while ((m = n));
}

struct mbuf *m_prepend(struct mbuf *m, size_t len)
{
	struct mbuf *mnew;

	MGET(mnew, m->m_type);
	if (!mnew) {
		/* free chain */
		return NULL;
	}

	if (m->m_flags & M_PKTHDR)
		M_MOVE_PKTHDR(mnew, m);
	mnew->m_next = m;
	m = mnew;
	if (len < MHLEN)
		MH_ALIGN(m, len);
	m->m_len = len;
	return (m);
}


