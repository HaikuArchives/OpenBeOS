/* net.c */

/* XXX - add documentation to this file! */
#include <kernel.h>
#include <sem.h>
#include <pools.h>
#include <vm.h>
#include <memheap.h>
#include <atomic.h>
#include <ktypes.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <module.h>
#include <pools.h>
#include <debug.h>
#include <module.h>

#define NETWORK_INTERFACES "network/interface"

int net_init(kernel_args *ka);
int sockets_init(kernel_args *ka);

/* XXX temp hack */
struct device_mi {
	module_info info;
	int (*init)(void);
};

static void find_interface_modules(void)
{
	void *ml = open_module_list(NETWORK_INTERFACES);
	size_t sz = SYS_MAX_PATH_LEN;
	char name[sz];
	struct device_mi *mi;
//	device_module_info *dmi = NULL;
	int rv;

	if (ml == NULL) {
		dprintf("failed to open the %s directory\n", 
			NETWORK_INTERFACES);
		return;
	}

	while (read_next_module_name(ml, name, &sz) == 0) {
		dprintf("*** net interface module: %s\n", name);
		rv = get_module(name, (module_info**)&mi);
		if (rv == 0) {
			mi->init();
		}
		sz = SYS_MAX_PATH_LEN;
	}

	close_module_list(ml);
}

int net_init(kernel_args *ka)
{
	dprintf("net_init: starting!\n");
	sockets_init(ka);
	find_interface_modules();
	return 0;
}
