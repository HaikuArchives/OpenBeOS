///-*-C++-*-//////////////////////////////////////////////////////////////////
//
// Hoard: A Fast, Scalable, and Memory-Efficient Allocator
//        for Shared-Memory Multiprocessors
// Contact author: Emery Berger, http://www.cs.utexas.edu/users/emery
//
// Copyright (c) 1998-2000, The University of Texas at Austin.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as
// published by the Free Software Foundation, http://www.fsf.org.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _ARCH_SPECIFIC_H_
#define _ARCH_SPECIFIC_H_

#include "config.h"

#if 1

//#include <syscalls.h>
#include <ktypes.h>
#include <types.h>
#include <assert.h>

#else

// Wrap architecture-specific functions.
#endif

#if 1

typedef struct {
  int32 ben;
  sem_id sem;
} hoardLockType;
typedef thread_id		hoardThreadType;
// Commented this out - why would we have new and operator in C code?
//inline void * operator new(size_t, void *_P)
//	{return (_P); }

#endif

extern "C" {

  ///// Thread-related wrappers.

  void	hoardCreateThread (hoardThreadType& t,
			   void *(*function) (void *),
			   void * arg);
  void	hoardJoinThread (hoardThreadType& t);
  void  hoardSetConcurrency (int n);

  // Return a thread identifier appropriate for hashing:
  // if the system doesn't produce consecutive thread id's,
  // some hackery may be necessary.
  int	hoardGetThreadID (void);

  ///// Lock-related wrappers.

#if !defined(WIN32) && !defined(__BEOS__) && !USER_LOCKS

  // Define the lock operations inline to save a little overhead.

  inline void hoardLockInit (hoardLockType& lock) {
    pthread_mutex_init (&lock, NULL);
  }

  inline void hoardLock (hoardLockType& lock) {
    pthread_mutex_lock (&lock);
  }

  inline void hoardUnlock (hoardLockType& lock) {
    pthread_mutex_unlock (&lock);
  }
#else
  void	hoardLockInit (hoardLockType& lock);
  void	hoardLock (hoardLockType& lock);
  void	hoardUnlock (hoardLockType& lock);

#endif

  ///// Memory-related wrapper.

  int	hoardGetPageSize (void);
  void *	hoardSbrk (long size);
  void	hoardUnsbrk (void * ptr, long size);

  ///// Other.

  void  hoardYield (void);
  int	hoardGetNumProcessors (void);
  unsigned long hoardInterlockedExchange (unsigned long * oldval,
					  unsigned long newval);
}

#endif // _ARCH_SPECIFIC_H_

