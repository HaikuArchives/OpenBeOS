/* 
** Copyright 2001, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#ifndef _ARCH_THREAD_H
#define _ARCH_THREAD_H

#include <kernel/thread.h>

int arch_proc_init_proc_struct(struct proc *p, bool kernel);
int arch_thread_init_thread_struct(struct thread *t);
void arch_thread_context_switch(struct thread *t_from, struct thread *t_to);
int arch_thread_initialize_kthread_stack(struct thread *t, int (*start_func)(void), void (*entry_func)(void), void (*exit_func)(void));
void arch_thread_dump_info(void *info);
void arch_thread_enter_uspace(addr entry, addr ustack_top);
void arch_thread_switch_kstack_and_call(struct thread *t, addr new_kstack, void (*func)(void *), void *arg);

#endif

