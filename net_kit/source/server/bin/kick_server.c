#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>

int main(int argc, char **argv)
{
	int fd = open("/dev/net/socket", O_RDONLY);
	int rv;

	printf("fd = %d\n", fd);
	if (fd < 0)
		printf("Error was %d [%s]\n", fd, strerror(errno));

	rv = close(fd);
	printf("Close gave %d\n", rv);
	
	return 0;
}

