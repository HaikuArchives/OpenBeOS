/* if helper functions */

#include <kernel/OS.h>

#include "if.h"

void *protocol_address(struct ifnet *ifa, int family)
{
	struct ifaddr *a = ifa->if_addrlist;

	while (a) {
		if (a->if_addr.sa_family == family)
			return &a->if_addr.sa_data;
		a = a->next;
	}
	return NULL;
}

