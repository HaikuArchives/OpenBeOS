/* OS.h
 * 
 * Quake with fear - it's back!!!
 */
 

#ifndef _OS_H
#define _OS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ktypes.h>


/* Threads */

//enum {
//	THREAD_STATE_READY = 0,   // ready to run
//	THREAD_STATE_RUNNING, // running right now somewhere
//	THREAD_STATE_WAITING, // blocked on something
//	THREAD_STATE_SUSPENDED, // suspended, not in queue
//	THREAD_STATE_FREE_ON_RESCHED, // free the thread structure upon reschedule
//	THREAD_STATE_BIRTH	// thread is being created
//};

typedef enum {
	B_THREAD_RUNNING=1,
	B_THREAD_READY,
	B_THREAD_RECEIVING,
	B_THREAD_ASLEEP,
	B_THREAD_SUSPENDED,
	B_THREAD_WAITING
} thread_state;

#define THREAD_IDLE_PRIORITY 0

#define THREAD_NUM_PRIORITY_LEVELS 64
#define THREAD_MIN_PRIORITY    (THREAD_IDLE_PRIORITY + 1)
#define THREAD_MAX_PRIORITY    (THREAD_NUM_PRIORITY_LEVELS - THREAD_NUM_RT_PRIORITY_LEVELS - 1)

#define THREAD_NUM_RT_PRIORITY_LEVELS 16
#define THREAD_MIN_RT_PRIORITY (THREAD_MAX_PRIORITY + 1)
#define THREAD_MAX_RT_PRIORITY (THREAD_NUM_PRIORITY_LEVELS - 1)

#define THREAD_LOWEST_PRIORITY    THREAD_MIN_PRIORITY
#define THREAD_LOW_PRIORITY       12
#define THREAD_MEDIUM_PRIORITY    24
#define THREAD_HIGH_PRIORITY      36
#define THREAD_HIGHEST_PRIORITY   THREAD_MAX_PRIORITY

#define THREAD_RT_LOW_PRIORITY    THREAD_MIN_RT_PRIORITY
#define THREAD_RT_HIGH_PRIORITY   THREAD_MAX_RT_PRIORITY

#define B_LOW_PRIORITY						5
#define B_NORMAL_PRIORITY					10
#define B_DISPLAY_PRIORITY					15
#define	B_URGENT_DISPLAY_PRIORITY			20
#define	B_REAL_TIME_DISPLAY_PRIORITY		100
#define	B_URGENT_PRIORITY					110
#define B_REAL_TIME_PRIORITY				120

/* temporary hacks */
typedef int32 team_id;
#define B_OS_NAME_LENGTH 32

typedef struct  {
	thread_id		thread;
	team_id			team;
	char			name[B_OS_NAME_LENGTH];
	thread_state	state;
	int32			priority;
	sem_id			sem;
	bigtime_t		user_time;
	bigtime_t		kernel_time;
	void			*stack_base;
	void			*stack_end;
} thread_info;

typedef struct {
	bigtime_t		user_time;
	bigtime_t		kernel_time;
} team_usage_info;

typedef int32 (*thread_func) (void *);

thread_id spawn_thread (thread_func, const char *, int32, void *);
int       kill_thread(thread_id thread);
int       resume_thread(thread_id thread);
int       suspend_thread(thread_id thread);

thread_id find_thread(const char *);

#ifdef __cplusplus
}
#endif

#endif /* _OS_H */

