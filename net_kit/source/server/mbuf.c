/* mbuf.c 
 * network buffer implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <kernel/OS.h>
#include <string.h>

#define _NET_STACK_

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


struct mbuf *m_devget(char *buf, int totlen, int off0,
			/*struct ifnet *ifp,*/
			void (*copy)(const void *, void *, size_t))
{
        struct mbuf *m;
        struct mbuf *top = NULL, **mp = &top;
        int off = off0, len;
        char *cp;
        char *epkt;

        cp = buf;
        epkt = cp + totlen;
        if (off) {
                /*
                 * If 'off' is non-zero, packet is trailer-encapsulated,
                 * so we have to skip the type and length fields.
                 */
                cp += off + 2 * sizeof(uint16);
                totlen -= 2 * sizeof(uint16);
        }
        MGETHDR(m, MT_DATA);
        if (m == NULL)
                return (NULL);
        //m->m_pkthdr.rcvif = ifp;
        m->m_pkthdr.len = totlen;
        m->m_len = MHLEN;

        while (totlen > 0) {
                if (top != NULL) {
                        MGET(m, MT_DATA);
                        if (m == NULL) {
                                m_freem(top);
                                return (NULL);
                        }
                        m->m_len = MLEN;
                }
                len = min(totlen, epkt - cp);
                if (len >= MINCLSIZE) {
                        MCLGET(m);
                        if (m->m_flags & M_EXT)
                                m->m_len = len = min(len, MCLBYTES);
                        else
                                len = m->m_len;
                } else {
                        /*
                         * Place initial small packet/header at end of mbuf.
                         */
                        if (len < m->m_len) {
                                if (top == NULL &&
                                    len + max_linkhdr <= m->m_len)
                                        m->m_data += max_linkhdr;
                                m->m_len = len;
                        } else
                                len = m->m_len;
                }
                if (copy)
                        copy(cp, mtod(m, void *), (size_t)len);
                else
                        memmove(mtod(m, void *), cp, (size_t)len);
                cp += len;
                *mp = m;
                mp = &m->m_next;
                totlen -= len;
                if (cp == epkt)
                        cp = buf;
        }
        return (top);
}

void m_adj(struct mbuf *mp, int req_len)
{
	struct mbuf *m;
	int len = req_len;

	if ((m = mp) == NULL)
		return;

	if (len >= 0) {
		/* trim from the head */
		while (m!= NULL && len > 0) {
			if (m->m_len <= len) {
				/* this whole mbuf isn't enough... */
				len -= m->m_len;
				m->m_len = 0;
				m = m->m_next;
			} else {
				/* this mbuf just needs trimming */
				m->m_len -= len;
				m->m_data += len;
				len = 0;
			}
		}
		m = mp;
		if (mp->m_flags & M_PKTHDR)
			m->m_pkthdr.len -= (req_len - len);
	}
	/* -ve case not yet implemented */
}

