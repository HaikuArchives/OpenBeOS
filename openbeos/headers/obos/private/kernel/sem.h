/* sem.h
 *
 * NB This file should be removed and the definitions here
 * added to OS.h as soon as we have it :)
 */

#ifndef _SEM_H
#define _SEM_H

#include <thread.h>
#include <stage2.h>
//#include <sem_types.h>

#define B_CAN_INTERRUPT          1
#define B_DO_NOT_RESCHEDULE      2
#define B_CHECK_PERMISSION       4
#define B_TIMEOUT                8
#define	B_RELATIVE_TIMEOUT       8
#define B_ABSOLUTE_TIMEOUT      16

typedef struct sem_info {
	sem_id		sem;
	proc_id		proc;
	char		name[SYS_MAX_OS_NAME_LEN];
	int32		count;
	thread_id	latest_holder;
} sem_info;


sem_id create_sem_etc(int count, const char *name, proc_id owner);
sem_id create_sem(int count, const char *name);
int    delete_sem(sem_id id);
int    delete_sem_etc(sem_id id, int return_code);
int    acquire_sem(sem_id id);
int    acquire_sem_etc(sem_id id, int count, int flags, bigtime_t timeout);
int    release_sem(sem_id id);
int    release_sem_etc(sem_id id, int count, int flags);
int    get_sem_count(sem_id id, int32* thread_count);
int    get_sem_info(sem_id id, struct sem_info *info);
int    get_next_sem_info(proc_id proc, uint32 *cookie, struct sem_info *info);
int    set_sem_owner(sem_id id, proc_id proc);

sem_id user_create_sem(int count, const char *name);
int    user_delete_sem(sem_id id);
int    user_delete_sem_etc(sem_id id, int return_code);
int    user_acquire_sem(sem_id id);
int    user_acquire_sem_etc(sem_id id, int count, int flags, bigtime_t timeout);
int    user_release_sem(sem_id id);
int    user_release_sem_etc(sem_id id, int count, int flags);
int    user_get_sem_count(sem_id id, int32* thread_count);
int    user_get_sem_info(sem_id id, struct sem_info *info);
int    user_get_next_sem_info(proc_id proc, uint32 *cookie, struct sem_info *info);
int    user_set_sem_owner(sem_id id, proc_id proc);


/* #ifdef _KERNEL_ */
int    sem_init(kernel_args *ka);
int    sem_delete_owned_sems(proc_id owner);
int    sem_interrupt_thread(struct thread *t);
/* #endif */

#endif /* _KERNEL_SEM_H */

