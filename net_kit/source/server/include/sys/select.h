/* select.h */

#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H

#include <sys/time.h> /* for struct timeval */
/*
 * You can define your own FDSETSIZE if you want more bits
 */

#ifndef FD_SETSIZE
#define FD_SETSIZE 256
#endif /* FD_SETSIZE */


/*
 * Compatibily only: use FD_SETSIZE instead
 */
#ifndef FDSETSIZE
#define FDSETSIZE FD_SETSIZE
#endif /* FDSETSIZE */

#define NFDBITS 32

typedef struct fd_set {
	unsigned mask[FDSETSIZE / NFDBITS];
} fd_set;

#define _FDMSKNO(fd) ((fd) / NFDBITS)
#define _FDBITNO(fd) ((fd) % NFDBITS)
#define FD_ZERO(setp) memset((setp)->mask, 0, sizeof((setp)->mask))
#define FD_SET(fd, setp) ((setp)->mask[_FDMSKNO(fd)] |= (1 << (_FDBITNO(fd))))
#define FD_CLR(fd, setp) ((setp)->mask[_FDMSKNO(fd)] &= ~(1 << (_FDBITNO(fd))))
#define FD_ISSET(fd, setp) ((setp)->mask[_FDMSKNO(fd)] & (1 << (_FDBITNO(fd))))

#ifdef _NETWORK_STACK
int select(int nbits, struct fd_set *rbits, 
                      struct fd_set *wbits, 
                      struct fd_set *ebits, 
                      struct timeval *timeout);
#else
int net_select(int nbits, struct fd_set *rbits, 
                      struct fd_set *wbits, 
                      struct fd_set *ebits, 
                      struct timeval *timeout);
#endif
#endif /* _SYS_SELECT_H */
