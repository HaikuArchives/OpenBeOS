/* test_mbuf.c 
 * Test the network buffer implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <kernel/OS.h>

#include "mbuf.h"

#define START_BUFFS	5
#define LOOPS	128
#define	MAX_BUFFS	START_BUFFS + LOOPS/16

static int32 mbuf_test_thread(void *data)
{
	int th = *(int*)data;
    struct mbuf *buf[MAX_BUFFS];
    int i, j;

printf("Thread %d starting...\n", th);

        for (j=0;j<LOOPS;j++) {
                for (i=0;i< (START_BUFFS + j%16);i++) {
                        buf[i] = m_get(MT_DATA);
//printf("Thread %d: loop %d, buf %d => %p\n", th, j,i, buf[i]);
                        if (!buf[i]) {
                        		printf("Thread %d, failed on loop %d, buf %d\n",
                        				th, j, i);
                                exit(-1);
                        }
                }
                for (i=0;i<(START_BUFFS + j%16);i++) {
                        m_free(buf[i]);
                }
                if (j%32 == 0)
                        printf("thread %d: %d loops complete\n", th, j);


        }
	printf("thread %d: %d loops complete!\n", th, j);
	return 0; 
}

int main(int argc, char **argv)
{
	int i;
	thread_id thd[3];
	status_t ev;
			
	printf("Network Buffer Test\n");
	printf("===================\n\n");
	
	mbinit();

	for (i=0;i<3;i++) {
		thd[i] = spawn_thread(mbuf_test_thread, "test_mbuf_thread",
				B_NORMAL_PRIORITY, &i);
		resume_thread(thd[i]);
	}

	printf("threads started...\n");
	wait_for_thread(thd[0], &ev);
	wait_for_thread(thd[1], &ev);
	wait_for_thread(thd[2], &ev);
	
printf("dumping freelist!\n");		

	dump_freelist();
	return 0;
}
