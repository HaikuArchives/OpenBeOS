#include <OS.h>
#include <stdio.h>

void panic(char*);

void panic(char*p)
{
	printf("%s\n",p);
	fflush(0);
//	exit(-1);
}

void *dbg_malloc(size_t size)
{
	static char *startaddr = 0;
	char *adr;
	size_t realsize;
	area_id id;
	
	//aquire lock sem here
	
	if (startaddr == 0) {
		//find a start address that is unused
		area_id id1;
		area_id id2;
		void *adr1;
		size_t size1 = 4000 * B_PAGE_SIZE; // about 16 MB
		size_t size2 = 30000 * B_PAGE_SIZE; // about 120 MB
		id1 = create_area("",(void **)&adr1,B_ANY_KERNEL_ADDRESS,size1,B_NO_LOCK,B_READ_AREA | B_WRITE_AREA);
		id2 = create_area("",(void **)&startaddr,B_ANY_KERNEL_ADDRESS,size2,B_NO_LOCK,B_READ_AREA | B_WRITE_AREA);
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
	
	id = create_area("memory",(void **)&adr,B_EXACT_ADDRESS,realsize,B_NO_LOCK,B_READ_AREA | B_WRITE_AREA);
	if (id < 0)
		panic("out of memory or address space");
	
	// align it to make the first byte after (adr + size) to be in an unallocated page
	adr += realsize - size;
	
	*(uint32*)adr = 0xDEADC0DE;
	adr += 4;
	
	// release lock sem
	
	return adr;
}

void dbg_free(void *ptr)
{
	char *p = (char *)ptr;
	area_id id;
	
	p -= 4;
	if (*(uint32*)p != 0xDEADC0DE) 
		panic("buffer underrun");

	id = area_for(p);
	if (id < 0)
		panic("no area for buffer");

	delete_area(id);
}

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

}
