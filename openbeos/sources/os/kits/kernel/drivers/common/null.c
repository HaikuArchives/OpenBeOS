/*
** Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <kernel.h>
#include <stage2.h>
#include <memheap.h>
#include <devfs.h>
#include <Errors.h>
#include <null.h>

static int null_open(const char *name, uint32 flags, void * *cookie)
{
	*cookie = NULL;
	return 0;
}

static int null_close(void * cookie)
{
	return 0;
}

static int null_freecookie(void * cookie)
{
	return 0;
}

static int null_seek(void * cookie, off_t pos, int st)
{
	return EPERM;
}

static int null_ioctl(void * cookie, uint32 op, void *buf, size_t len)
{
	return EPERM;
}

static ssize_t null_read(void * cookie, off_t pos, void *buf, size_t *len)
{
	return 0;
}

static ssize_t null_write(void * cookie, off_t pos, const void *buf, size_t *len)
{
	return 0;
}

device_hooks null_hooks = {
	&null_open,
	&null_close,
	&null_freecookie,
	&null_ioctl,
	&null_read,
	&null_write,
	NULL,
	NULL,
//	NULL,
//	NULL
};

int null_dev_init(kernel_args *ka)
{
	// create device node
	devfs_publish_device("null", NULL, &null_hooks);

	return 0;
}

