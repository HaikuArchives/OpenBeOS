/* timer_test - tests the net_timer implementation
**
** Initial version by Axel Dörfler, axeld@pinc-software.de
**
** This file may be used under the terms of the OpenBeOS License.
*/


#include "net_timer.h"

#include <OS.h>

#include <stdio.h>
#include <string.h>


#define TIMERS 10
#define SNOOZE 10
int32 gIntervals[TIMERS] = {2,3,5,7,3,6,4,2,5,8};
int32 gStart;


void hook(void *data)
{
	int32 num = (int32)data;

	printf("%ld at %ld (interval %ld secs)\n",num,real_time_clock() - gStart,gIntervals[num - 1]);
}


int main(void)
{
	net_timer_id timer[TIMERS];
	int32 i,half = TIMERS / 2;

	if (net_init_timer() < B_OK) {
		puts("could not initialize timer!");
		return -1;
	}

	gStart = real_time_clock();

	for (i = 0;i < TIMERS;i++)
		timer[i] = net_add_timer(hook,(void *)((uint32)i + 1UL),gIntervals[i] * 1000000LL);

	puts("snoozing...");
	snooze(SNOOZE * 1000000LL);

	puts("remove some timers and snooze again...");
	for (i = 0;i < half;i++)
		net_remove_timer(timer[i]);

	snooze(SNOOZE * 1000000LL);

	puts("remove the rest and snooze again...");
	for (i = half;i < TIMERS;i++)
		net_remove_timer(timer[i]);

	snooze(SNOOZE * 1000000LL);

	net_shutdown_timer();
	return 0;
}
