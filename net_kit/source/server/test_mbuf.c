/* test_mbuf.c 
 * Test the network buffer implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <kernel/OS.h>

#include "mbuf.h"

int main(int argc, char **argv)
{
	struct mbuf *buf[16];
	int i, j;
		
	printf("Network Buffer Test\n");
	printf("===================\n\n");
	
	mbinit();
	
	//dump_freelist();

	for (j=0;j<64;j++) {
		for (i=0;i<16;i++) {
			buf[i] = m_get(MT_DATA);
			if (!buf[i])
				break;
		}
		for (i=0;i<16;i++) {
			m_free(buf[i]);
		}
		
	}
			
	dump_freelist();
	return 0;
}