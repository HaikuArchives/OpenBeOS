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
	
	mbuf_init();
	
	//dump_freelist();

	for (j=0;j<64;j++) {
		for (i=0;i<16;i++) {
			buf[i] = get_free_mbuf();
			if (!buf[i])
				break;
		}
		for (i=0;i<16;i++) {
			free_mbuf(buf[i]);
		}
		
	}
			
	dump_freelist();
	return 0;
}