#include <unistd.h>
#include <kernel/image.h>
#include <kernel/OS.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <posix/errno.h>

#include "net_stack_driver.h"

#ifndef BONE_VERSION
_EXPORT int select(int nbits, struct fd_set * rbits, 
                      struct fd_set * wbits, 
                      struct fd_set * ebits, 
                      struct timeval * tv)
{
	int fd;
	int n;
	struct select_args args;
	struct r5_selectsync rss;
	status_t status;
	struct fd_set r, w, e;


	if (rbits) {
		memcpy(&r, rbits, sizeof(r));
		FD_ZERO(rbits);
	}
	if (wbits){
		memcpy(&w, wbits, sizeof(w));
		FD_ZERO(wbits);
	}
	if (ebits) {
		memcpy(&e, ebits, sizeof(e));
		FD_ZERO(ebits);
	}

	rss.lock = create_sem(1, "r5_select_lock");
	rss.wait = create_sem(0, "r5_select_wait");
	
	rss.nfd = 0;
	rss.rbits = rbits;
	rss.wbits = wbits;
	rss.ebits = ebits;

	args.sync = (struct selectsync *) &rss;

	/* call, indirectly, net_stack_select() for each event to monitor
	 * as we are not the vfs, we can't call this device hook ourself, but 
	 * our NET_STACK_SELECT opcode would do it for us...
	 */
	n = 0;
	for(fd = 1; fd < nbits; fd++) {
		if (FD_ISSET(fd, &r)) {
			args.ref = (fd << 8) | 1;
			if (ioctl(fd, NET_STACK_SELECT, &args, sizeof(args)) >= 0)
				n++;
    	};
		if (FD_ISSET(fd, &w)) {
			args.ref = (fd << 8) | 2;
			if (ioctl(fd, NET_STACK_SELECT, &args, sizeof(args)) >= 0)
				n++;
    	};
		if (FD_ISSET(fd, &e)) {
			args.ref = (fd << 8) | 3;
			if (ioctl(fd, NET_STACK_SELECT, &args, sizeof(args)) >= 0)
				n++;
    	};
	}
	
	if (n < 1) {
		errno = B_BAD_VALUE;
		delete_sem(rss.lock);
		delete_sem(rss.wait);
		return -1;
	};

	// okay, up to now no event was notified
	if (tv) {
		bigtime_t timeout;

		timeout = tv->tv_sec * 1000000 + tv->tv_usec;
 		status = acquire_sem_etc(rss.wait, 1, B_RELATIVE_TIMEOUT | B_CAN_INTERRUPT, timeout);
	} else
		status = acquire_sem(rss.wait);  


	// unregister socket event notification
 	for(fd = 1; fd < nbits; fd++) {
		if (FD_ISSET(fd, &r)) {
			args.ref = (fd << 8) | 1;
       		ioctl(fd, NET_STACK_DESELECT, &args, sizeof(args));
    	}
		if (FD_ISSET(fd, &w)) {
			args.ref = (fd << 8) | 2;
			ioctl(fd, NET_STACK_DESELECT, &args, sizeof(args));
    	}
		if (FD_ISSET(fd, &e)) {
			args.ref = (fd << 8) | 3;
			ioctl(fd, NET_STACK_DESELECT, &args, sizeof(args));
		}
	};

	delete_sem(rss.lock);
	delete_sem(rss.wait);

	if (status == B_TIMED_OUT)
		return 0;

	if (status != B_OK && status != B_WOULD_BLOCK) {
		errno = status;
    	return -1;
	};

	return rss.nfd;
}
#endif
