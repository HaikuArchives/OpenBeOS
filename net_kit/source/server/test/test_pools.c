/* test_pools.c */

#include <kernel/OS.h>
#include <stdio.h>
#include <malloc.h>

#include "pools.h"

#define SIZE			200
#define INNER_LOOPS 	1000
#define LOOPS			50
#define PUDDLE_SIZE 	256

#define PRINT_SINGLE	0


int main(int argc, char **argv)
{
	char *ptrs[INNER_LOOPS];
	int i,j;
	bigtime_t tm, intv;
	bigtime_t malloc_total = 0,free_total = 0;
	bigtime_t get_total = 0,put_total = 0;

	pool_ctl *pool;
	pool_init(&pool, PUDDLE_SIZE);

	for (j = 0;j < LOOPS;j++) {
		tm = real_time_clock_usecs();
		for (i = 0;i < INNER_LOOPS;i++) {
			ptrs[i] = malloc(PUDDLE_SIZE);
		}
		intv = real_time_clock_usecs()- tm;
		#if PRINT_SINGLE
			printf("malloc took %lld\n", intv);
		#endif
		malloc_total += intv;

		tm = real_time_clock_usecs();
		for (i = 0;i < INNER_LOOPS;i++) {
			free(ptrs[i]);
		}
		intv = real_time_clock_usecs() - tm;
		#if PRINT_SINGLE
			printf("free took %lld\n", intv);
		#endif
		free_total += intv;
	}

	printf("For %d loops malloc()/free() took %lld\n", LOOPS, malloc_total + free_total);
	printf("\tmalloc() took %lld\n", malloc_total);
	printf("\tfree() took %lld\n", free_total);

	for (j = 0;j < LOOPS;j++) {
		tm = real_time_clock_usecs();
		for (i = 0;i < INNER_LOOPS;i++) {
			ptrs[i] = pool_get(pool);
		}
		intv = real_time_clock_usecs() - tm;
		#if PRINT_SINGLE
			printf("pool_get took %lld\n", intv);
		#endif
		get_total += intv;

		tm = real_time_clock_usecs();
		for (i = 0;i < INNER_LOOPS;i++) {
			pool_put(pool, ptrs[i]);
		}
		intv = real_time_clock_usecs() - tm;
		#if PRINT_SINGLE
			printf("pool_put took %lld\n", intv);
		#endif
		put_total += intv;
	}
	printf("For %d loops pool_get()/pool_put() took %lld (%Ld%%)\n", LOOPS,
			get_total + put_total, 100*(get_total + put_total)/(malloc_total + free_total));
	printf("\tpool_get() took %lld (%Ld%%)\n", get_total, 100*get_total/malloc_total);
	printf("\tpool_put() took %lld (%Ld%%)\n", put_total, 100*put_total/free_total);

	return 0;
}

