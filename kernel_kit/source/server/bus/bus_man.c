/* 
** Copyright 2001, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <kernel/kernel.h>
#include <kernel/lock.h>
#include <kernel/debug.h>
#include <kernel/heap.h>
#include <kernel/vfs.h>
#include <bus/bus.h>
#include <libc/string.h>
#include <sys/errors.h>

#include <bus/pci/pci_bus.h>

typedef struct bus {
	struct bus *next;
	const char *path;
} bus;

static bus *bus_list;
static mutex bus_lock;

int bus_man_init(kernel_args *ka)
{	
	mutex_init(&bus_lock, "bus_lock");

	bus_list = NULL;

	return 0;
}

static bus *find_bus(const char *path)
{
	bus *b;

	for(b = bus_list; b != NULL; b = b->next) {
		if(!strcmp(b->path, path))
			break;
	}
	return b;
}

int bus_register_bus(const char *path)
{
	bus *b;
	int err = 0;

	dprintf("bus_register_bus: path '%s'\n", path);
	
	mutex_lock(&bus_lock);

	if(!find_bus(path)) {
		b = (bus *)kmalloc(sizeof(bus));	
		if(b == NULL) {
			err = ERR_NO_MEMORY;
			goto err;
		}

		b->path = kmalloc(strlen(path)+1);
		if(b->path == NULL) {
			err = ERR_NO_MEMORY;
			kfree(b);
			goto err;
		}
		strcpy((char *)b->path, path);

		b->next = bus_list;
		bus_list = b;
	} else {
		err = ERR_NOT_FOUND;
	}

	err = 0;
err:
	mutex_unlock(&bus_lock);
	return err;
}

static int bus_find_device_recurse(int *n, char *base_path, int base_fd, id_list *vendor_ids, id_list *device_ids)
{
	char leaf[256];
	int base_path_len = strlen(base_path);
	int len;
	int fd;
	int err;
	
	len = sizeof(leaf);
	for(len = sizeof(leaf); sys_read(base_fd, &leaf, -1, &len) == 0 && len > 0; len = sizeof(leaf)) {
		// reset the base_path to the original string passed in
		base_path[base_path_len] = 0;

		strcat(base_path, leaf);
		fd = sys_open(base_path, "", STREAM_TYPE_DEVICE);
		if(fd < 0) {
			fd = sys_open(base_path, "", STREAM_TYPE_DIR);
			if(fd < 0)
				continue;
			strcat(base_path, "/");
			err = bus_find_device_recurse(n, base_path, fd, vendor_ids, device_ids);
			sys_close(fd);
			if(err >= 0)
				return err;
		}

		// we opened the device
		// XXX assumes PCI
		{
			struct pci_cfg cfg;
			uint32 i, j;
			
			err = sys_ioctl(fd, PCI_GET_CFG, &cfg, sizeof(struct pci_cfg));
			if(err >= 0) {
				// see if the vendor & device id matches
				for(i=0; i<vendor_ids->num_ids; i++) {
					if(cfg.vendor_id == vendor_ids->id[i]) {
						for(j=0; j<device_ids->num_ids; j++) {
							if(cfg.device_id == device_ids->id[j]) {
								// found it
								(*n)--;
								if(*n <= 0)
									return fd;
							}
						}
					}
				}
			}
			sys_close(fd);
		}
	}
	return ERR_NOT_FOUND;
}

int bus_find_device(int n, id_list *vendor_ids, id_list *device_ids, device *dev)
{
	int base_fd;
	int fd;
	char path[256];
	bus *b;
	int err = -1;

	for(b = bus_list; b != NULL && err < 0; b = b->next) {	
		base_fd = sys_open(b->path, "", STREAM_TYPE_DIR);
		if(base_fd < 0)
			continue;

		strcpy(path, b->path);
		strcat(path, "/");
		fd = bus_find_device_recurse(&n, path, base_fd, vendor_ids, device_ids);
		if(fd >= 0) {
			// we have a device!
			// XXX assumes pci
			struct pci_cfg cfg;
			int i;

			err = sys_ioctl(fd, PCI_GET_CFG, &cfg, sizeof(struct pci_cfg));
			if(err >= 0) {
				// copy the relevant data from the pci config to the more generic config
				memset(dev, 0, sizeof(device));
				dev->vendor_id = cfg.vendor_id;
				dev->device_id = cfg.device_id;
				dev->irq = cfg.irq;
				for(i=0; i<6; i++) {
					dev->base[i] = cfg.base[i];
					dev->size[i] = cfg.size[i];
				}
				strcpy(dev->dev_path, path);
			}
			sys_close(fd);
		}
		sys_close(base_fd);
	}

	return err;
}


