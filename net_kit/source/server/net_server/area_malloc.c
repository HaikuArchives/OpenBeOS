/* Code from Marcus Overhagen <marcus@overhagen.de>
 *
 * This will abort on a buffer overrun and warn on a buffer underrun.
 */

#include <string.h>
#include "net_malloc.h"

#ifdef _KERNEL_MODE
#include <KernelExport.h>
int32 where = B_ANY_KERNEL_ADDRESS;
#else
#include <stdio.h>
#include <kernel/OS.h>
int32 where = B_ANY_ADDRESS;

void panic(char*p)
{
	printf("%s\n",p);
	fflush(0);
	exit(-1); //commented out to see all tests 
}
#endif

	
void *dbg_malloc(char *file, int line, size_t size)
{
	static char *startaddr = 0;
	char *adr;
	size_t realsize;
	char name[64];
	area_id id;
	char *p;

	p = strrchr(file, '/');
	p = (p == NULL) ? file : p + 1;
	sprintf(name, "%s, %d",p,line);

	printf("MALLOC: %s: %d bytes requested\n", name, size);
			
	//aquire lock sem here

	if (startaddr == 0) {
		//find a start address that is unused
		area_id id1;
		area_id id2;
		void *adr1;
		size_t size1 = 4000 * B_PAGE_SIZE; // about 16 MB
		size_t size2 = 30000 * B_PAGE_SIZE; // about 120 MB
		id1 = create_area("", (void **)&adr1, where,
			size1, B_NO_LOCK, B_READ_AREA | B_WRITE_AREA);
		id2 = create_area("", (void **)&startaddr, where,
			size2, B_NO_LOCK, B_READ_AREA | B_WRITE_AREA);
		if (id1 < 0 || id2 < 0)
			panic("out of memory in init code");
		delete_area(id1); 
		delete_area(id2);
	}

	size += 4;

	realsize = (size + B_PAGE_SIZE - 1) & ~(B_PAGE_SIZE - 1);
	adr = startaddr;

	// next start address is one unmapped page away
	startaddr += realsize + B_PAGE_SIZE;

	id = create_area(name, (void **)&adr, B_EXACT_ADDRESS,
		realsize, B_NO_LOCK, B_READ_AREA | B_WRITE_AREA);
	if (id < 0)
		panic("out of memory or address space");

	// align it to make the first byte after (adr + size) to be in an unallocated page
#ifdef CHECK_OVERRUN
	adr += realsize - size;
	*(uint32*)adr = 0xDEADC0DE;
	adr += 4;
#endif

	// release lock sem

	return adr;
}

void dbg_free(char *file, int line, void *ptr)
{
	char *p = (char *)ptr;
	area_id id;
	char text[64];
	char *t;
	
#ifdef CHECK_OVERRUN
	p -= 4;
#endif

	t = strrchr(file, '/');
	t = (t == 0) ? file : t + 1;
	sprintf(text,"%s, %d buffer underrun",t,line);

	printf("FREE: %s: free'ing %p\n", text, ptr);
	
#ifdef CHECK_OVERRUN
	if (*(uint32*)p != 0xDEADC0DE)
		panic(text);
#endif

	id = area_for(p);
	if (id < 0)
		panic(text);
	
	delete_area(id);
}

#ifdef MEMORY_TEST
int main()
{
	char *p;

	printf("!1\n");
	p = (char *)dbg_malloc(1);
	dbg_free(p);

	printf("!2\n");
	p = (char *)dbg_malloc(1);
	p[0] = 0;
	dbg_free(p);

	printf("!3\n");
	p = (char *)dbg_malloc(1);
	p[-1] = 0;
	dbg_free(p);

	printf("!4\n");
	p = (char *)dbg_malloc(1);
	p[1] = 0;
	dbg_free(p);

	return 0;
}
#endif /* MEMORY_TEST */ 


