/* pools.c */

#include <stdio.h>
#include <stdlib.h>
#include "pools.h"

void pool_debug_walk(struct pool_ctl *p)
{
	char *ptr = p->freelist;
	int i = 1;	
	
	printf("%ld byte blocks allocated, but now free:\n\n", p->alloc_size);
	
	while (ptr) {
		printf("  %02d: %p\n", i++, ptr);
		ptr = ((struct free_blk*)ptr)->next;
	}
}

static struct pool_mem *get_mem_block(size_t size, struct pool_mem *nxt)
{
	struct pool_mem *blk = malloc(sizeof(struct pool_mem));
	blk->aid = create_area("net_stack_pools_block", (void**)&blk->base_addr,
						B_ANY_ADDRESS, size, B_LAZY_LOCK, 
						B_READ_AREA|B_WRITE_AREA);

	if (!blk->aid)
		return NULL;
	
	blk->mem_size = blk->avail = size;
	blk->ptr = blk->base_addr;
	if (nxt)
		blk->next = nxt;
	
	return blk;
}

void pool_init(struct pool_ctl **p, size_t sz)
{
	struct pool_ctl *pnew = malloc(sizeof(struct pool_ctl));
	size_t alloc_sz = B_PAGE_SIZE;
		
	if (!pnew)
		return;	

	/* minimum block size is 4 */
	if (sz < 4)
		return;

	/* normally we allocate 4096 bytes... */
	if (sz > alloc_sz)
/* XXX - needs to be page sized */
			alloc_sz = sz;
			
	/* now add a first block */
	pnew->list = get_mem_block(alloc_sz, NULL);
	if (!pnew->list)
		return;
	
	pnew->freelist = pnew->tail = NULL;
	pnew->alloc_size = sz;
	(*p) = pnew;
}

char *pool_get(struct pool_ctl *p)
{
	/* ok, so now we look for a suitable block... */
	struct pool_mem *mp = p->list;
	char *rv = NULL;

	if (p->freelist) {
		/* woohoo, just grab a block! */
		rv = p->freelist;
		p->freelist = ((struct free_blk*)rv)->next;
		return rv;
	}
	
	/* no free blocks, try to allocate of the top of the memory blocks */
	do {
		if (mp->avail >= p->alloc_size) {
			rv = mp->ptr;
			mp->ptr += p->alloc_size;
			mp->avail -= p->alloc_size;
			break;
		}
	} while ((mp = mp->next) != NULL);

/* XXX - we should allocate more memory if we get here with rv == NULL */

	return rv;
}

void pool_put(struct pool_ctl *p, void *ptr)
{
	if (p->freelist) {
		((struct free_blk*)ptr)->next = p->freelist;
	}
	p->freelist = ptr;
}

void pool_destroy(struct pool_ctl *p)
{
	struct pool_mem *mp = p->list;
	struct pool_mem *temp;
	
	do {
		delete_area(mp->aid);
		temp = mp;
		mp = mp->next;
		free(temp);
	} while (mp);
	free(p);
}
