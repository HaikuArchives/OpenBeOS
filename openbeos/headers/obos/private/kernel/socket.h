/* socket.h
 * Very simply file. This will grow!
 */

#ifndef _KERNEL_SOCKET_H
#define _KERNEL_SOCKET_H


#ifdef _KERNEL_
int socket(int, int, int, bool);

#else
int socket(int, int, int);

#endif



#endif	/* _KERNEL_SOCKET_H */
