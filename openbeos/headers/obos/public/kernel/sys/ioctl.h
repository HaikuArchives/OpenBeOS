/**
 * @file sys/ioctl.h
 * @brief I/O control functions
 */

#ifndef _SYS_IOCTL_H
#define _SYS_IOCTL_H

/**
 * @defgroup IOCTL sys/ioctl.h
 * @brief I/O control functions
 * @ingroup OpenBeOS_POSIX
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif


/* These all belong is sys/ */
/* XXX - change these when sys/ is available */
#include <sys/ioccom.h>

#include <sys/filio.h>
#include <socket.h>

#ifndef _KERNEL_
  /** @fn int ioctl(int fd, ulong cmd, ...)
   * Manipulates the characteristics of the affected descriptor. May be used
   * on all forms of descriptor, including sockets and pipes. cmd should be one
   * of the values given in 
   * @ref IOCTL_cmds
   */
  int ioctl(int, ulong, ...);
#endif /* !_KERNEL_ */

/** @} */
#endif /* _SYS_IOCTL_H */
