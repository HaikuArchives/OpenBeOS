#ifdef ARCH_x86
#include "arch/x86/syscalls.inc"
#endif
#ifdef ARCH_alpha
#include "arch/alpha/syscalls.inc"
#endif
#ifdef ARCH_sh4
#include "arch/sh4/syscalls.inc"
#endif
#ifdef ARCH_sparc
#include "arch/sparc/syscalls.inc"
#endif
#ifdef ARCH_sparc64
#include "arch/sparc64/syscalls.inc"
#endif
#ifdef ARCH_mips
#include "arch/mips/syscalls.inc"
#endif
#ifdef ARCH_ppc
#include "arch/ppc/syscalls.inc"
#endif
#ifdef ARCH_m68k
#include "arch/m68k/syscalls.inc"
#endif



SYSCALL0(sys_null, 0)
SYSCALL4(sys_mount, 1)
SYSCALL1(sys_unmount, 2)
SYSCALL0(sys_sync, 3)
SYSCALL3(sys_open, 4)
SYSCALL1(sys_close, 5)
SYSCALL1(sys_fsync, 6)
SYSCALL5(sys_read, 7)
SYSCALL5(sys_write, 8)
SYSCALL4(sys_seek, 9)
SYSCALL4(sys_ioctl, 10)
SYSCALL2(sys_create, 11)
SYSCALL1(sys_unlink, 12)
SYSCALL2(sys_rename, 13)
SYSCALL2(sys_rstat, 14)
SYSCALL3(sys_wstat, 15)
SYSCALL0(sys_system_time, 16)
SYSCALL2(sys_snooze, 17)
SYSCALL2(sys_sem_create, 18)
SYSCALL1(sys_sem_delete, 19)
SYSCALL2(sys_sem_acquire, 20)
SYSCALL4(sys_sem_acquire_etc, 21)
SYSCALL2(sys_sem_release, 22)
SYSCALL3(sys_sem_release_etc, 23)
SYSCALL0(sys_get_current_thread_id, 24)
SYSCALL1(sys_exit, 25)
SYSCALL5(sys_proc_create_proc, 26)
SYSCALL2(sys_thread_wait_on_thread, 27)
SYSCALL2(sys_proc_wait_on_proc, 28)
SYSCALL6(sys_vm_create_anonymous_region, 29)
SYSCALL6(sys_vm_clone_region, 30)
SYSCALL9(sys_vm_map_file, 31)
SYSCALL1(sys_vm_delete_region, 32)
SYSCALL2(sys_vm_get_region_info, 33)
SYSCALL1(sys_find_region_by_name, 34)
SYSCALL3(sys_thread_create_thread, 35)
SYSCALL1(sys_thread_kill_thread, 36)
SYSCALL1(sys_thread_suspend_thread, 37)
SYSCALL1(sys_thread_resume_thread, 38)
SYSCALL1(sys_proc_kill_proc, 39)
SYSCALL0(sys_get_current_proc_id, 40)
SYSCALL2(sys_getcwd, 41)
SYSCALL1(sys_setcwd, 42)
SYSCALL2(sys_port_create, 43)
SYSCALL1(sys_port_close, 44)
SYSCALL1(sys_port_delete, 45)
SYSCALL1(sys_port_find, 46)
SYSCALL2(sys_port_get_info, 47)
SYSCALL3(sys_port_get_next_port_info, 48)
SYSCALL1(sys_port_buffer_size, 49)
SYSCALL4(sys_port_buffer_size_etc, 50)
SYSCALL1(sys_port_count, 51)
SYSCALL4(sys_port_read, 52)
SYSCALL7(sys_port_read_etc, 53)
SYSCALL2(sys_port_set_owner, 54)
SYSCALL4(sys_port_write, 55)
SYSCALL7(sys_port_write_etc, 56)
SYSCALL2(sys_sem_get_count, 57)
SYSCALL2(sys_sem_get_sem_info, 58)
SYSCALL3(sys_sem_get_next_sem_info, 59)
SYSCALL2(sys_set_sem_owner, 60)
SYSCALL1(sys_dup, 61)
SYSCALL2(sys_dup2, 62)
SYSCALL2(sys_proc_get_table, 63)
SYSCALL2(sys_getrlimit, 64)
SYSCALL2(sys_setrlimit, 65)
SYSCALL2(sys_atomic_add, 66)
SYSCALL2(sys_atomic_and, 67)
SYSCALL2(sys_atomic_or, 68)
SYSCALL2(sys_atomic_set, 69)
SYSCALL3(sys_test_and_set, 70)
SYSCALL6(sys_sysctl, 71)
SYSCALL3(sys_socket, 72)

