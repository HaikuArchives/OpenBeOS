/* mbuf.c 
 * network buffer implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <kernel/OS.h>
#include <string.h>

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
	if (!mbpool)
		pool_init(&mbpool, 256);
	if (!clpool)
		pool_init(&clpool, MCLBYTES);
        
        /* The max_linkhdr is the biggest link level header 
         * we need to prepend to a packet before sending it.
         *  
         * Here we set it to 14, the size of an ethernet
         * header (2 x 6-byte MAC address + 2-byte type)
         */
	max_linkhdr = 14;
	max_protohdr = 40;
	max_hdr = max_linkhdr + max_protohdr;
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
			struct ifnet *ifp,
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
        m->m_pkthdr.rcvif = ifp;
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

void m_reserve(struct mbuf *mp, int len)
{
	if (mp->m_len == 0) {
		/* empty buffer! */
		if (mp->m_flags & M_PKTHDR) {
			if (len < MHLEN) {
				mp->m_data += len;
				mp->m_len -= len;
				mp->m_pkthdr.len -= len;
				return;
			}
			/* ?? */
		} else {
			if (len <= MLEN) {
				mp->m_data += len;
				mp->m_len -= len;
				return;
			}
		}
	}

	if (len > 0) {
		if (len <= mp->m_len) {
			mp->m_data += len;
			mp->m_len -= len;
		}
	}
	if (mp->m_flags & M_PKTHDR)
		mp->m_pkthdr.len -= len;
}

void m_adj(struct mbuf *mp, int req_len)
{
	struct mbuf *m;
	int len = req_len, count = 0;

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
	} else {
		/* trim from tail... */
		len = -len;
		count = 0;
		for (;;) {
			count += m->m_len;
			if (m->m_next == NULL)
				break;
			m = m->m_next;
		}
		if (m->m_len >= len) {
			m->m_len -= len;
			if (mp->m_flags & M_PKTHDR)
				mp->m_pkthdr.len -= len;
			return;
		}
		count -= len;
		if (count < 0)
			count = 0;
		/* The correct length for the chain is now "count".
		 * find the last mbuf, adjust it's length and toss
		 * remaining mbufs...
		 */
		m = mp; /* first mbuf */
		if (m->m_flags & M_PKTHDR)
			m->m_pkthdr.len = count;
		for (; m; m= m->m_next) {
			if (m->m_len >= count) {
				m->m_len = count;
				break;
			}
			count -= m->m_len;
		}
		while (m->m_next)
			(m = m->m_next)->m_len = 0;
	}
}

void m_copydata(struct mbuf *m, int off, int len, caddr_t cp)
{
        uint count;

        if (off < 0) {
                printf("m_copydata: off %d < 0", off);
		return;
	}
        if (len < 0) {
                printf("m_copydata: len %d < 0", len);
		return;
	}	
        while (off > 0) {
                if (m == NULL) {
                        printf("m_copydata: null mbuf in skip");
			return;
		}
                if (off < m->m_len)
                        break;
                off -= m->m_len;
                m = m->m_next;
        }
        while (len > 0) {
                if (m == NULL) {
                        printf("m_copydata: null mbuf");
			return;
		}
                count = min(m->m_len - off, len);
                memcpy(cp, mtod(m, caddr_t) + off, count);
                len -= count;
                cp += count;
                off = 0;
                m = m->m_next;
        }
}

struct mbuf *m_copym(struct mbuf *m, int off0, int len)
{
        struct mbuf *n, **np;
        int off = off0;
        struct mbuf *top;
        int copyhdr = 0;

        if (off < 0 || len < 0) {
                printf("m_copym0: off %d, len %d\n", off, len);
		exit(-1);
	}
        if (!off && m->m_flags & M_PKTHDR)
                copyhdr = 1;
        while (off > 0) {
                if (!m)
                        printf("m_copym: null mbuf\n");
                if (off < m->m_len)
                        break;
                off -= m->m_len;
                m = m->m_next;
        }
        np = &top;
        top = 0;
        while (len > 0) {
                if (!m) {
                        if (len != M_COPYALL)
                                printf("m_copym0: m == 0 and not COPYALL\n");
                        break;
                }
                MGET(n, m->m_type);
                *np = n;
                if (!n)
                        goto nospace;
                if (copyhdr) {
                        M_DUP_PKTHDR(n, m);
                        if (len == M_COPYALL)
                                n->m_pkthdr.len -= off0;
                        else
                                n->m_pkthdr.len = len;
                        copyhdr = 0;
                }
                n->m_len = min(len, m->m_len - off);
                if (m->m_flags & M_EXT) {
			/*
                         * we are unsure about the way m was allocated.
                         * copy into multiple MCLBYTES cluster mbufs.
                         */
                        MCLGET(n);
                        n->m_len = 0;
                        n->m_len = M_TRAILINGSPACE(n);
                        n->m_len = min(n->m_len, len);
                        n->m_len = min(n->m_len, m->m_len - off);
                        memcpy(mtod(n, caddr_t), mtod(m, caddr_t) + off,
                               (unsigned)n->m_len);
                } else
                        memcpy(mtod(n, caddr_t), mtod(m, caddr_t)+off,
                            (unsigned)n->m_len);
                if (len != M_COPYALL)
                        len -= n->m_len;
                off += n->m_len;
                if (off == m->m_len) {
                        m = m->m_next;
                        off = 0;
                }
                np = &n->m_next;
        }
        return (top);
nospace:
        m_freem(top);
        return (0);
}
