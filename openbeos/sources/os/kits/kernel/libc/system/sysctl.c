#include <ktypes.h>
#include <sysctl.h>
#include <socket.h>

int sys_sysctl(int *, uint, void *, size_t *, void *, size_t);
int sys_socket(int, int, int);

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

/* This should maybe have it's own file but that seems a bit daft.
 * Hmm, maybe this file should just have a more generic name?
 */
int socket(int dom, int type, int prot)
{
	int err = sys_socket(dom, type, prot);
	
	if (err < 0) {
		/* XXX - set errno */
	}
	
	return err;
}
