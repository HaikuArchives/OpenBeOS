/* 
** Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#ifndef _KERNEL_ARCH_KERNEL_H
#define _KERNEL_ARCH_KERNEL_H

#ifdef ARCH_x86
#include <arch/x86/kernel.h>
#endif
#ifdef ARCH_sh4
#include <arch/sh4/kernel.h>
#endif
#ifdef ARCH_alpha
#include <arch/alpha/kernel.h>
#endif
#ifdef ARCH_sparc64
#include <arch/sparc64/kernel.h>
#endif
#ifdef ARCH_mips
#include <arch/mips/kernel.h>
#endif
#ifdef ARCH_ppc
#include <arch/ppc/kernel.h>
#endif
#ifdef ARCH_m68k
#include <arch/m68k/kernel.h>
#endif

#define KSTACK_SIZE (PAGE_SIZE*2)
#define STACK_SIZE  (PAGE_SIZE*16)

#define ROUNDUP(a, b) (((a) + ((b)-1)) & ~((b)-1))
#define ROUNDOWN(a, b) (((a) / (b)) * (b))

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define CHECK_BIT(a, b) ((a) & (1 << (b)))
#define SET_BIT(a, b) ((a) | (1 << (b)))
#define CLEAR_BIT(a, b) ((a) & (~(1 << (b))))

#endif
