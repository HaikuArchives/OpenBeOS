/* mbuf.c 
 * network buffer implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <kernel/OS.h>

#include "mbuf.h"

static struct mbuf *freelist;

#define MBUF_ALLOCSIZE		4096

static void _add_free_mbufs(void)
{
	char *base_addr;
	struct mbuf *bufs;
	int i = 0;
	uint32 size = MBUF_ALLOCSIZE;
	int qty = size / sizeof(struct mbuf);
	
	area_id aid = create_area("mbuf_area", (void**)&base_addr, B_ANY_ADDRESS,
							size,
							B_LAZY_LOCK, B_READ_AREA|B_WRITE_AREA);
	
	if (aid < B_OK)
		return;
	
	//printf("mbuf area created - %p!\n", base_addr);
	bufs = (struct mbuf*)base_addr;

	for (i = 0;i < qty;i++) {
		_MFILL(bufs, MT_FREE);
		if (freelist)
			bufs->m_next = freelist;
		freelist = bufs;
		bufs++;
	}
	//printf("added %d bufs to free list.\n", qty);
}		

struct mbuf *get_free_mbuf(void)
{
	struct mbuf *ret = freelist;
	
	if (!ret)
		return NULL;
		
	freelist = ret->m_next;
	ret->m_next = NULL;
	
	return ret;
}

void free_mbuf(struct mbuf *b)
{
	if (b) {
/* XXX this needs to be finished! */
		if (b->m_flags & M_EXT) {
			if (b->m_flags & M_CLUSTER) {
				//free_cluster(b->m_ext.ext_buf);
				b->m_flags |= M_CLUSTER | M_EXT;
			}
			/* other forms of cluster need to be removed/freed here */
		}
		
		_MFILL(b, MT_FREE);
		if (freelist)
			b->m_next = freelist;
		freelist = b;
	}
}

void dump_freelist(void)
{
	struct mbuf *b = freelist;
	
	do {
		printf("	%p next @ %p\n", b, b->m_next);
	} while ((b = b->m_next) != NULL);
}
		
/* init the mbuf data structures */		
void mbinit(void)
{
	freelist = NULL;

	_add_free_mbufs();	

/*
	printf("mbuf is %ld bytes, cluster is %d bytes\n", 
			sizeof(struct mbuf),
			MCLBYTES);
*/
}
