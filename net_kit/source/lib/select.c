#include <unistd.h>
#include <fcntl.h>
 
#include "net/stack_driver.h"

extern const char * g_stack_driver_path;

// -------------------------------
int select(int nbits, struct fd_set *rbits, struct fd_set *wbits, struct fd_set *ebits, struct timeval *timeout)
{
	int				tmp_sock;
	select_ioctl	ctl;
	int				rc;

	tmp_sock = open(g_stack_driver_path, O_RDWR);
	if (tmp_sock < 0)
		return tmp_sock;

	ctl.nbits 	= nbits;
	ctl.rbits 	= rbits;
	ctl.wbits 	= wbits;
	ctl.ebits 	= ebits;
	ctl.timeout	= timeout;

	rc = ioctl(tmp_sock, NET_STACK_SELECT, &ctl);

	close(tmp_sock);
	return rc;
}
