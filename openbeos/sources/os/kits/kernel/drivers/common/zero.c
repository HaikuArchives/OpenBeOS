/*
** Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <kernel.h>
#include <stage2.h>
#include <memheap.h>
#include <devfs.h>
#include <vm.h>
#include <zero.h>
#include <string.h>
#include <errors.h>

static int zero_open(const char *name, uint32 flags, void **cookie)
{
	*cookie = NULL;
	return 0;
}

static int zero_close(void * cookie)
{
	return 0;
}

static int zero_freecookie(void * cookie)
{
	return 0;
}

static int zero_seek(void * cookie, off_t pos, seek_type st)
{
	return ERR_NOT_ALLOWED;
}

static int zero_ioctl(void * cookie, uint32 op, void *buf, size_t len)
{
	return ERR_NOT_ALLOWED;
}

static ssize_t zero_read(void * cookie, off_t pos, void *buf, size_t *len)
{
	int rc;

	rc = user_memset(buf, 0, *len);
	if(rc < 0)
		return rc;

	return 0;
}

static ssize_t zero_write(void * cookie, off_t pos, const void *buf, size_t *len)
{
	return 0;
}

device_hooks zero_hooks = {
	&zero_open,
	&zero_close,
	&zero_freecookie,
	&zero_ioctl,
	&zero_read,
	&zero_write,
	NULL,
	NULL,
//	NULL,
//	NULL
};

int zero_dev_init(kernel_args *ka)
{
	// create device node
	devfs_publish_device("zero", NULL, &zero_hooks);

	return 0;
}
