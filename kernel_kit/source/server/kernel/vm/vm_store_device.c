/* 
** Copyright 2001, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <kernel/kernel.h>
#include <kernel/vm.h>
#include <kernel/heap.h>
#include <kernel/debug.h>
#include <kernel/lock.h>
#include <kernel/vm_store_device.h>
#include <sys/errors.h>

struct device_store_data {
	addr base_addr;
};

static void device_destroy(struct vm_store *store)
{
	if(store) {
		kfree(store);
	}
}

static off_t device_commit(struct vm_store *store, off_t size)
{
	return 0;
}

static int device_has_page(struct vm_store *store, off_t offset)
{
	// this should never be called
	return 0;
}

static int device_read(struct vm_store *store, off_t offset, void *buf, size_t *len)
{
	panic("device_store: read called. Invalid!\n");
	return ERR_UNIMPLEMENTED;
}

static int device_write(struct vm_store *store, off_t offset, const void *buf, size_t *len)
{
	panic("device_store: write called. Invalid!\n");
	return ERR_UNIMPLEMENTED;
}

// this fault handler should take over the page fault routine and map the page in
//
// setup: the cache that this store is part of has a ref being held and will be
// released after this handler is done
static int device_fault(struct vm_store *store, struct vm_address_space *aspace, off_t offset)
{
	struct device_store_data *d = (struct device_store_data *)store->data;
	vm_cache_ref *cache_ref = store->cache->ref;
	vm_region *region;
	
	dprintf("device_fault: offset 0x%x 0x%x + base_addr 0x%x\n", offset, d->base_addr);
	
	// figure out which page needs to be mapped where
	(*aspace->translation_map.ops->lock)(&aspace->translation_map);
	mutex_lock(&cache_ref->lock);

	// cycle through all of the regions that map this cache and map the page in
	for(region = cache_ref->region_list; region != NULL; region = region->cache_next) {
		// make sure this page in the cache that was faulted on is covered in this region
		if(offset >= region->cache_offset && (offset - region->cache_offset) < region->size) {
			dprintf("device_fault: mapping paddr 0x%x to vaddr 0x%x\n",
				(addr)(d->base_addr + offset),
				(addr)(region->base + (offset - region->cache_offset)));
			(*aspace->translation_map.ops->map)(&aspace->translation_map,
				region->base + (offset - region->cache_offset),
				d->base_addr + offset, region->lock);
		}
	}
	
	mutex_unlock(&cache_ref->lock);	
	(*aspace->translation_map.ops->unlock)(&aspace->translation_map);

	dprintf("device_fault: done\n");

	return 0;
}

static vm_store_ops device_ops = {
	&device_destroy,
	&device_commit,
	&device_has_page,
	&device_read,
	&device_write,
	&device_fault
};

vm_store *vm_store_create_device(addr base_addr)
{
	vm_store *store;
	struct device_store_data *d;
	
	store = kmalloc(sizeof(vm_store) + sizeof(struct device_store_data));
	if(store == NULL)
		return NULL;

	store->ops = &device_ops;
	store->cache = NULL;
	store->data = (void *)((addr)store + sizeof(vm_store));

	d = (struct device_store_data *)store->data;
	d->base_addr = base_addr;

	return store;
}

