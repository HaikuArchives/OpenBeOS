/* prio.c - prio command for BeOs, change priority of a given thread
 * (c) 2001, 2002, François Revol (mmu_man) for OpenBeOS
 * released under the MIT licence.
 *
 * ChangeLog:
 * 04-25-2002 v1.0
 *  Initial. Used my renice.c code to rewrite 'prio' BeOS command for OpenBeOS.
 * 
 * prio is a stripped-down version of renice
 * seems to behave the same way as the original BeOS version. :)
 */

#include <OS.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	thread_id th;
	int32 prio;
	status_t ret;
	
	if (argc != 3) {
		puts("Usage: prio pid newpriority");
		return 1;
	}

	th = atoi(argv[1]);
	prio = atoi(argv[2]);

	// ret > 0 means successful, and is the previous priority
	ret = set_thread_priority(th, prio);
	if (ret >= B_OK)
		return 0;
	puts("Priority changed failed.");
	return 1;
}

