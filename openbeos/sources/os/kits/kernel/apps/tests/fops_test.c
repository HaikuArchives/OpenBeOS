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

int main(int argc, char **argv)
{
	FILE *f;
	char buffer[100];
	int rv;

	f = fopen(TEST_FILE, "r");
	if (!f) {
		printf("Failed to open %s :( [%s]\n", TEST_FILE,
                       strerror(errno));
		return -1;
	}
	
	rv = fread(buffer, 100, 1, f);
	printf("fread() gave %d\n%s\n", rv, buffer);
	fclose(f);
	
	printf("File opened and closed\n");

	return 0;
}
