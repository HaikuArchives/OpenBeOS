/* test_pools.c */

#include <kernel/OS.h>
#include <stdio.h>
#include <malloc.h>

#include "pools.h"

#define SIZE	200
#define LOOPS	5

int main(int argc, char **argv)
{
	char *ptrs[100];
	int i,j;
	bigtime_t tm, intv, cum = 0;
	pool_ctl *p;

	pool_init(&p, 20);

	for (j=0;j<LOOPS;j++) {
		tm=real_time_clock_usecs();
		for (i=0;i<100;i++) {
			ptrs[i] = malloc(20);
		}
		intv = real_time_clock_usecs()- tm;
		printf("malloc took %lld\n", intv);
		cum += intv;

        	tm=real_time_clock_usecs();
        	for (i=0;i<100;i++) {
                	free(ptrs[i]);
        	}
        	intv = real_time_clock_usecs() - tm;
        	printf("free took %lld\n", intv);
		cum += intv;
	}

	printf("For %d loops, took %lld\n", LOOPS, cum);
	cum = 0;

	for (j=0;j<LOOPS;j++) {
		tm=real_time_clock_usecs();
		for (i=0;i<100;i++) {
			ptrs[i] = pool_get(p);
		}
		intv = real_time_clock_usecs() - tm;
		printf("pool_get took %lld\n", intv);
		cum+= intv;

		tm=real_time_clock_usecs();
		for (i=0;i<100;i++) {
			pool_put(p, ptrs[i]);
		}
		intv = real_time_clock_usecs() - tm;
		printf("pool_put took %lld\n", intv);
	}
	printf("For %d loops, took %lld\n", LOOPS, cum);

	return 0;
}

