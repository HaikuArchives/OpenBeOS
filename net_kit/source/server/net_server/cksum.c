#include <stdio.h>

#ifdef _KERNEL_
#include <KernelExport.h>
#endif

#include "net_misc.h"

/* This from Stevens Vol.2 */
#define ADDCARRY(x)	(x > 65535 ? x -= 65535 : x)
#define REDUCE	{l_util.l = sum; sum = l_util.s[0] + l_util.s[1];ADDCARRY(sum);}

uint16 in_cksum(struct mbuf *m, int len, int off)
{
	uint16 *w;
	int sum = 0;
	int mlen = 0;
	int byte_swapped = 0;
	struct mbuf *orig_m = m;

	union {
		uint8	c[2];
		uint16	s;
	} s_util;
	union {
		uint16 s[2];
		uint32	l;
	} l_util;

	if (off) {
		m->m_len -= off;
		m->m_data += off;
		if (m->m_flags & M_PKTHDR)
			m->m_pkthdr.len -= off;
	}

	for (; m && len; m=m->m_next) {
		if (m->m_len == 0)
			continue;
		w = mtod(m, uint16 *);
		if (mlen == -1) {
			/* first byte is a continuation of
			 * a 16 bit word spanning this mbuf
			 * and the previous one.
			 *
			 * s_util.c[0] is already saved.
			 */
			s_util.c[1] = *(char*)w;
			sum += s_util.s;
			w = (uint16*) ((char*) w + 1);
			mlen = m->m_len - 1;
			len--;
		} else
			mlen = m->m_len;
		if (len < mlen)
			mlen = len;
		len -= mlen;
		/* force to even boundry */
		if ((1 & (int)w) && (mlen > 0)) {
			REDUCE;
			sum <<= 8;
			s_util.c[0] = *(char*)w;
			w = (uint16*)((char*)w + 1);
			mlen--;
			byte_swapped = 1;
		}
		/* unroll the loop to make overhead from branches
		 * &c small.
		 */
		while ((mlen -= 32) >= 0) {
			sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];
                        sum += w[4]; sum += w[5]; sum += w[6]; sum += w[7];
                        sum += w[8]; sum += w[9]; sum += w[10]; sum += w[11];
                        sum += w[12]; sum += w[13]; sum += w[14]; sum += w[15];

			w += 16;
		}	
		mlen += 32;
 		while ((mlen -= 8) >= 0) {
                        sum += w[0]; sum += w[1]; sum += w[2]; sum += w[3];

			w += 4;
		}
		mlen += 8;
		if (mlen == 0 && byte_swapped == 0)
			continue;
		REDUCE;
		while ((mlen -= 2) >= 0) {
			sum += *w++;
		}
		if (byte_swapped) {
			REDUCE;
			sum <<= 8;
			byte_swapped = 0;
			if (mlen == -1) {
				s_util.c[1] = *(char*)w;
				sum += s_util.s;
				mlen = 0;
			} else 
				mlen = -1;
		} else if (mlen == -1)
			s_util.c[0] = *(char*)w;
	}
	if (len)
		printf("cksum: out of data!\n");
	if (mlen == -1) {
		/* last mbuf was an odd number of bytes! */
		s_util.c[1] = 0;
		sum += s_util.s;
	}
	REDUCE;

	if (off) {
		orig_m->m_len += off;
		orig_m->m_data -= off;
		if (orig_m->m_flags & M_PKTHDR)
			orig_m->m_pkthdr.len += off;
	}	
	return (uint16)(~sum & 0xffff);	
}

