#include <unistd.h>
#include <kernel/image.h>
#include <kernel/OS.h>
#include <iovec.h>
#include <stdio.h>
#include <sys/time.h>
#include <posix/signal.h>
 
#include "net_stack_driver.h"

extern const char * g_stack_driver_path;

typedef int (*select_function)(int, struct fd_set *, struct fd_set *,
								struct fd_set *, struct timeval *);

void on_select_timeout(int signal);

_EXPORT int select(int nbits, struct fd_set *rbits, 
                      struct fd_set *wbits, 
                      struct fd_set *ebits, 
                      struct timeval *timeout)
{
	static select_function sf = NULL;

	image_id iid;
	int tmpfd;
	struct select_args args;
	int rv;
	__signal_func_ptr previous_sigalrm_handler = NULL;
	bigtime_t when = 0;
	
	if (sf == NULL) {
		// first time, try to figure if libroot.so have a (better) select() support
		// TODO: search in LIBRARY_PATH environment variable, not in hardcoded place!
		iid = load_add_on("/boot/beos/system/lib/libroot.so");
		if (iid > 0 ) {
			if (get_image_symbol(iid, "select", B_SYMBOL_TYPE_TEXT, (void **) &sf) != B_OK)
				// libroot.so don't export a select() function, so we use our
				sf = select;
			unload_add_on(iid);
		};
		
	};
	
	if (sf != select)
		// pass the call to libroot.so one...
		return sf(nbits, rbits, wbits, ebits, timeout);
	
	tmpfd = open(g_stack_driver_path, O_RDWR);
	if (tmpfd < 0)
		return tmpfd;

	args.nbits = nbits;
	args.rbits = rbits;
	args.wbits = wbits;
	args.ebits = ebits;
	args.timeout = timeout;

	if (timeout) {
		bigtime_t duration;

		// BeOS R5.0.3- select() don't honnor timeout arg!
		// Here we use a alarm signal to work around this bug
		duration = timeout->tv_sec * 1000000 + timeout->tv_usec;
		if (duration < 1000) 
			duration = 1000;	// non-blocking mode (timeval = 0) don't works too,
								// and too small timeout will be fired *before*
								// we've call ioctl()!
		previous_sigalrm_handler = signal(SIGALRM, on_select_timeout);
		when = system_time() + duration;
		set_alarm(duration, B_ONE_SHOT_RELATIVE_ALARM);
	};
	
	args.rv = B_OK;
	rv = ioctl(tmpfd, NET_STACK_SELECT, &args, sizeof(args));
	if (rv == 0)
		rv = args.rv;
		
	if (timeout) {
		signal(SIGALRM, previous_sigalrm_handler);
		if (when < system_time())
			rv = 0;	// okay, select() timed out, so return 0 (see select() man page)
	};
		
	close(tmpfd);
	
	return rv;
}


void on_select_timeout(int signal)
{
	printf("select() timed out!\n");
}

