/* pools.c */

#include <stdio.h>
#include <stdlib.h>
#include "pools.h"
#include "net_misc.h"

#define ROUND_TO_PAGE_SIZE(x) (((x) + (B_PAGE_SIZE) - 1) & ~((B_PAGE_SIZE) - 1))


void pool_debug_walk(struct pool_ctl *p)
{
	char *ptr;
	int i = 1;	
	
	printf("%ld byte blocks allocated, but now free:\n\n", p->alloc_size);

	#if POOL_USES_BENAPHORES
		ACQUIRE_BENAPHORE(p->lock);
	#else
		ACQUIRE_READ_LOCK(p->lock);
	#endif
	ptr = p->freelist;	
	while (ptr) {
		printf("  %02d: %p\n", i++, ptr);
		ptr = ((struct free_blk*)ptr)->next;
	}
	#if POOL_USES_BENAPHORES
		RELEASE_BENAPHORE(p->lock);
	#else
		RELEASE_READ_LOCK(p->lock);
	#endif
}


static struct pool_mem *get_mem_block(struct pool_ctl *pool)
{
	struct pool_mem *block = malloc(sizeof(struct pool_mem));
	int32 where; /* where should new block be allocated */
	int32 how; /* what type of memory do we want */
	if (block == NULL)
		return NULL;

#ifndef _KERNEL_MODE
	where = B_ANY_ADDRESS;
	how = B_LAZY_LOCK|B_CONTIGUOUS;
#else
	where = B_ANY_KERNEL_ADDRESS;
	how = B_FULL_LOCK|B_CONTIGUOUS;
#endif
	
	block->aid = create_area("net_stack_pools_block",
						(void**)&block->base_addr,
						where, pool->block_size, how, 
						B_READ_AREA|B_WRITE_AREA);
	if (block->aid < B_OK) {
		free(block);
		return NULL;
	}

	block->mem_size = block->avail = pool->block_size;
	block->ptr = block->base_addr;
	INIT_BENAPHORE(block->lock, "pool_mem_lock");

	if (CHECK_BENAPHORE(block->lock) >= B_OK) {
		#if POOL_USES_BENAPHORES
			ACQUIRE_BENAPHORE(pool->lock);
		#else
			ACQUIRE_WRITE_LOCK(pool->lock);
		#endif

		// insert block at the beginning of the pools
		if (pool->list)
			block->next = pool->list;

		pool->list = block;
		
		#if POOL_USES_BENAPHORES
			RELEASE_BENAPHORE(pool->lock);
		#else
			RELEASE_WRITE_LOCK(pool->lock);
		#endif

		return block;
	}
	UNINIT_BENAPHORE(block->lock);
	
	delete_area(block->aid);
	free(block);

	return NULL;
}


status_t pool_init(struct pool_ctl **_newPool, size_t size)
{
	struct pool_ctl *pool;

	/* minimum block size is sizeof the free_blk structure */
	if (size < sizeof(struct free_blk)) 
		return B_BAD_VALUE;

	pool = malloc(sizeof(struct pool_ctl));
	if (pool == NULL)
		return B_NO_MEMORY;

	#if POOL_USES_BENAPHORES
		INIT_BENAPHORE(pool->lock, "pool_lock");
		if (CHECK_BENAPHORE(pool->lock) < B_OK) {
			free(pool);
			return B_ERROR;
		}
	#else
		INIT_RW_LOCK(pool->lock, "pool_lock");
		if (CHECK_RW_LOCK(pool->lock) < B_OK) {
			free(pool);
			return B_ERROR;
		}
	#endif

	// 4 puddles will always fit in one pool
	pool->block_size = ROUND_TO_PAGE_SIZE(size * 8);
	pool->alloc_size = size;
	pool->list = NULL;
	pool->freelist = NULL;

	/* now add a first block */
	get_mem_block(pool);
	if (!pool->list) {
		#if POOL_USES_BENAPHORES
			UNINIT_BENAPHORE(pool->lock);
		#else
			UNINIT_RW_LOCK(pool->lock);
		#endif
		free(pool);
		return B_NO_MEMORY;
	}

	*_newPool = pool;
	
	return B_OK;
}


char *pool_get(struct pool_ctl *p)
{
	/* ok, so now we look for a suitable block... */
	struct pool_mem *mp = p->list;
	char *rv = NULL;

	#if POOL_USES_BENAPHORES
		ACQUIRE_BENAPHORE(p->lock);
	#else
		ACQUIRE_WRITE_LOCK(p->lock);
	#endif

	if (p->freelist) {
		/* woohoo, just grab a block! */
		rv = p->freelist;

		p->freelist = ((struct free_blk*)rv)->next;

		#if POOL_USES_BENAPHORES
			RELEASE_BENAPHORE(p->lock);
		#else
			RELEASE_WRITE_LOCK(p->lock);
		#endif

		return rv;
	}		
	#if !POOL_USES_BENAPHORES
		RELEASE_WRITE_LOCK(p->lock);
		ACQUIRE_READ_LOCK(p->lock);
	#endif

	/* no free blocks, try to allocate of the top of the memory blocks
	** we must hold the global pool lock while iterating through the list!
	*/

	do {
		ACQUIRE_BENAPHORE(mp->lock);

		if (mp->avail >= p->alloc_size) {
			rv = mp->ptr;
			mp->ptr += p->alloc_size;
			mp->avail -= p->alloc_size;
			RELEASE_BENAPHORE(mp->lock);
			break;
		}
		RELEASE_BENAPHORE(mp->lock);
	} while ((mp = mp->next) != NULL);

	#if POOL_USES_BENAPHORES
		RELEASE_BENAPHORE(p->lock);
	#else
		RELEASE_READ_LOCK(p->lock);
	#endif

	if (rv)
		return rv;

	mp = get_mem_block(p);
	if (mp == NULL)
		return NULL;

	ACQUIRE_BENAPHORE(mp->lock);

	if (mp->avail >= p->alloc_size) {
		rv = mp->ptr;
		mp->ptr += p->alloc_size;
		mp->avail -= p->alloc_size;
	}
	RELEASE_BENAPHORE(mp->lock);

	return rv;
}


void pool_put(struct pool_ctl *p, void *ptr)
{
	#if POOL_USES_BENAPHORES
		ACQUIRE_BENAPHORE(p->lock);
	#else
		ACQUIRE_WRITE_LOCK(p->lock);
	#endif
	if (p->freelist) {
		((struct free_blk*)ptr)->next = p->freelist;
	}
	p->freelist = ptr;
	
	#if POOL_USES_BENAPHORES
		RELEASE_BENAPHORE(p->lock);
	#else
		RELEASE_WRITE_LOCK(p->lock);
	#endif
}


void pool_destroy(struct pool_ctl *p)
{
	struct pool_mem *mp,*temp;

	if (p == NULL)
		return;

	/* the semaphore will be deleted, so we don't have to unlock */
	ACQUIRE_WRITE_LOCK(p->lock);

	mp = p->list;
	while (mp != NULL) {
		delete_area(mp->aid);
		temp = mp;
		mp = mp->next;
		UNINIT_BENAPHORE(mp->lock);
		free(temp);
	}

	#if POOL_USES_BENAPHORES
		UNINIT_BENAPHORE(p->lock);
	#else
		UNINIT_RW_LOCK(p->lock);
	#endif
	free(p);
}
