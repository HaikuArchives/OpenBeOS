/* 
** Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#ifndef _NEWOS_KERNEL_ARCH_KERNEL_H
#define _NEWOS_KERNEL_ARCH_KERNEL_H

#ifdef ARCH_i386
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

#endif
