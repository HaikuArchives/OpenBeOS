#include <stdio.h>
#include <kernel/OS.h>

#include "nhash.h"
#define MAX_CHAR	0xff
#define CHAR_STEP	16
#define MAX_LENGTH	16
#define MIN_LEN		3
#define LEN_STEP	2

#define ARRAY_SZ	(MAX_CHAR / CHAR_STEP) * \
			((MAX_LENGTH - MIN_LEN + 1) / LEN_STEP)
#define LOOPS 20

#if DEBUG_ON
static void dump_data(void *ptr, int len)
{
	uint8* b = (uint8*)ptr;
	int i;

	for (i=0;i<len;i++) {
		printf(" %02x ", b[i]);
	}
	printf("\n");
}
#endif

int main(int argc, char **argv)
{
	net_hash *h;
	uint8 data[16];
	int32 dp[ARRAY_SZ];
	int i, j, k;
	int32 ptr = 0;
	void * rv;
	bigtime_t timer, cum = 0;

	h = nhash_make();

	for (k = 0; k < LOOPS; k++) {

		timer = real_time_clock_usecs();

		for (i=CHAR_STEP; i < MAX_CHAR + 1; i += CHAR_STEP) {
			for (j=MIN_LEN; j < MAX_LENGTH; j+=LEN_STEP) {
				memset(&data, i, j+1);
				dp[ptr] = (i * j) + 1;
				nhash_set(h, &data, j+1, &dp[ptr++]);
			}
		}
		for (i=CHAR_STEP;i<MAX_CHAR + 1;i += CHAR_STEP) {
			for (j=MIN_LEN;j<MAX_LENGTH;j+= LEN_STEP) {
				memset(&data, i, j+1);
				rv = nhash_get(h, &data, j+1);
				if (rv) {
					ptr = *(int32*)rv;
					if (ptr != (i*j)+1) {
						printf("Failed! %ld instead of %d\n",
							ptr, (i*j)+1);
					}
				}
			}
		}

		timer = real_time_clock_usecs() - timer;

                for (i=CHAR_STEP; i < MAX_CHAR + 1; i += CHAR_STEP) {
                        for (j=MIN_LEN; j < MAX_LENGTH; j+=LEN_STEP) {
                                memset(&data, i, j+1);
				nhash_set(h, &data, j+1, NULL);
			}
		}
		cum += timer;
	}
	printf("%d loops took %lld, average = %lld\n", LOOPS, cum, cum / LOOPS);
	return 0;
}

