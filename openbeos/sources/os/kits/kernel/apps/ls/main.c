/*
** Copyright 2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <syscalls.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

extern char *__progname;

void (*disp_func)(const char *, struct file_stat *) = NULL;

static void display_l(const char *filename, struct file_stat *stat)
{
	const char *type;

	switch(stat->type) {
		case STREAM_TYPE_FILE:
			type = "FILE";
			break;
		case STREAM_TYPE_DEVICE:
			type = "DEV ";
			break;
		case STREAM_TYPE_DIR:
			type = "DIR ";
			break;
		default:
			type = "UNKN";
	}

	printf("%s %12Ld %s\n", type, stat->size, filename);
}

static void display(const char *filename, struct file_stat *stat)
{
	printf("%s\n", filename);
}

int main(int argc, char *argv[])
{
	int rc;
	int rc2;
	int count = 0;
	struct file_stat stat;
	char *arg;
	int ch;
	
	disp_func = display;
	
	while ((ch = getopt(argc, argv, "l")) != -1) {
		switch (ch) {
			case 'l':
				disp_func = display_l;
		}
	}
	argc -= optind;
	argv += optind;

	if(*argv == NULL) {
		arg = ".";
	} else {
		arg = *argv;
	}
	
	rc = sys_rstat(arg, &stat);
	if(rc < 0) {
		printf("%s: %s: %s\n", __progname,
		       arg, strerror(rc));
		goto err_ls;
	}

	switch(stat.type) {
		case STREAM_TYPE_DIR: {
			int fd;
			char buf[1024];

			fd = sys_open(arg, STREAM_TYPE_DIR, 0);
			if(fd < 0) {
				printf("%s: %s: %s\n", 
				       __progname, arg, strerror(fd));
				break;
			}

			for(;;) {
				char buf2[1024];

				rc = sys_read(fd, buf, -1, sizeof(buf));
				if(rc <= 0)
					break;

				buf2[0] = 0;
				if(strcmp(arg, ".") != 0) {
					strlcpy(buf2, arg, sizeof(buf2));
					strlcat(buf2, "/", sizeof(buf2));
				}
				strlcat(buf2, buf, sizeof(buf2));

				rc2 = sys_rstat(buf2, &stat);
				if(rc2 >= 0) {
					(*disp_func)(buf2, &stat);
				}
				count++;
			}
			sys_close(fd);

			printf("%d files found\n", count);
			break;
		}
		default:
			(*disp_func)(argv[1], &stat);
			break;
	}

err_ls:
	return 0;
}
