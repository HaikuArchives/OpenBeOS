/* Contains code to load dev module and bootstrap it */

/*
** Copyright 2001, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <kernel.h>
#include <stage2.h>
#include <dev.h>
#include <vfs.h>
#include <debug.h>
#include <elf.h>
#include <errors.h>

#include <stdio.h>

int dev_init(kernel_args *ka)
{
	int fd;

	dprintf("dev_init: entry\n");

	fd = sys_open("/boot/addons/dev", STREAM_TYPE_DIR, 0);
	if(fd >= 0) {
		ssize_t len;
		char buf[SYS_MAX_NAME_LEN];

		while((len = sys_read(fd, buf, 0, sizeof(buf))) > 0) {
			dprintf("loading '%s' dev module\n", buf);
			dev_load_dev_module(buf);
		}
		sys_close(fd);
	}

	return 0;
}

image_id dev_load_dev_module(const char *name)
{
	image_id id;
	void (*bootstrap)();
	char path[SYS_MAX_PATH_LEN];

	sprintf(path, "/boot/addons/dev/%s", name);

	id = elf_load_kspace(path, "");
	if(id < 0)
		return id;

	bootstrap = (void *)elf_lookup_symbol(id, "dev_bootstrap");
	if(!bootstrap)
		return ERR_VFS_INVALID_FS;

	bootstrap();

	return id;
}

