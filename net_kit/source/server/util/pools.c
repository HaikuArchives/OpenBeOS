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

static void get_mem_block(size_t size, struct pool_ctl *p)
{
	struct pool_mem *blk = malloc(sizeof(struct pool_mem));
	blk->aid = create_area("net_stack_pools_block", (void**)&blk->base_addr,
						B_ANY_ADDRESS, size, B_LAZY_LOCK, 
						B_READ_AREA|B_WRITE_AREA);
	if (!blk->aid)
		return;
	
	blk->mem_size = blk->avail = size;
	blk->ptr = blk->base_addr;
	blk->lock = create_sem(1, "pool_mem_lock");

	if (!blk->ptr || blk->lock < B_OK)
		return;

	acquire_sem(p->lock);
	if (p->list)
		blk->next = p->list;
	p->list = blk;
	release_sem(p->lock);

	return;
}

void pool_init(struct pool_ctl **p, size_t sz)
{
	struct pool_ctl *pnew = malloc(sizeof(struct pool_ctl));
	size_t alloc_sz = B_PAGE_SIZE;
		
	if (!pnew)
		return;	

	/* minimum block size is sizeof the free_blk structure */
	if (sz < sizeof(struct free_blk))
		return;

	/* normally we allocate 4096 bytes... */
	if (sz > alloc_sz)
		alloc_sz = sz;

	pnew->lock = create_sem(1, "pool_lock");
			
	/* now add a first block */
	get_mem_block(alloc_sz, pnew);
	if (!pnew->list)
		return;
	
	pnew->freelist = NULL;
	pnew->alloc_size = sz;

	(*p) = pnew;
}

char *pool_get(struct pool_ctl *p)
{
	/* ok, so now we look for a suitable block... */
	struct pool_mem *mp = p->list;
	char *rv = NULL;

	acquire_sem(p->lock);
	if (p->freelist) {
		/* woohoo, just grab a block! */
		rv = p->freelist;
		p->freelist = ((struct free_blk*)rv)->next;
		release_sem(p->lock);
		return rv;
	}
	release_sem(p->lock);

	/* no free blocks, try to allocate of the top of the memory blocks */
	do {
		acquire_sem(mp->lock);
		if (mp->avail >= p->alloc_size) {
			rv = mp->ptr;
			mp->ptr += p->alloc_size;
			mp->avail -= p->alloc_size;
			release_sem(mp->lock);
			break;
		}
		release_sem(mp->lock);
	} while ((mp = mp->next) != NULL);

	if (rv)
		return rv;

	get_mem_block(B_PAGE_SIZE, p);
	mp = p->list;
	acquire_sem(mp->lock);
	if (mp->avail >= p->alloc_size) {
		rv = mp->ptr;
        	mp->ptr += p->alloc_size;
        	mp->avail -= p->alloc_size;
	}
	release_sem(mp->lock);

	return rv;
}

void pool_put(struct pool_ctl *p, void *ptr)
{
	acquire_sem(p->lock);
	if (p->freelist) {
		((struct free_blk*)ptr)->next = p->freelist;
	}
	p->freelist = ptr;
	release_sem(p->lock);
}

void pool_destroy(struct pool_ctl *p)
{
	struct pool_mem *mp = p->list;
	struct pool_mem *temp;
	
	do {
		delete_area(mp->aid);
		temp = mp;
		mp = mp->next;
		delete_sem(mp->lock);
		free(temp);
	} while (mp);
	delete_sem(p->lock);
	free(p);
}
