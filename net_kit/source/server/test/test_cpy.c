#include <stdio.h>
#include <strings.h>
#include <kernel/OS.h>

#define BUFSIZE		1024
#define LOOPS		10

#define move4(a,b) { \
	uint8 *aa = (uint8*)a; \
	uint8 *bb = (uint8*)b; \
	aa[0] = bb[0]; aa[1] = bb[1]; aa[2] = bb[2]; aa[3] = bb[3]; \
	}

int main(int argc, char **argv)
{
	char buffera[BUFSIZE];
	char bufferb[BUFSIZE];

	char *ptra;
	char *ptrb;
	int i, j;
	bigtime_t tm;
	bigtime_t t1, t2;

	memset(buffera, 0xff, BUFSIZE);

	/* first off memcpy */
	tm = real_time_clock_usecs();
	for (i=0;i<LOOPS;i++) {
		ptra = (char*)&buffera;
		ptrb = (char*)&bufferb;
		for (j=0;j<BUFSIZE;j+=4) {
			memcpy(ptra, ptrb, 4);
			ptra += 4;
			ptrb += 4;
		}
	}
	t1 = real_time_clock_usecs() - tm;

        tm = real_time_clock_usecs();
        for (i=0;i<LOOPS;i++) {
                ptra = (char*)&buffera;
                ptrb = (char*)&bufferb;
                for (j=0;j<BUFSIZE;j+=4) {
                        move4(ptra, ptrb)
                        ptra += 4;
                        ptrb += 4;
                }
        }
        t2 = real_time_clock_usecs() - tm;

	printf("Results: \n");
	printf("t1 = %lld\n", t1);
	printf("t2 = %lld\n", t2);
	printf("diff (t1 - t2) = %lld\n", t1 - t2);

	return 0;
}

