/*
** Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#ifndef _NEWOS_KERNEL_ARCH_CPU_H
#define _NEWOS_KERNEL_ARCH_CPU_H

#include <kernel.h>
#include <ktypes.h>
#include <stage2.h>

#define PAGE_ALIGN(x) (((x) + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1))

#ifdef __cplusplus
extern "C" {
#endif 
int atomic_add(volatile int *val, int incr);
int atomic_and(volatile int *val, int incr);
int atomic_or(volatile int *val, int incr);
int atomic_set(volatile int *val, int set_to);
int test_and_set(int *val, int set_to, int test_val);

bigtime_t system_time(void);
int arch_cpu_preboot_init(kernel_args *ka);
int arch_cpu_init(kernel_args *ka);
int arch_cpu_init2(kernel_args *ka);
void reboot(void);
#ifdef __cplusplus
}
#endif 

void arch_cpu_invalidate_TLB_range(addr start, addr end);
void arch_cpu_invalidate_TLB_list(addr pages[], int num_pages);
void arch_cpu_global_TLB_invalidate(void);

int arch_cpu_user_memcpy(void *to, const void *from, size_t size, addr *fault_handler);
int arch_cpu_user_strcpy(char *to, const char *from, addr *fault_handler);
int arch_cpu_user_strncpy(char *to, const char *from, size_t size, addr *fault_handler);
int arch_cpu_user_memset(void *s, char c, size_t count, addr *fault_handler);

#ifdef ARCH_x86
#include <arch/x86/cpu.h>
#endif
#ifdef ARCH_sh4
#include <arch/sh4/cpu.h>
#endif
#ifdef ARCH_alpha
#include <arch/alpha/cpu.h>
#endif
#ifdef ARCH_sparc64
#include <arch/sparc64/cpu.h>
#endif
#ifdef ARCH_mips
#include <arch/mips/cpu.h>
#endif
#ifdef ARCH_ppc
#include <arch/ppc/cpu.h>
#endif
#ifdef ARCH_m68k
#include <arch/m68k/cpu.h>
#endif

#endif

