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

/*
  processheap.h
  ------------------------------------------------------------------------
  We use one processHeap for the whole program.
  ------------------------------------------------------------------------
  @(#) $Id$
  ------------------------------------------------------------------------
  Emery Berger                    | <http://www.cs.utexas.edu/users/emery>
  Department of Computer Sciences |             <http://www.cs.utexas.edu>
  University of Texas at Austin   |                <http://www.utexas.edu>
  ========================================================================
*/

#ifndef _PROCESSHEAP_H_
#define _PROCESSHEAP_H_

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "arch-specific.h"
#include "heap.h"
#if USE_PRIVATE_HEAPS
#include "privateheap.h"
#define HEAPTYPE privateHeap
#else
#define HEAPTYPE threadHeap
#include "threadheap.h"
#endif

#if HEAP_LOG
#include "memstat.h"
#include "log.h"
#endif

class processHeap : public hoardHeap {

public:

  // Always grab at least this many superblocks' worth of memory which
  // we parcel out.
  enum { REFILL_NUMBER_OF_SUPERBLOCKS = 16 };

  processHeap (void);

  ~processHeap (void) {
#if HEAP_STATS
    stats();
#endif
  }

  // Memory deallocation routines.
  void free (void * ptr);

  // Print out statistics information.
  void stats (void);

  // Get a thread heap index.
  inline int getHeapIndex (void);

  // Get the thread heap with index i.
  inline HEAPTYPE& getHeap (int i);

  // Extract a superblock.
  inline superblock * acquire (const int c,
			       hoardHeap * dest);

  // Get space for a superblock.
  inline char * getSuperblockBuffer (void);

  // Insert a superblock.
  inline void release (superblock * sb);

#if HEAP_LOG
  // Get the log for index i.
  inline Log<MemoryRequest>& getLog (int i);
#endif

#if HEAP_FRAG_STATS
  // Declare that we have allocated an object.
  void setAllocated (int requestedSize,
		     int actualSize);

  // Declare that we have deallocated an object.
  void setDeallocated (int requestedSize,
		       int actualSize);

  // Return the number of wasted bytes at the high-water mark
  // (maxAllocated - maxRequested)
  inline int getFragmentation (void);

  int getMaxAllocated (void) {
    return _maxAllocated;
  }

  int getInUseAtMaxAllocated (void) {
    return _inUseAtMaxAllocated;
  }

  int getMaxRequested (void) {
    return _maxRequested;
  }

#endif

private:

  // Hide the lock & unlock methods.

  void lock (void) {
    hoardHeap::lock();
  }

  void unlock (void) {
    hoardHeap::unlock();
  }

  // Prevent copying and assignment.
  processHeap (const processHeap&);
  const processHeap& operator= (const processHeap&);

  // The per-thread heaps.
  HEAPTYPE theap[MAX_HEAPS];

#if HEAP_FRAG_STATS
  // Statistics required to compute fragmentation.  We cannot
  // unintrusively keep track of these on a multiprocessor, because
  // this would become a bottleneck.

  int _currentAllocated;
  int _currentRequested;
  int _maxAllocated;
  int _maxRequested;
  int _inUseAtMaxAllocated;
  int _fragmentation;

  // A lock to protect these statistics.
  hoardLockType _statsLock;
#endif

#if HEAP_LOG
  Log<MemoryRequest> _log[MAX_HEAPS + 1];
#endif

  // A lock for the superblock buffer.
  hoardLockType _bufferLock;

  char * 	_buffer;
  int 		_bufferCount;
};


HEAPTYPE& processHeap::getHeap (int i)
{
  assert (i >= 0);
  assert (i < MAX_HEAPS);
  return theap[i];
}


#if HEAP_LOG
Log<MemoryRequest>& processHeap::getLog (int i)
{
  assert (i >= 0);
  assert (i < MAX_HEAPS + 1);
  return _log[i];
}
#endif


// Return ceil(log_2(num)).
// num must be positive.
static int lg (int num)
{
  assert (num > 0);
  int power = 0;
  int n = 1;
  // Invariant: 2^power == n.
  while (n < num) {
    n <<= 1;
    power++;
  }
  return power;
}


// Hash out the thread id to a heap and return an index to that heap.
int processHeap::getHeapIndex (void) {
  // Here we use the number of processors as the maximum number of heaps.
  // In fact, for efficiency, we just round up to the highest power of two,
  // times two.
  int tid = hoardGetThreadID() & _numProcessorsMask;
  assert (tid < MAX_HEAPS);
  return tid;
}


superblock * processHeap::acquire (const int sizeclass,
				   hoardHeap * dest)
{
  lock ();

  // Remove the superblock with the most free space.
  superblock * maxSb = removeMaxSuperblock (sizeclass);
  if (maxSb) {
    maxSb->setOwner (dest);
  }

  unlock ();

  return maxSb;
}


inline char * processHeap::getSuperblockBuffer (void)
{
  char * buf;
  hoardLock (_bufferLock);
  if (_bufferCount == 0) {
    _buffer = (char *) hoardSbrk (SUPERBLOCK_SIZE * REFILL_NUMBER_OF_SUPERBLOCKS);
    _bufferCount = REFILL_NUMBER_OF_SUPERBLOCKS;
  }
  buf = _buffer;
  _buffer += SUPERBLOCK_SIZE;
  _bufferCount--;
  hoardUnlock (_bufferLock);
  return buf;
}


// Put a superblock back into our list of superblocks.
void processHeap::release (superblock * sb)
{
  assert (EMPTY_FRACTION * sb->getNumAvailable() > sb->getNumBlocks());

  lock();

  // Insert the superblock.
  insertSuperblock (sb->getBlockSizeClass(), sb, this);

  unlock();
}


#endif // _PROCESSHEAP_H_

