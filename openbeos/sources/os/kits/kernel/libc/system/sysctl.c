#include <ktypes.h>
#include <sysctl.h>

int sys_sysctl(int *, uint, void *, size_t *, void *, size_t);

int sysctl(int *name, uint namelen, void *oldp, size_t *oldlenp,
           void *newp, size_t newlen)
{
	/* we should handle CTL_USER here, but as we don't even define it yet :) */
	int err = sys_sysctl(name, namelen, oldp, oldlenp, newp, newlen);
	
	if (err < 0) {
		/* XXX - set errno */
	}
	
	return err;
}
