/*
** Copyright 2001, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#ifndef _NEWOS_KERNEL_ATOMIC_H
#define _NEWOS_KERNEL_ATOMIC_H

#ifdef __cplusplus
extern "C" {
#endif 

/* XXX - atomic_set is defined as using  avolatile as this stops a 
 * compiler warning, but is there any reason why they shouldn't all
 * be so defined? They were in arch/cpu.h...
 */
int atomic_add(int *val, int incr);
int atomic_and(int *val, int incr);
int atomic_or(int *val, int incr);
int atomic_set(volatile int *val, int set_to);
int test_and_set(int *val, int set_to, int test_val);

#ifdef __cplusplus
}
#endif 

#endif /* _NEWOS_KERNEL_ATOMIC_H */

