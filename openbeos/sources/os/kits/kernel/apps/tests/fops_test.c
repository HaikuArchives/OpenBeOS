#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <syscalls.h>
#include <ktypes.h>
#include <resource.h>
#include <Errors.h>
#include <OS.h>
#include <stdlib.h>

#define TEST_FILE "/boot/etc/fortunes"
#define RD_BUFFER 100

int main(int argc, char **argv)
{
	FILE *f;
	char buffer[RD_BUFFER];
	int rv;

	f = fopen(TEST_FILE, "r");
	if (!f) {
		printf("Failed to open %s :( [%s]\n", TEST_FILE,
                       strerror(errno));
		return -1;
	}

	memset(buffer, 0, RD_BUFFER);	
	rv = fread(buffer, RD_BUFFER, 1, f);
	printf("fread() returned %d elements\n%s\n", rv, buffer);
	fclose(f);
	
	printf("File opened and closed\n");

	return 0;
}
