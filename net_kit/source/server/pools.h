/* pools.h
 * simple fixed size block allocator
 */

#include <kernel/OS.h>

#ifndef OBOS_POOLS_H
#define OBOS_POOLS_H

struct pool_mem {
	struct pool_mem *next;
	area_id	aid;
	char *base_addr;
	size_t mem_size;
	char *ptr;
	size_t avail;
	sem_id lock;
};

struct free_blk {
	char *next;
};

struct pool_ctl {
	struct pool_mem *list;
	size_t alloc_size;
	char *freelist;
	sem_id lock;
};

void pool_init(struct pool_ctl **p, size_t sz);
char *pool_get(struct pool_ctl *p);
void pool_put(struct pool_ctl *p, void *ptr);
void pool_destroy(struct pool_ctl *p);

void pool_debug_walk(struct pool_ctl *p);

#endif
	
