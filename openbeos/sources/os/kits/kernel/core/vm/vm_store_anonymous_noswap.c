/* 
** Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <kernel.h>
#include <vm.h>
#include <memheap.h>
#include <debug.h>
#include <vm_store_anonymous_noswap.h>
#include <errors.h>

static void anonymous_destroy(struct vm_store *store)
{
	if(store) {
		kfree(store);
	}
}

static off_t anonymous_commit(struct vm_store *store, off_t size)
{
	return 0; // no swap, so we commit no memory
}

static int anonymous_has_page(struct vm_store *store, off_t offset)
{
	return 0;
}

static ssize_t anonymous_read(struct vm_store *store, off_t offset, iovecs *vecs)
{
	panic("anonymous_store: read called. Invalid!\n");
	return ERR_UNIMPLEMENTED;
}

static ssize_t anonymous_write(struct vm_store *store, off_t offset, iovecs *vecs)
{
	// no place to write, this will cause the page daemon to skip this store
	return 0;
}

/*
static int anonymous_fault(struct vm_store *backing_store, struct vm_address_space *aspace, off_t offset)
{
	// unused
}
*/

static vm_store_ops anonymous_ops = {
	&anonymous_destroy,
	&anonymous_commit,
	&anonymous_has_page,
	&anonymous_read,
	&anonymous_write,
	NULL, // fault() is unused
	NULL,
	NULL
};

vm_store *vm_store_create_anonymous_noswap()
{
	vm_store *store;
	
	store = kmalloc(sizeof(vm_store));
	if(store == NULL)
		return NULL;

	store->ops = &anonymous_ops;
	store->cache = NULL;
	store->data = NULL;
	store->committed_size = 0;

	return store;
}

