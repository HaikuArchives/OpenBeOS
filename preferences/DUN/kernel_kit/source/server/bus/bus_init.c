/* 
** Copyright 2001, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <kernel/kernel.h>
#include <kernel/heap.h>
#include <kernel/debug.h>
#include <bus/bus.h>

#ifdef ARCH_i386
#include <bus/pci/pci_bus.h>
#endif
#ifdef ARCH_sh4
#endif

int bus_init(kernel_args *ka)
{
	bus_man_init(ka);

#ifdef ARCH_i386
	pci_bus_init(ka);
#endif	

#ifdef ARCH_sh4
#endif

#if 0
	{
		id_list *vendor_ids;
		id_list *device_ids;
		device dev;
		int rc;
		
		vendor_ids = kmalloc(sizeof(id_list) + sizeof(uint32));
		vendor_ids->num_ids = 1;
		vendor_ids->id[0] = 0x10ec;
		
		device_ids = kmalloc(sizeof(id_list) + sizeof(uint32));
		device_ids->num_ids = 1;
		device_ids->id[0] = 0x8139;

		rc = bus_find_device(1, vendor_ids, device_ids, &dev);
		if(rc >= 0) {
			dprintf("found device : v 0x%x, d 0x%x, irq 0x%x\n", dev.vendor_id, dev.device_id, dev.irq);
		}
		kfree(vendor_ids);
		kfree(device_ids);
	}
#endif

	return 0;
}
