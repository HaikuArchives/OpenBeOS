/* socket "server" */

#include <stdio.h>
#include <kernel/OS.h>

#include "sys/socket.h"
#include "sys/socketvar.h"
#include "pools.h"
#include "netinet/in_pcb.h"
#include "net_module.h"
#include "net_misc.h"
#include "protocols.h"
#include "mbuf.h"

uint32 sb_max = SB_MAX; /* hard value, recompile needed to alter :( */


/*
 * Allot mbufs to a sockbuf.
 * Attempt to scale mbmax so that mbcnt doesn't become limiting
 * if buffering efficiency is near the normal case.
 */
int sbreserve(struct sockbuf *sb, uint32 cc)
{
	uint64 dd = (uint64)cc;
	uint64 ee = (sb_max * MCLBYTES) / ((MSIZE) + (MCLBYTES));

        if (cc == 0) 
		return 0;
	if (dd > ee)
//(uint64)((sb_max * MCLBYTES) / (MSIZE + MCLBYTES)))
		return 0;

        sb->sb_hiwat = cc;
        sb->sb_mbmax = min((cc * 2), sb_max);
        if (sb->sb_lowat > sb->sb_hiwat)
                sb->sb_lowat = sb->sb_hiwat;
        return (1);
}

void sbdrop(struct sockbuf *sb, int len)
{
        struct mbuf *m, *mn;
        struct mbuf *next;

        next = (m = sb->sb_mb) ? m->m_nextpkt : 0;
        while (len > 0) {
                if (m == 0) {
                        if (next == 0) {
                                printf("sbdrop");
				exit(-1);
			}
                        m = next;
                        next = m->m_nextpkt;
                        continue;
                }
                if (m->m_len > len) {
                        m->m_len -= len;
                        m->m_data += len;
                        sb->sb_cc -= len;
                        break;
                }
                len -= m->m_len;
                sbfree(sb, m);
                MFREE(m, mn);
                m = mn;
        }
        while (m && m->m_len == 0) {
                sbfree(sb, m);
                MFREE(m, mn);
                m = mn;
        }
        if (m) {
                sb->sb_mb = m;
                m->m_nextpkt = next;
        } else
                sb->sb_mb = next;
}

/*
 * Free all mbufs in a sockbuf.
 * Check that all resources are reclaimed.
 */
void sbflush(struct sockbuf *sb)
{

        if (sb->sb_flags & SB_LOCK) {
                printf("sbflush");
		exit(-1);
	}
        while (sb->sb_mbcnt)
                sbdrop(sb, (int)sb->sb_cc);
        if (sb->sb_cc || sb->sb_mb) {
                printf("sbflush 2\n");
		exit(-1);
	}	
}

/*
 * Free mbufs held by a socket, and reserved mbuf space.
 */
void sbrelease(struct sockbuf *sb)
{

        sbflush(sb);
        sb->sb_hiwat = sb->sb_mbmax = 0;
}

