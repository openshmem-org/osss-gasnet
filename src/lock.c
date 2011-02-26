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
#include "trace.h"
#include "comms.h"

#include "shmem.h"

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
#define LOCK_OWNER(LOCK)	(((uintptr_t)(LOCK) >> 3) % (_num_pes()))

#if 0
static
void
dump_shmem_lock(char *name, SHMEM_LOCK *L)
{
  __shmem_trace(SHMEM_LOG_LOCK,
		"%s: l_locked=%d, l_next=%d",
		name,
		L->l_locked,
		L->l_next
		);
}
#endif

static
void
mcs_lock_acquire(SHMEM_LOCK *node, SHMEM_LOCK *lock, long this_pe)
{
  SHMEM_LOCK tmp;
  long locked, prev_pe;

  node->l_next = _SHMEM_LOCK_FREE;

  /* Form our lock request (integer) */
  tmp.l_locked = 1;
  tmp.l_next = this_pe;

  LOAD_STORE_FENCE();

  /*
   * Swap this_pe into the global lock owner, returning previous
   * value, atomically
   */
  tmp.l_word = shmem_int_swap((int *)&lock->l_word, tmp.l_word, LOCK_OWNER(lock));

  /* Translate old (broken) default lock state */
  if (tmp.l_word == _SHMEM_LOCK_FREE) {
    tmp.l_word = _SHMEM_LOCK_RESET;
  }

  /* Extract the global lock (tail) state */
  prev_pe = tmp.l_next;
  locked = tmp.l_locked;

  /* Is the lock held by someone else ? */
  if (locked) 
    {
      /*
       * This flag gets cleared (remotely) once the lock is dropped
       */
      node->l_locked = 1;
	
      LOAD_STORE_FENCE();

      /*
       * I'm now next in global linked list, update l_next in the
       * prev_pe process with our vp
       */
      shmem_short_p((short *)&node->l_next, this_pe, prev_pe);

      /* Wait for flag to be released */
      do {
	__comms_pause();
      } while (node->l_locked);

    }
}

static
void
mcs_lock_release(SHMEM_LOCK *node, SHMEM_LOCK *lock, long this_pe)
{
  /* Is there someone on the linked list ? */
  if (node->l_next == _SHMEM_LOCK_FREE) {
    SHMEM_LOCK tmp;

    /* Form the remote atomic compare value (int) */
    tmp.l_locked = 1;
    tmp.l_next = this_pe;

    /*
     * If global lock owner value still equals this_pe, load RESET
     * into it & return prev value
     */
#if 0
    __shmem_trace(SHMEM_LOG_LOCK,
		  "cswap(%p, %d = %d + %d, %d, %d)",
		  (int *)&lock->l_word,
		  tmp.l_word,
		  tmp.l_locked, tmp.l_next,
		  _SHMEM_LOCK_RESET,
		  LOCK_OWNER(lock)
		  );
#endif
    tmp.l_word = shmem_int_cswap((int *)&lock->l_word,
				 tmp.l_word,
				 _SHMEM_LOCK_RESET,
				 LOCK_OWNER(lock)
				 );

    if (tmp.l_next == this_pe) {
      /* We were still the only requestor, all done */
      return;
    }
      
    /*
     * Somebody is about to chain themself off us, wait for them to do it.
     *
     * Quadrics: we have seen l_next being written as two individual
     * bytes here when when the usercopy device is active, poll for
     * it being valid as well as it being set to ensure both bytes
     * are written before we try to use its value below.
     *       
     */
    do {
      __comms_pause();
    } while ( (node->l_next == _SHMEM_LOCK_FREE) || (node->l_next < 0) );
  }

  /* Be more strict about the test above,
   * this memory consistency problem is a tricky one
   *
   * ("while" instead of "do...while" because the condition
   * could easily be true already)
   */

  while ( node->l_next < 0 ) {
    __comms_pause();
  }
    
  /*
   * Release any waiters on the linked list
   */

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
int
mcs_lock_test(SHMEM_LOCK *node, SHMEM_LOCK *lock, long this_pe)
{  
  SHMEM_LOCK tmp;
  int retval;

  /* Read the remote global lock value */
  tmp.l_word = shmem_int_g((int *)&lock->l_word, LOCK_OWNER(lock));

  /* Translate old (broken) default lock state */
  if (tmp.l_word == _SHMEM_LOCK_FREE)
    tmp.l_word = _SHMEM_LOCK_RESET;

  /* If lock already set then return 1, otherwise grab the lock & return 0 */
  if (tmp.l_word == _SHMEM_LOCK_RESET) {
    mcs_lock_acquire(node, lock, this_pe);
    retval = 0;
  }
  else {
    retval = 1;
  }
  
  return retval;
}

/*
 * ------------------------------------------------------------
 */

/* @api@ */
void
pshmem_set_lock(long *lock)
{
  mcs_lock_acquire(&((SHMEM_LOCK *)lock)[1],
		   &((SHMEM_LOCK *)lock)[0],
		   _my_pe()
		   );
}

/* @api@ */
void
pshmem_clear_lock(long *lock)
{
  /* The Cray man pages suggest we also need to do this (addy 12.10.05) */
  shmem_quiet();

  mcs_lock_release(&((SHMEM_LOCK *)lock)[1],
		   &((SHMEM_LOCK *)lock)[0],
		   _my_pe()
		   );
}

/* @api@ */
int
pshmem_test_lock(long *lock)
{
  return mcs_lock_test(&((SHMEM_LOCK *)lock)[1],
		       &((SHMEM_LOCK *)lock)[0],
		       _my_pe()
		       );
}

#pragma weak shmem_set_lock = pshmem_set_lock
#pragma weak shmem_clear_lock = pshmem_clear_lock
#pragma weak shmem_test_lock = pshmem_test_lock
