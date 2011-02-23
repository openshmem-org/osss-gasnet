/*
 *    Copyright (c) 1996-2002 by Quadrics Supercomputers World Ltd.
 *    Copyright (c) 2003-2005 by Quadrics Ltd.
 *
 *    For licensing information please see the supplied COPYING file
 *
 */

#include <stdlib.h>
#include <stdint.h>

#include "atomic.h"

#include "shmem.h"

#if defined(__i386) || defined(__x86_64)
/* REP NOP (PAUSE) is a good thing to insert into busy-wait loops. */
extern inline void rep_nop(void)
{
  __asm__ __volatile__("rep;nop": : :"memory");
}
#define PAUSE() rep_nop()
#else
#define PAUSE()
#endif

/*
 * Implement the CRAY SHMEM locking API using MCS locks
 * 
 * Mellor-Crummey & Scott, Algorithms for scalable synchronisation on
 * shared-memory multiprocessors ACM Trans. Computer Systems, 1991
 *
 * With CRAY SHMEM locks we are given an 8-byte global symmetric
 * object. This memory is pre-initialised to zero in all processes.
 *
 * We split this lock memory into two 32-bit halves where each half
 * then represents a SHMEM_LOCK.  The SHMEM_LOCK struct consists of a
 * 16-bit boolean flag (locked) and a 16-bit vp (next)
 *
 * One vp is chosen to the global lock owner process and here the 1st
 * SHMEM_LOCK acts as the 'tail' of a globally distributed linked
 * list.  In all processes the 2nd SHMEM_LOCK is used to hold and
 * manage the distributed linked list state.
 */

typedef struct {
  union {
    struct {
      volatile uint16_t locked;    /* boolean to indicate current state of lock */
      volatile int16_t  next;      /* vp of next requestor */
    } s;
    volatile uint32_t word;
  } u; 
#define l_locked        u.s.locked
#define l_next          u.s.next
#define l_word          u.word
} SHMEM_LOCK;

#define _SHMEM_LOCK_FREE  -1
#define _SHMEM_LOCK_RESET  0
#define _SHMEM_LOCK_SET    1

/* Macro to map lock virtual address to owning process vp */
#define LOCK_OWNER(LOCK)	(((uintptr_t)(LOCK) >> 3) % (shmem_n_pes()))

static
void
mcs_lock_acquire(SHMEM_LOCK *node, SHMEM_LOCK *lock, long this_pe)
{
  SHMEM_LOCK tmp;
  long locked, prev_pe;
    
  node->l_next = _SHMEM_LOCK_FREE;

  LOAD_STORE_FENCE();

  /* Form our lock request (integer) */
  tmp.l_locked = 1;
  tmp.l_next = this_pe;

  /* Swap this_pe into the global lock owner, returning previous value, atomically */
  tmp.l_word = shmem_int_swap((int *)&lock->l_word, tmp.l_word, LOCK_OWNER(lock));

  /* Translate old (broken) default lock state */
  if (tmp.l_word == _SHMEM_LOCK_FREE)
    tmp.l_word = _SHMEM_LOCK_RESET;

  /* Extract the global lock (tail) state */
  prev_pe = tmp.l_next;
  locked = tmp.l_locked;

  /* Is the lock held by someone else ? */
  if (locked) 
    {
      /* This flag gets cleared (remotely) once the lock is dropped */
      node->l_locked = 1;
	
      LOAD_STORE_FENCE();

      /* I'm now next in global linked list, update l_next in the prev_pe process with our vp */
      shmem_short_p((short *)&node->l_next, this_pe, prev_pe);
      
      /* Wait for flag to be released */
      do { PAUSE(); } while (node->l_locked);
    }
}

static
void mcs_lock_release (SHMEM_LOCK *node, SHMEM_LOCK *lock, long this_pe)
{
  /* Is there someone on the linked list ? */
  if (node->l_next == _SHMEM_LOCK_FREE) 
    {
      SHMEM_LOCK tmp;

      /* Form the remote atomic compare value (int) */
      tmp.l_locked = 1;
      tmp.l_next = this_pe;

      /* If global lock owner value still equals this_pe, load RESET into it & return prev value */ 
      tmp.l_word = shmem_int_cswap((int *)&lock->l_word, tmp.l_word, _SHMEM_LOCK_RESET, LOCK_OWNER(lock));
      if (tmp.l_next == this_pe)
	/* We were still the only requestor, all done */
	return;
      
      /* Somebody is about to chain themself off us, wait for them to do it */
      /* We have seen l_next being written as two individual bytes here when when
       * the usercopy device is active, poll for in being valid as well as it being
       * set to ensure both bytes are written before we try and use it's value below.
       */
      do { PAUSE(); } while ((node->l_next == _SHMEM_LOCK_FREE)||(node->l_next <0));
    }
   
  /* Be more strict about the test above, this memory consistiancy problem is a tricky one */
  while ( node->l_next < 0 )
    LOAD_STORE_FENCE();
    
  /*
   * Release any waiters on the linked list
   */

  /* Write 0 into the locked flag on PE<l_next> */
  shmem_short_p((short *)&node->l_locked, 0, node->l_next);
}


/*
 * I am not sure this is strictly correct. The Cray man pages suggest
 * that this routine should not block. With this implementation we could
 * race with another PE doing the same and then block in the acquire
 * Perhaps a conditional swap at the beginning would fix it ??
 *
 * (addy 12.10.05)
 */
static
int mcs_lock_test (SHMEM_LOCK *node, SHMEM_LOCK *lock, long this_pe)
{  
  SHMEM_LOCK tmp;

  /* Read the remote global lock value */
  tmp.l_word = shmem_int_g((int *)&lock->l_word, LOCK_OWNER(lock));

  /* Translate old (broken) default lock state */
  if (tmp.l_word == _SHMEM_LOCK_FREE)
    tmp.l_word = _SHMEM_LOCK_RESET;

  /* If lock already set then return 1, otherwise grab the lock & return 0 */
  if (tmp.l_word == _SHMEM_LOCK_RESET) {
    mcs_lock_acquire(node, lock, this_pe);
    return 0;
  }
  else {
    return 1;
  }
}

void
pshmem_set_lock (long *lock)
{
  mcs_lock_acquire (&((SHMEM_LOCK *)lock)[1], &((SHMEM_LOCK *)lock)[0], shmem_my_pe());
}
#pragma weak shmem_set_lock = pshmem_set_lock

void
pshmem_clear_lock (long *lock)
{
  /* The Cray man pages suggest we also need to do this (addy 12.10.05) */
  shmem_quiet();

  mcs_lock_release (&((SHMEM_LOCK *)lock)[1], &((SHMEM_LOCK *)lock)[0], shmem_my_pe());
}
#pragma weak shmem_clear_lock = pshmem_clear_lock

int
pshmem_test_lock (long *lock)
{
  return mcs_lock_test (&((SHMEM_LOCK *)lock)[1], &((SHMEM_LOCK *)lock)[0], shmem_my_pe());
}
#pragma weak shmem_test_lock = pshmem_test_lock
