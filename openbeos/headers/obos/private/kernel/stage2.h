/* 
** Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/

// this file declares stuff like addr_range, MAX_*, etc.
#include <stage2_struct.h>

#ifdef ARCH_i386
#include <arch/x86/stage2.h>
#endif
#ifdef ARCH_sh4
#include <arch/sh4/stage2.h>
#endif
#ifdef ARCH_sparc64
#include <arch/sparc64/stage2.h>
#endif
#ifdef ARCH_mips
#include <arch/mips/stage2.h>
#endif
#ifdef ARCH_ppc
#include <arch/ppc/stage2.h>
#endif
#ifdef ARCH_m68k
#include <arch/m68k/stage2.h>
#endif

#ifndef _NEWOS_BOOT_STAGE2_H
#define _NEWOS_BOOT_STAGE2_H

// kernel args
typedef struct ka {
	unsigned int cons_line;
	char *str;
	addr_range bootdir_addr;
	addr_range kernel_seg0_addr;
	addr_range kernel_seg1_addr;
	addr_range kernel_dynamic_section_addr;
	unsigned int num_phys_mem_ranges;

	addr_range phys_mem_range[MAX_PHYS_MEM_ADDR_RANGE];
	unsigned int num_phys_alloc_ranges;
	addr_range phys_alloc_range[MAX_PHYS_ALLOC_ADDR_RANGE];
	unsigned int num_virt_alloc_ranges;
	addr_range virt_alloc_range[MAX_VIRT_ALLOC_ADDR_RANGE];
	unsigned int num_cpus;
	addr_range cpu_kstack[MAX_BOOT_CPUS];

	arch_kernel_args arch_args;

	struct framebuffer {
	int enabled;
	int x_size;
	int y_size;
	int bit_depth;
	int already_mapped;
	addr_range mapping;
	} fb;

} kernel_args;


#endif
