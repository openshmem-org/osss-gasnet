/*
 *
 * Copyright (c) 2011 - 2013
 *   University of Houston System and Oak Ridge National Laboratory.
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * o Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * 
 * o Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 
 * o Neither the name of the University of Houston System, Oak Ridge
 *   National Laboratory nor the names of its contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */



/**
 * This file provides the layer on top of GASNet, ARMCI or whatever.
 * API should be formalized at some point, but basically everything
 * non-static that starts with "__shmem_comms_"
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

#include <gasnet.h>

#include "uthash.h"

#include "state.h"
#include "memalloc.h"
#include "trace.h"
#include "atomic.h"
#include "ping.h"
#include "utils.h"
#include "exe.h"
#include "globalvar.h"

#include "service.h"

#include "unitparse.h"

#include "../comms.h"

/**
 * gasnet put model: this is just for testing different put
 * emulations; generally we want the nbi routines to get performance.
 *
 */

#define USING_IMPLICIT_HANDLES 1

#if USING_IMPLICIT_HANDLES
# define GASNET_PUT(pe, dst, src, len)      gasnet_put_nbi (pe, dst, src, len)
# define GASNET_PUT_BULK(pe, dst, src, len) gasnet_put_nbi_bulk (pe, dst, src, len)
# define GASNET_PUT_VAL(pe, dst, src, len)  gasnet_put_nbi_val (pe, dst, src, len)
# define GASNET_WAIT_PUTS()                 gasnet_wait_syncnbi_puts ()
# define GASNET_WAIT_ALL()                  gasnet_wait_syncnbi_all ()
#else
# define GASNET_PUT(pe, dst, src, len)      gasnet_put (pe, dst, src, len)
# define GASNET_PUT_BULK(pe, dst, src, len) gasnet_put_bulk (pe, dst, src, len)
# define GASNET_PUT_VAL(pe, dst, src, len)  gasnet_put_val (pe, dst, src, len)
# define GASNET_WAIT_PUTS()
# define GASNET_WAIT_ALL()
#endif /* USING_IMPLICIT_HANDLES */

#define GASNET_GET(pe, dst, src, len)      gasnet_get (pe, dst, src, len)
#define GASNET_GET_BULK(pe, dst, src, len) gasnet_get_bulk (pe, dst, src, len)

/**
 * gasnet model choice
 *
 */

#if defined(GASNET_SEGMENT_FAST)
# define HAVE_MANAGED_SEGMENTS 1
#elif defined(GASNET_SEGMENT_LARGE)
# define HAVE_MANAGED_SEGMENTS 1
#elif defined(GASNET_SEGMENT_EVERYTHING)
# undef HAVE_MANAGED_SEGMENTS
#else
# error "I don't know what kind of GASNet segment model you're trying to use"
#endif

/**
 * set up segment/symmetric handling
 *
 */

static gasnet_seginfo_t *seginfo_table;

#if ! defined(HAVE_MANAGED_SEGMENTS)

/**
 * this will be malloc'ed so we can respect setting from environment
 * variable
 */

#define DEFAULT_HEAP_SIZE 2000000000L	/* 2G */

static void *great_big_heap;

#endif /* ! HAVE_MANAGED_SEGMENTS */

/**
 * trap gasnet errors gracefully
 *
 */
#define GASNET_SAFE(fncall) \
  do {									\
    const int _retval = fncall ;					\
    if (_retval != GASNET_OK)						\
      {									\
	__shmem_trace (SHMEM_LOG_FATAL,					\
		       "error calling: %s at %s:%i, %s (%s)\n",		\
		       #fncall, __FILE__, __LINE__,			\
		       gasnet_ErrorName (_retval),			\
		       gasnet_ErrorDesc (_retval)			\
		       );						\
      }									\
  } while(0)

/*
 * --------------------------------------------------------------
 *
 * GASNet allows applications to use handler codes 128-255.
 *
 * See http://gasnet.cs.berkeley.edu/dist/docs/gasnet.html, under
 * description of gasnet_attach ()
 */

enum
  {
    GASNET_HANDLER_SETUP_OUT = 128,
    GASNET_HANDLER_SETUP_BAK,
    GASNET_HANDLER_SWAP_OUT,
    GASNET_HANDLER_SWAP_BAK,
    GASNET_HANDLER_CSWAP_OUT,
    GASNET_HANDLER_CSWAP_BAK,
    GASNET_HANDLER_FADD_OUT,
    GASNET_HANDLER_FADD_BAK,
    GASNET_HANDLER_FINC_OUT,
    GASNET_HANDLER_FINC_BAK,
    GASNET_HANDLER_ADD_OUT,
    GASNET_HANDLER_ADD_BAK,
    GASNET_HANDLER_INC_OUT,
    GASNET_HANDLER_INC_BAK,
    GASNET_HANDLER_PING_OUT,
    GASNET_HANDLER_PING_BAK,
    GASNET_HANDLER_XOR_OUT,
    GASNET_HANDLER_XOR_BAK,
    GASNET_HANDLER_GLOBALVAR_PUT_OUT,
    GASNET_HANDLER_GLOBALVAR_PUT_BAK,
    GASNET_HANDLER_GLOBALVAR_GET_OUT,
    GASNET_HANDLER_GLOBALVAR_GET_BAK,
  };

/**
 * --------------- real work starts here ---------------------
 *
 */

/**
 * work out how big the symmetric segment areas should be.
 *
 * Either from environment setting, or default value from
 * implementation
 */
static size_t
__shmem_comms_get_segment_size (void)
{
  char *mlss_str = __shmem_comms_getenv ("SHMEM_SYMMETRIC_HEAP_SIZE");
  size_t retval;

  if (mlss_str == (char *) NULL)
    {
#ifdef HAVE_MANAGED_SEGMENTS
      return (size_t) gasnet_getMaxLocalSegmentSize ();
#else
      return DEFAULT_HEAP_SIZE;
#endif
    }

  retval = __shmem_parse_size (mlss_str);
  if (retval == (size_t) -1)
    {
      /* don't know that unit! */
      __shmem_trace (SHMEM_LOG_FATAL,
		     "unknown data size unit in symmetric heap specification"
		     );
      /* NOT REACHED */
    }

  return retval;
}

/**
 * allow the runtime to change the spin/block behavior dynamically,
 * would allow adaptivity
 */
void
__shmem_comms_set_waitmode (comms_spinmode_t mode)
{
  int gm;
  const char *mstr;

  switch (mode)
    {
    case SHMEM_COMMS_SPINBLOCK:
      gm = GASNET_WAIT_SPINBLOCK;
      mstr = "spinblock";
      break;
    case SHMEM_COMMS_SPIN:
      gm = GASNET_WAIT_SPIN;
      mstr = "spin";
      break;
    case SHMEM_COMMS_BLOCK:
      gm = GASNET_WAIT_BLOCK;
      mstr = "block";
      break;
    default:
      __shmem_trace (SHMEM_LOG_FATAL,
		     "tried to set unknown wait mode %d", (int) mode);
      /* NOT REACHED */
      break;
    }

  GASNET_SAFE (gasnet_set_waitmode (gm));

  __shmem_trace (SHMEM_LOG_DEBUG, "set waitmode to %s", mstr);
}

/**
 * traffic progress
 *
 */
void
__shmem_comms_service (void)
{
  GASNET_SAFE (gasnet_AMPoll ());
}

/**
 * can't just call getenv, it might not pass through environment
 * info to other nodes from launch.
 */
char *
__shmem_comms_getenv (const char *name)
{
  return gasnet_getenv (name);
}

/**
 * which node (PE) am I?
 */
int
__shmem_comms_mynode (void)
{
  return (int) gasnet_mynode ();
}

/**
 * how many nodes (PEs) take part in this program?
 */
int
__shmem_comms_nodes (void)
{
  return (int) gasnet_nodes ();
}

/**
 * ---------------------------------------------------------------------------
 *
 * global barrier done through gasnet
 *
 */

static long barcount = 0;
static int barflag = 0;

void
__shmem_comms_barrier_all (void)
{
  /* GASNET_BEGIN_FUNCTION(); */

  /* use gasnet's global barrier */
  gasnet_barrier_notify (barcount, barflag);
  GASNET_SAFE (gasnet_barrier_wait (barcount, barflag));

  barcount += 1;
}

/**
 * ---------------------------------------------------------------------------
 *
 * initialize the symmetric segments.
 *
 * In the gasnet fast/large models, use the attached segments and
 * manage address translations through the segment table
 *
 * In the everything model, we allocate on our own heap and send out
 * the addresses with active messages
 */

#if ! defined(HAVE_MANAGED_SEGMENTS)

/**
 * remotely modified, stop it being put in a register
 */
static volatile int seg_setup_replies_received = 0;

static gasnet_hsl_t setup_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t setup_bak_lock = GASNET_HSL_INITIALIZER;

/**
 * unpack buf from sender PE and store seg info locally.  Ack. receipt.
 */
static void
handler_segsetup_out (gasnet_token_t token,
		      void *buf, size_t bufsiz)
{
  gasnet_node_t src_pe;
  gasnet_seginfo_t *gsp = (gasnet_seginfo_t *) buf;

  /*
   * no lock here: each PE writes exactly once to its own array index,
   * and only to that...
   */

  /* gasnet_hsl_lock(& setup_out_lock); */

  GASNET_SAFE (gasnet_AMGetMsgSource (token, &src_pe));

  seginfo_table[(int) src_pe].addr = gsp->addr;
  seginfo_table[(int) src_pe].size = gsp->size;

  /* gasnet_hsl_unlock(& setup_out_lock); */

  gasnet_AMReplyMedium0 (token, GASNET_HANDLER_SETUP_BAK,
			 (void *) NULL, 0
			 );
}

/**
 * record receipt ack.  We only need to count the number of replies
 */
static void
handler_segsetup_bak (gasnet_token_t token,
		      void *buf, size_t bufsiz)
{
  gasnet_hsl_lock (&setup_bak_lock);

  seg_setup_replies_received += 1;

  gasnet_hsl_unlock (&setup_bak_lock);
}

#endif /* ! HAVE_MANAGED_SEGMENTS */

/**
 * initialize the symmetric memory, taking into account the different
 * gasnet configurations
 */

void
__shmem_symmetric_memory_init (void)
{
  const int me = GET_STATE (mype);
  const int npes = GET_STATE (numpes);
  const size_t heapsize = GET_STATE (heapsize);
  int pm_r;

  /*
   * calloc zeroes for us
   */
  seginfo_table =
    (gasnet_seginfo_t *) calloc (npes, sizeof (gasnet_seginfo_t));
  if (seginfo_table == (gasnet_seginfo_t *) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "could not allocate GASNet segments (%s)",
		     strerror (errno)
		     );
      /* NOT REACHED */
    }

  /*
   * prep the segments for use across all PEs
   *
   */

#ifdef HAVE_MANAGED_SEGMENTS

  /* gasnet handles the segment allocation for us */
  GASNET_SAFE (gasnet_getSegmentInfo (seginfo_table, npes));

#else

  /* allocate the heap - has to be pagesize aligned */
  pm_r = posix_memalign (&great_big_heap, GASNET_PAGESIZE, heapsize);
  if (pm_r != 0)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "unable to allocate symmetric heap (%s)",
		     strerror (pm_r)
		     );
      /* NOT REACHED */
    }

  /* everyone has their local info before exchanging messages */
  __shmem_comms_barrier_all ();

  __shmem_trace (SHMEM_LOG_MEMORY,
		 "symmetric heap @ %p, size is %ld bytes",
		 great_big_heap, heapsize
		 );

  /* store my own heap entry */
  seginfo_table[me].addr = great_big_heap;
  seginfo_table[me].size = heapsize;

  {
    gasnet_seginfo_t gs;
    int pe;

    gs.addr = great_big_heap;
    gs.size = heapsize;

    for (pe = 0; pe < npes; pe += 1)
      {
	/* send to everyone else */
	if (me != pe)
	  {
	    gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_SETUP_OUT,
				     &gs, sizeof (gs)
				     );
	  }
      }

    /* now wait on the AM replies (0-based AND don't count myself) */
    GASNET_BLOCKUNTIL (seg_setup_replies_received == npes - 1);

    __shmem_trace (SHMEM_LOG_MEMORY,
		   "received all replies (%d)", seg_setup_replies_received
		   );

  }

#endif /* HAVE_MANAGED_SEGMENTS */

  /* initialize my heap */
  __shmem_mem_init (seginfo_table[me].addr, seginfo_table[me].size);

  /* and make sure everyone is up-to-speed */
  __shmem_comms_barrier_all ();

  /*
   * spit out the seginfo table (but check first that the loop is
   * warranted)
   */
  if (__shmem_trace_is_enabled (SHMEM_LOG_INIT))
    {
      int pe;
      for (pe = 0; pe < npes; pe += 1)
	{
	  __shmem_trace (SHMEM_LOG_INIT,
			 "cross-check: segment[%d] = { .addr = %p, .size = %ld }",
			 pe, seginfo_table[pe].addr, seginfo_table[pe].size
			 );
	}
    }
}

/**
 * shut down the memory allocation handler
 */
void
__shmem_symmetric_memory_finalize (void)
{
  __shmem_mem_finalize ();
#if ! defined(HAVE_MANAGED_SEGMENTS)
  free (great_big_heap);
#endif /* HAVE_MANAGED_SEGMENTS */
}

/**
 * where the symmetric memory starts on the given PE
 */
void *
__shmem_symmetric_var_base (int p)
{
  return seginfo_table[p].addr;
}

/**
 * is the address in the managed symmetric area?
 */
int
__shmem_symmetric_var_in_range (void *addr, int pe)
{
  int retval;

  if (addr < seginfo_table[pe].addr)
    {
      __shmem_trace (SHMEM_LOG_MEMORY,
		     "addr %p < seginfo %p", addr, seginfo_table[pe].addr
		     );
      retval = 0;
    }
  else if (addr > (seginfo_table[pe].addr + seginfo_table[pe].size))
    {
      __shmem_trace (SHMEM_LOG_MEMORY,
		     "addr %p > seginfo + size %p",
		     addr, seginfo_table[pe].addr + seginfo_table[pe].size
		     );
      retval = 0;
    }
  else
    {
      retval = 1;
    }

  return retval;
}

/**
 * translate my "dest" to corresponding address on PE "pe"
 */
void *
__shmem_symmetric_addr_lookup (void *dest, int pe)
{
  const int me = GET_STATE (mype);
  size_t offset;
  char *rdest;

  /* globals are in same place everywhere */
  if (__shmem_symmetric_is_globalvar (dest))
    {
      return dest;
    }

  /* not symmetric if outside of heap */
  if (! __shmem_symmetric_var_in_range (dest, me))
    {
      return NULL;
    }

  /* where this is in *my* heap */
  offset = (char *) dest - (char *) __shmem_symmetric_var_base (me);
  /* and where it is in the remote heap */
  rdest = (char *) __shmem_symmetric_var_base (pe) + offset;

  /* assume this is good */
  return rdest;
}

/**
 * -- lock finding/creating utility --
 */

typedef struct
{
  void *addr;
  gasnet_hsl_t *lock;
  UT_hash_handle hh;		/* makes this structure hashable */
} lock_table_t;

static lock_table_t *lock_table = NULL;

/**
 * Look up the lock for a given address ADDR.  If ADDR has never been
 * seen before, create the lock for it.
 *
 */
static gasnet_hsl_t *
get_lock_for (void *addr)
{
  lock_table_t *try;

  HASH_FIND_PTR (lock_table, &addr, try);

  if (try == (lock_table_t *) NULL)
    {
      gasnet_hsl_t *L = (gasnet_hsl_t *) malloc (sizeof (*L));

      if (L == (gasnet_hsl_t *) NULL)
	{
	  __shmem_trace (SHMEM_LOG_FATAL,
			 "internal error: unable to allocate lock for address %p",
			 addr);
	  /* NOT REACHED */
	}

      try = (lock_table_t *) malloc (sizeof (*try));
      if (try == (lock_table_t *) NULL)
	{
	  __shmem_trace (SHMEM_LOG_FATAL,
			 "internal error: unable to allocate lock table entry for address %p",
			 addr
			 );
	  /* NOT REACHED */
	}

      gasnet_hsl_init (L);

      try->addr = addr;
      try->lock = L;

      HASH_ADD_PTR (lock_table, addr, try);

      __shmem_trace (SHMEM_LOG_LOCK,
		     "created new lock for address %p",
		     addr
		     );
    }
  else
    {
      __shmem_trace (SHMEM_LOG_LOCK,
		     "already have a lock for address %p",
		     addr
		     );
    }

  return try->lock;
}

/**
 * -- swap handlers ---------------------------------------------------------
 */

/**
 * NB we make the cond/value "long long" throughout
 * to be used by smaller types as self-contained payload
 */

typedef struct
{
  void *local_store;		/* sender saves here */
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be swapped */
} swap_payload_t;

/**
 * called by remote PE to do the swap.  Store new value, send back old value
 */
static void
handler_swap_out (gasnet_token_t token,
		  void *buf, size_t bufsiz)
{
  long long old;
  swap_payload_t *pp = (swap_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for (pp->r_symm_addr);

  gasnet_hsl_lock (lk);

  /* save and update */
  (void) memmove (&old, pp->r_symm_addr, pp->nbytes);
  (void) memmove (pp->r_symm_addr, &(pp->value), pp->nbytes);
  pp->value = old;

  LOAD_STORE_FENCE;

  gasnet_hsl_unlock (lk);

  /* return updated payload */
  gasnet_AMReplyMedium0 (token, GASNET_HANDLER_SWAP_BAK, buf, bufsiz);
}

/**
 * called by swap invoker when old value returned by remote PE
 */
static void
handler_swap_bak (gasnet_token_t token,
		  void *buf, size_t bufsiz)
{
  swap_payload_t *pp = (swap_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for (pp->r_symm_addr);

  gasnet_hsl_lock (lk);

  /* save returned value */
  (void) memmove (pp->local_store, &(pp->value), pp->nbytes);

  LOAD_STORE_FENCE;

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock (lk);
}

/**
 * perform the swap
 */
void
__shmem_comms_swap_request (void *target, void *value, size_t nbytes,
			    int pe, void *retval)
{
  swap_payload_t *p = (swap_payload_t *) malloc (sizeof (*p));
  if (p == (swap_payload_t *) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: unable to allocate swap payload memory"
		     );
    }
  /* build payload to send */
  p->local_store = retval;
  p->r_symm_addr = __shmem_symmetric_addr_lookup (target, pe);
  p->nbytes = nbytes;
  p->value = *(long long *) value;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* fire off request */
  gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_SWAP_OUT, p, sizeof (*p));

  /*
   * TODO:
   *
   * This, as in all the atomic handlers, is where the opportunity gap
   * is.  We could do useful things between firing off the request and
   * waiting for the completion notification.  So split this out into
   * a post and wait/poll pair, post returning a handle for the atomic
   * op. in progress.
   */

  WAIT_ON_COMPLETION (p->completed);

  free (p);
}

typedef struct
{
  void *local_store;		/* sender saves here */
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be swapped */
  long long cond;		/* conditional value */
} cswap_payload_t;

/**
 * called by remote PE to do the swap.  Store new value if cond
 * matches, send back old value in either case
 */
static void
handler_cswap_out (gasnet_token_t token,
		   void *buf, size_t bufsiz)
{
  void *old;
  cswap_payload_t *pp = (cswap_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for (pp->r_symm_addr);

  gasnet_hsl_lock (lk);

  old = malloc (pp->nbytes);
  if (old == NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: unable to allocate cswap save space"
		     );
    }

  /* save current target */
  memmove (old, pp->r_symm_addr, pp->nbytes);

  /* update value if cond matches */
  if (memcmp (&(pp->cond), pp->r_symm_addr, pp->nbytes) == 0)
    {
      memmove (pp->r_symm_addr, &(pp->value), pp->nbytes);
    }
  /* return value */
  memmove (&(pp->value), old, pp->nbytes);

  LOAD_STORE_FENCE;

  free (old);

  gasnet_hsl_unlock (lk);

  /* return updated payload */
  gasnet_AMReplyMedium0 (token, GASNET_HANDLER_CSWAP_BAK, buf, bufsiz);
}

/**
 * called by swap invoker when old value returned by remote PE
 * (same as swap_bak for now)
 */
static void
handler_cswap_bak (gasnet_token_t token,
		   void *buf, size_t bufsiz)
{
  cswap_payload_t *pp = (cswap_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for (pp->r_symm_addr);

  gasnet_hsl_lock (lk);

  /* save returned value */
  (void) memmove (pp->local_store, &(pp->value), pp->nbytes);

  LOAD_STORE_FENCE;

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock (lk);
}

/**
 * perform the conditional swap
 */
void
__shmem_comms_cswap_request (void *target, void *cond, void *value,
			     size_t nbytes, int pe, void *retval)
{
  cswap_payload_t *cp = (cswap_payload_t *) malloc (sizeof (*cp));
  if (cp == (cswap_payload_t *) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: unable to allocate conditional swap payload memory"
		     );
    }
  /* build payload to send */
  cp->local_store = retval;
  cp->r_symm_addr = __shmem_symmetric_addr_lookup (target, pe);
  cp->nbytes = nbytes;
  cp->value = cp->cond = 0LL;
  memmove (&(cp->value), value, nbytes);
  memmove (&(cp->cond), cond, nbytes);
  cp->completed = 0;
  cp->completed_addr = &(cp->completed);

  LOAD_STORE_FENCE;

  /* fire off request */
  gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_CSWAP_OUT, cp, sizeof (*cp));

  WAIT_ON_COMPLETION (cp->completed);

  free (cp);
}

/**
 * fetch/add
 */

typedef struct
{
  void *local_store;		/* sender saves here */
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be added & then return old */
} fadd_payload_t;

/**
 * called by remote PE to do the fetch and add.  Store new value, send
 * back old value
 */
static void
handler_fadd_out (gasnet_token_t token,
		  void *buf, size_t bufsiz)
{
  long long old = 0;
  long long plus;
  fadd_payload_t *pp = (fadd_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for (pp->r_symm_addr);

  gasnet_hsl_lock (lk);

  /* save and update */
  (void) memmove (&old, pp->r_symm_addr, pp->nbytes);
  plus = old + pp->value;
  (void) memmove (pp->r_symm_addr, &plus, pp->nbytes);
  pp->value = old;

  LOAD_STORE_FENCE;

  gasnet_hsl_unlock (lk);

  /* return updated payload */
  gasnet_AMReplyMedium0 (token, GASNET_HANDLER_FADD_BAK, buf, bufsiz);
}

/**
 * called by fadd invoker when old value returned by remote PE
 */
static void
handler_fadd_bak (gasnet_token_t token,
		  void *buf, size_t bufsiz)
{
  fadd_payload_t *pp = (fadd_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for (pp->r_symm_addr);

  gasnet_hsl_lock (lk);

  /* save returned value */
  (void) memmove (pp->local_store, &(pp->value), pp->nbytes);

  LOAD_STORE_FENCE;

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock (lk);
}

/**
 * perform the fetch-and-add
 */
void
__shmem_comms_fadd_request (void *target, void *value, size_t nbytes, int pe,
			    void *retval)
{
  fadd_payload_t *p = (fadd_payload_t *) malloc (sizeof (*p));
  if (p == (fadd_payload_t *) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: unable to allocate fetch-and-add payload memory"
		     );
    }
  /* build payload to send */
  p->local_store = retval;
  p->r_symm_addr = __shmem_symmetric_addr_lookup (target, pe);
  p->nbytes = nbytes;
  p->value = *(long long *) value;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* fire off request */
  gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_FADD_OUT, p, sizeof (*p));

  WAIT_ON_COMPLETION (p->completed);

  free (p);
}

/**
 * fetch/increment
 */

typedef struct
{
  void *local_store;		/* sender saves here */
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be returned */
} finc_payload_t;

/**
 * called by remote PE to do the fetch and increment.  Store new
 * value, send back old value
 */
static void
handler_finc_out (gasnet_token_t token,
		  void *buf, size_t bufsiz)
{
  long long old = 0;
  long long plus;
  finc_payload_t *pp = (finc_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for (pp->r_symm_addr);

  gasnet_hsl_lock (lk);

  /* save and update */
  (void) memmove (&old, pp->r_symm_addr, pp->nbytes);
  plus = old + 1;
  (void) memmove (pp->r_symm_addr, &plus, pp->nbytes);
  pp->value = old;

  LOAD_STORE_FENCE;

  gasnet_hsl_unlock (lk);

  /* return updated payload */
  gasnet_AMReplyMedium0 (token, GASNET_HANDLER_FINC_BAK, buf, bufsiz);
}

/**
 * called by finc invoker when old value returned by remote PE
 */
static void
handler_finc_bak (gasnet_token_t token,
		  void *buf, size_t bufsiz)
{
  finc_payload_t *pp = (finc_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for (pp->r_symm_addr);

  gasnet_hsl_lock (lk);

  /* save returned value */
  (void) memmove (pp->local_store, &(pp->value), pp->nbytes);

  LOAD_STORE_FENCE;

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock (lk);
}

/**
 * perform the fetch-and-increment
 */
void
__shmem_comms_finc_request (void *target, size_t nbytes, int pe, void *retval)
{
  finc_payload_t *p = (finc_payload_t *) malloc (sizeof (*p));
  if (p == (finc_payload_t *) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: unable to allocate fetch-and-increment payload memory"
		     );
    }
  /* build payload to send */
  p->local_store = retval;
  p->r_symm_addr = __shmem_symmetric_addr_lookup (target, pe);
  p->nbytes = nbytes;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* fire off request */
  gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_FINC_OUT, p, sizeof (*p));

  WAIT_ON_COMPLETION (p->completed);

  free (p);
}

/**
 * remote add
 */

typedef struct
{
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be returned */
} add_payload_t;

/**
 * called by remote PE to do the remote add.
 */
static void
handler_add_out (gasnet_token_t token,
		 void *buf, size_t bufsiz)
{
  long long old = 0;
  long long plus;
  add_payload_t *pp = (add_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for (pp->r_symm_addr);

  gasnet_hsl_lock (lk);

  /* save and update */
  (void) memmove (&old, pp->r_symm_addr, pp->nbytes);
  plus = old + pp->value;
  (void) memmove (pp->r_symm_addr, &plus, pp->nbytes);

  LOAD_STORE_FENCE;

  gasnet_hsl_unlock (lk);

  /* return updated payload */
  gasnet_AMReplyMedium0 (token, GASNET_HANDLER_ADD_BAK, buf, bufsiz);
}

/**
 * called by remote add invoker when store done
 */
static void
handler_add_bak (gasnet_token_t token,
		 void *buf, size_t bufsiz)
{
  add_payload_t *pp = (add_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for (pp->r_symm_addr);

  gasnet_hsl_lock (lk);

  LOAD_STORE_FENCE;

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock (lk);
}

/**
 * perform the add
 */
void
__shmem_comms_add_request (void *target, void *value, size_t nbytes, int pe)
{
  add_payload_t *p = (add_payload_t *) malloc (sizeof (*p));
  if (p == (add_payload_t *) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: unable to allocate remote add payload memory"
		     );
    }
  /* build payload to send */
  p->r_symm_addr = __shmem_symmetric_addr_lookup (target, pe);
  p->nbytes = nbytes;
  p->value = *(long long *) value;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* fire off request */
  gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_ADD_OUT, p, sizeof (*p));

  WAIT_ON_COMPLETION (p->completed);

  free (p);
}

/**
 * remote increment
 */

typedef struct
{
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
  size_t nbytes;		/* how big the value is */
} inc_payload_t;

/**
 * called by remote PE to do the remote increment
 */
static void
handler_inc_out (gasnet_token_t token,
		 void *buf, size_t bufsiz)
{
  long long old = 0;
  long long plus;
  inc_payload_t *pp = (inc_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for (pp->r_symm_addr);

  gasnet_hsl_lock (lk);

  /* save and update */
  (void) memmove (&old, pp->r_symm_addr, pp->nbytes);
  plus = old + 1;
  (void) memmove (pp->r_symm_addr, &plus, pp->nbytes);
  LOAD_STORE_FENCE;

  __shmem_trace (SHMEM_LOG_ATOMIC, "%lld -> %lld", old, plus);

  gasnet_hsl_unlock (lk);

  /* return updated payload */
  gasnet_AMReplyMedium0 (token, GASNET_HANDLER_INC_BAK, buf, bufsiz);
}

/**
 * called by remote increment invoker when store done
 */
static void
handler_inc_bak (gasnet_token_t token,
		 void *buf, size_t bufsiz)
{
  inc_payload_t *pp = (inc_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for (pp->r_symm_addr);

  gasnet_hsl_lock (lk);

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock (lk);
}

/**
 * perform the increment
 */
void
__shmem_comms_inc_request (void *target, size_t nbytes, int pe)
{
  inc_payload_t *p = (inc_payload_t *) malloc (sizeof (*p));
  if (p == (inc_payload_t *) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: unable to allocate remote increment payload memory"
		     );
    }
  /* build payload to send */
  p->r_symm_addr = __shmem_symmetric_addr_lookup (target, pe);
  p->nbytes = nbytes;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* fire off request */
  gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_INC_OUT, p, sizeof (*p));

  WAIT_ON_COMPLETION (p->completed);

  free (p);
}

/**
 * Proposed by IBM Zurich
 *
 * remote xor
 */

typedef struct
{
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* to xor on remote var */
} xor_payload_t;

/**
 * called by remote PE to do the remote xor
 */
static void
handler_xor_out (gasnet_token_t token,
		 void *buf, size_t bufsiz)
{
  xor_payload_t *pp = (xor_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for (pp->r_symm_addr);
  long long v;

  gasnet_hsl_lock (lk);

  /* save and update */
  v = * ((long long *)pp->r_symm_addr);
  v ^= pp->value;
  * ((long long *)pp->r_symm_addr) = v;
  LOAD_STORE_FENCE;

  gasnet_hsl_unlock (lk);

  /* return updated payload */
  gasnet_AMReplyMedium0 (token, GASNET_HANDLER_XOR_BAK, buf, bufsiz);
}

/**
 * called by remote xor invoker when store done
 */
static void
handler_xor_bak (gasnet_token_t token,
		 void *buf, size_t bufsiz)
{
  xor_payload_t *pp = (xor_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for (pp->r_symm_addr);

  gasnet_hsl_lock (lk);

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock (lk);
}

/**
 * perform the xor
 */
void
__shmem_comms_xor_request (void *target, void *value, size_t nbytes, int pe)
{
  xor_payload_t *p = (xor_payload_t *) malloc (sizeof (*p));
  if (p == (xor_payload_t *) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: unable to allocate remote exclusive-or payload memory"
		     );
    }
  /* build payload to send */
  p->r_symm_addr = __shmem_symmetric_addr_lookup (target, pe);
  p->nbytes = nbytes;
  p->value = *(long long *) value;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* fire off request */
  gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_XOR_OUT, p, sizeof (*p));

  WAIT_ON_COMPLETION (p->completed);

  free (p);
}



/**
 * ---------------------------------------------------------------------------
 *
 * Handlers for pinging for shmem_pe_accessible
 *
 */

typedef struct
{
  pe_status_t remote_pe_status;	/* health of remote PE */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
} ping_payload_t;

static int pe_acked = 1;

static jmp_buf jb;

/**
 * do this when the timeout occurs
 */
static void
ping_timeout_handler (int signum)
{
  pe_acked = 0;

  longjmp (jb, 1);
}

/**
 * can use single static lock here, no per-addr needed
 */
static gasnet_hsl_t ping_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t ping_bak_lock = GASNET_HSL_INITIALIZER;

/**
 * called by remote PE when (if) it gets the ping
 */
static void
handler_ping_out (gasnet_token_t token,
		  void *buf, size_t bufsiz)
{
  ping_payload_t *pp = (ping_payload_t *) buf;

  gasnet_hsl_lock (&ping_out_lock);

  pp->remote_pe_status = PE_RUNNING;

  gasnet_hsl_unlock (&ping_out_lock);

  /* return ack'ed payload */
  gasnet_AMReplyMedium0 (token, GASNET_HANDLER_PING_BAK, buf, bufsiz);
}

/**
 * called by sender PE when (if) remote PE ack's the ping
 */
static void
handler_ping_bak (gasnet_token_t token,
		  void *buf, size_t bufsiz)
{
  ping_payload_t *pp = (ping_payload_t *) buf;

  gasnet_hsl_lock (&ping_bak_lock);

  pe_acked = pe_acked && (pp->remote_pe_status == PE_RUNNING);

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock (&ping_bak_lock);
}

/**
 * perform the ping
 *
 * TODO: JUST RETURN TRUE FOR NOW, NEED TO WORK ON PROGRESS LOGIC
 *
 */
int
__shmem_comms_ping_request (int pe)
{
  return 1;

#if 0
  sighandler_t sig;
  int sj_status;
  ping_payload_t *p = (ping_payload_t *) malloc (sizeof (*p));
  if (p == (ping_payload_t *) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: unable to allocate remote accessibility payload memory"
		     );
    }
  /* build payload to send */
  p->completed = 0;
  p->completed_addr = &(p->completed);
  p->remote_pe_status = PE_UNKNOWN;

  /* now the ping is ponged, or we timeout waiting... */
  sig = signal (SIGALRM, ping_timeout_handler);
  if (sig == SIG_ERR)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: registration of ping timeout handler failed"
		     );
      /* NOT REACHED */
    }

  /* hope for the best */
  pe_acked = 1;

  __shmem_ping_set_alarm ();

  sj_status = setjmp (jb);

  /* only ping if we're coming through the first time */
  if (sj_status == 0)
    {
      /* fire off request */
      gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_PING_OUT,
			       p, sizeof (*p)
			       );

      WAIT_ON_COMPLETION (p->completed);
    }

  __shmem_ping_clear_alarm ();

  sig = signal (SIGALRM, sig);
  if (sig == SIG_ERR)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: release of ping timeout handler failed"
		     );
      /* NOT REACHED */
    }

  free (p);

  return pe_acked;
#endif
}

/**
 * ---------------------------------------------------------------------------
 */

#if defined(HAVE_MANAGED_SEGMENTS)

/**
 * atomic counters
 */
static volatile unsigned long put_counter = 0L;
static volatile unsigned long get_counter = 0L;

static gasnet_hsl_t put_counter_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t get_counter_lock = GASNET_HSL_INITIALIZER;

static void
atomic_inc_put_counter (void)
{
  gasnet_hsl_lock (&put_counter_lock);
  put_counter += 1L;
  gasnet_hsl_unlock (&put_counter_lock);
}

static void
atomic_dec_put_counter (void)
{
  gasnet_hsl_lock (&put_counter_lock);
  put_counter -= 1L;
  gasnet_hsl_unlock (&put_counter_lock);
}

static void
atomic_wait_put_zero (void)
{
  WAIT_ON_COMPLETION (put_counter == 0L);
}

static void
atomic_inc_get_counter (void)
{
  gasnet_hsl_lock (&get_counter_lock);
  get_counter += 1L;
  gasnet_hsl_unlock (&get_counter_lock);
}

static void
atomic_dec_get_counter (void)
{
  gasnet_hsl_lock (&get_counter_lock);
  get_counter -= 1L;
  gasnet_hsl_unlock (&get_counter_lock);
}

static void
atomic_wait_get_zero (void)
{
  WAIT_ON_COMPLETION (get_counter == 0L);
}

#else /* ! HAVE_MANAGED_SEGMENTS */

#define atomic_inc_put_counter()
#define atomic_dec_put_counter()

#define atomic_inc_get_counter()
#define atomic_dec_get_counter()

#define atomic_wait_put_zero()
#define atomic_wait_get_zero()

#endif /* HAVE_MANAGED_SEGMENTS */


/**
 * ---------------------------------------------------------------------------
 *
 * global variable put/get handlers (for non-everything cases):
 *
 * TODO: locking feels too coarse-grained with static (single) buffer,
 * TODO: would love to find a better way of doing this bit
 *
 */

/**
 * suitably allocate buffer for transfers.
 *
 */

static void
allocate_buffer_and_check (void **buf, size_t siz)
{
  int r = posix_memalign (buf, GASNET_PAGESIZE, siz);
  switch (r)
    {
    case 0:
      /* all ok, return */
      break;
    case EINVAL:
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: global variable payload not aligned correctly"
		     );
      /* NOT REACHED */
      break;
    case ENOMEM:
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: no memory to allocate global variable payload"
		     );
      /* NOT REACHED */
      break;
    default:
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: unknown error with global variable payload (posix_memalign returned %d)",
		     r
		     );
      /* NOT REACHED */
      break;
    }
}

#if defined(HAVE_MANAGED_SEGMENTS)

typedef struct
{
  size_t nbytes;		/* size of write */
  void *target;			/* where to write */
  void *source;			/* data we want to get */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
} globalvar_payload_t;

/**
 * Puts
 */

/**
 * called by remote PE to grab and write to its variable
 */
static void
handler_globalvar_put_out (gasnet_token_t token,
			   void *buf, size_t bufsiz)
{
  globalvar_payload_t *pp = (globalvar_payload_t *) buf;
  void *data = buf + sizeof (*pp);

  memmove (pp->target, data, pp->nbytes);
  LOAD_STORE_FENCE;

  /* return ack, just need the control structure */
  gasnet_AMReplyMedium0 (token, GASNET_HANDLER_GLOBALVAR_PUT_BAK,
			 buf, sizeof (*pp)
			 );
}

/**
 * invoking PE just has to ack remote write
 */
static void
handler_globalvar_put_bak (gasnet_token_t token,
			   void *buf, size_t bufsiz)
{
  globalvar_payload_t *pp = (globalvar_payload_t *) buf;

  *(pp->completed_addr) = 1;
}

/**
 * Generate the active message to do a put to a global variable.
 *
 * Put a lock around this bit so we know when it is safe to reuse the
 * buffer.
 *
 */
static void
put_a_chunk (void *buf, size_t bufsize,
	     void *target, void *source,
	     size_t offset, size_t bytes_to_send, int pe)
{
  globalvar_payload_t *p = buf;
  void *data = buf + sizeof (*p);

  /*
   * build payload to send
   * (global var is trivially symmetric here, no translation needed)
   */
  p->nbytes = bytes_to_send;
  p->source = NULL;		/* not used in put */
  p->target = target + offset;	/* on the other PE */
  p->completed = 0;
  p->completed_addr = &(p->completed);

  atomic_inc_put_counter ();

  /* data added after control structure */
  memmove (data, source + offset, bytes_to_send);
  LOAD_STORE_FENCE;

  gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_GLOBALVAR_PUT_OUT,
			   p, bufsize
			   );

  WAIT_ON_COMPLETION (p->completed);

  atomic_dec_put_counter ();
}

/**
 * perform the put to a global variable
 */
void
__shmem_comms_globalvar_put_request (void *target, void *source,
				     size_t nbytes, int pe)
{
  /* get the buffer size and chop off control structure */
  const size_t max_req = gasnet_AMMaxMedium ();
  const size_t max_data = max_req - sizeof (globalvar_payload_t);
  /* how to split up transfers */
  const size_t nchunks = nbytes / max_data;
  const size_t rem_send = nbytes % max_data;
  /* track size and progress of transfers */
  size_t payload_size;
  size_t alloc_size;
  size_t offset = 0;
  void *put_buf;

  alloc_size = max_req;
  payload_size = max_data;

  allocate_buffer_and_check (&put_buf, alloc_size);

  if (nchunks > 0)
    {
      size_t i;

      for (i = 0; i < nchunks; i += 1)
	{
	  put_a_chunk (put_buf, alloc_size,
		       target, source, offset, payload_size, pe
		       );
	  offset += payload_size;
	}
    }

  if (rem_send > 0)
    {
      payload_size = rem_send;

      put_a_chunk (put_buf, alloc_size,
		   target, source, offset, payload_size, pe
		   );
    }

  free (put_buf);
}

/**
 * Gets
 *
 */

/**
 * called by remote PE to grab remote data and return
 */
static void
handler_globalvar_get_out (gasnet_token_t token,
			   void *buf, size_t bufsiz)
{
  globalvar_payload_t *pp = (globalvar_payload_t *) buf;
  globalvar_payload_t *datap = buf + sizeof (*pp);

  /* fetch from remote global var into payload */
  memmove (datap, pp->source, pp->nbytes);
  LOAD_STORE_FENCE;

  /* return ack, copied data is returned */
  gasnet_AMReplyMedium0 (token, GASNET_HANDLER_GLOBALVAR_GET_BAK,
			 buf, bufsiz
			 );
}

/**
 * called by invoking PE to write fetched data
 */
static void
handler_globalvar_get_bak (gasnet_token_t token,
			   void *buf, size_t bufsiz)
{
  globalvar_payload_t *pp = (globalvar_payload_t *) buf;

  /* write back payload data here */
  memmove (pp->target, buf + sizeof (*pp), pp->nbytes);
  LOAD_STORE_FENCE;

  *(pp->completed_addr) = 1;
}

/**
 * Generate the active message to do a get from a global variable.
 *
 */
static void
get_a_chunk (globalvar_payload_t * p, size_t bufsize,
	     void *target, void *source,
	     size_t offset, size_t bytes_to_send, int pe)
{
  /*
   * build payload to send
   * (global var is trivially symmetric here, no translation needed)
   */
  p->nbytes = bytes_to_send;
  p->source = source + offset;	/* on the other PE */
  p->target = target + offset;	/* track my local writes upon return */
  p->completed = 0;
  p->completed_addr = &(p->completed);

  atomic_inc_get_counter ();

  gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_GLOBALVAR_GET_OUT,
			   p, bufsize
			   );

  WAIT_ON_COMPLETION (p->completed);

  atomic_dec_get_counter ();
}

/**
 * perform the get from a global variable
 */
void
__shmem_comms_globalvar_get_request (void *target, void *source,
				     size_t nbytes, int pe)
{
  /* get the buffer size and chop off control structure */
  const size_t max_req = gasnet_AMMaxMedium ();
  const size_t max_data = max_req - sizeof (globalvar_payload_t);
  /* how to split up transfers */
  const size_t nchunks = nbytes / max_data;
  const size_t rem_send = nbytes % max_data;
  /* track size and progress of transfers */
  size_t payload_size;
  size_t alloc_size;
  size_t offset = 0;
  void *get_buf;

  alloc_size = max_req;

  allocate_buffer_and_check (&get_buf, alloc_size);

  if (nchunks > 0)
    {
      size_t i;

      payload_size = max_data;

      for (i = 0; i < nchunks; i += 1)
	{
	  get_a_chunk (get_buf, alloc_size,
		       target, source, offset, payload_size, pe
		       );
	  offset += payload_size;
	}
    }

  if (rem_send > 0)
    {
      payload_size = rem_send;

      get_a_chunk (get_buf, alloc_size,
		   target, source, offset, payload_size, pe
		   );
    }

  free (get_buf);
}

#endif /* HAVE_MANAGED_SEGMENTS */


/**
 * ---------------------------------------------------------------------------
 */

void
__shmem_comms_put (void *dst, void *src, size_t len, int pe)
{
  void *their_dst = __shmem_symmetric_addr_lookup (dst, pe);

#if defined(HAVE_MANAGED_SEGMENTS)
  if (__shmem_symmetric_is_globalvar (their_dst))
    {
      __shmem_comms_globalvar_put_request (their_dst, src, len, pe);
    }
  else
    {
      GASNET_PUT (pe, their_dst, src, len);
    }
#else
  GASNET_PUT (pe, their_dst, src, len);
#endif /* HAVE_MANAGED_SEGMENTS */
}

void
__shmem_comms_put_bulk (void *dst, void *src, size_t len, int pe)
{
  void *their_dst = __shmem_symmetric_addr_lookup (dst, pe);

#if defined(HAVE_MANAGED_SEGMENTS)
  if (__shmem_symmetric_is_globalvar (their_dst))
    {
      __shmem_comms_globalvar_put_request (their_dst, src, len, pe);
    }
  else
    {
      GASNET_PUT_BULK (pe, their_dst, src, len);
    }
#else
  GASNET_PUT_BULK (pe, their_dst, src, len);
#endif /* HAVE_MANAGED_SEGMENTS */
}

void
__shmem_comms_get (void *dst, void *src, size_t len, int pe)
{
  void *their_src = __shmem_symmetric_addr_lookup (src, pe);

#if defined(HAVE_MANAGED_SEGMENTS)
  if (__shmem_symmetric_is_globalvar (their_src))
    {
      __shmem_comms_globalvar_get_request (dst, their_src, len, pe);
    }
  else
    {
      GASNET_GET (dst, pe, their_src, len);
    }
#else
  GASNET_GET (dst, pe, their_src, len);
#endif /* HAVE_MANAGED_SEGMENTS */
}

void
__shmem_comms_get_bulk (void *dst, void *src, size_t len, int pe)
{
  void *their_src = __shmem_symmetric_addr_lookup (src, pe);

#if defined(HAVE_MANAGED_SEGMENTS)
  if (__shmem_symmetric_is_globalvar (their_src))
    {
      __shmem_comms_globalvar_get_request (dst, their_src, len, pe);
    }
  else
    {
      GASNET_GET_BULK (dst, pe, their_src, len);
    }
#else
  GASNET_GET_BULK (dst, pe, their_src, len);
#endif /* HAVE_MANAGED_SEGMENTS */
}

/**
 * not completely sure about using longs in these two:
 * it's big enough and hides the gasnet type: is that good enough?
 */

void
__shmem_comms_put_val (void *dst, long src, size_t len, int pe)
{
  void *their_dst = __shmem_symmetric_addr_lookup (dst, pe);

#if defined(HAVE_MANAGED_SEGMENTS)
  if (__shmem_symmetric_is_globalvar (their_dst))
    {
      __shmem_comms_globalvar_put_request (their_dst, &src, len, pe);
    }
  else
    {
      GASNET_PUT_VAL (pe, their_dst, src, len);
    }
#else
  GASNET_PUT_VAL (pe, their_dst, src, len);
#endif /* HAVE_MANAGED_SEGMENTS */
}

long
__shmem_comms_get_val (void *src, size_t len, int pe)
{
  long retval;
  void *their_src = __shmem_symmetric_addr_lookup (src, pe);

#if defined(HAVE_MANAGED_SEGMENTS)
  if (__shmem_symmetric_is_globalvar (their_src))
    {
      __shmem_comms_globalvar_get_request (&retval, their_src, len, pe);
    }
  else
    {
      retval = gasnet_get_val (pe, their_src, len);
    }
#else
  retval = gasnet_get_val (pe, their_src, len);
#endif /* HAVE_MANAGED_SEGMENTS */

  return retval;
}


/**
 * ---------------------------------------------------------------------------
 *
 * non-blocking puts: not part of current API
 */

typedef struct
{
  gasnet_handle_t handle;	/* the handle for the NB op. */
  /*
   * might want to put something else here to record more information
   */
  UT_hash_handle hh;		/* makes this structure hashable */
} nb_table_t;

static nb_table_t *nb_table = NULL;

/**
 * couple of simple helper macros
 */

#define HASH_FIND_NB_TABLE(head, findnb, out)			\
  HASH_FIND(hh, head, findnb, sizeof (gasnet_handle_t), out)

#define HASH_ADD_NB_TABLE(head, nbfield, add)			\
  HASH_ADD(hh, head, nbfield, sizeof (gasnet_handle_t), add)

/**
 * add handle into hash table
 */

static void *
nb_table_add (gasnet_handle_t h)
{
  nb_table_t *n = malloc (sizeof (*n));
  if (n == (nb_table_t *) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: unable to alloate memory for non-blocking table"
		     );
      /* NOT REACHED */
    }
  memset (n, 0, sizeof (*n));
  n->handle = h;
  HASH_ADD_NB_TABLE (nb_table, handle, n);
  return n;
}

/**
 * iterate over hash table, build array of handles,
 * pass this to gasnet to wait
 */
static void
nb_table_wait (void)
{
  gasnet_handle_t *g;
  nb_table_t *current, *tmp;
  unsigned int n = HASH_COUNT(nb_table);
  unsigned int i = 0;

  g = malloc (n * sizeof (*g));
  if (g != NULL)
    {
      HASH_ITER (hh, nb_table, current, tmp)
	{
	  g[i] = current->handle;
	  i += 1;
	}
      gasnet_wait_syncnb_all (g, n);
      free (g);
    }
  else
    {
      __shmem_trace (SHMEM_LOG_INFO,
		     "unable to malloc temporary handle storage for"
		     " non-blocking wait, using linearized method"
		     );
      HASH_ITER (hh, nb_table, current, tmp)
	{
	  gasnet_wait_syncnb (current->handle);
	}
    }
}

static
void *
put_nb_helper (void *dst, void *src, size_t len, int pe)
{
  void *n;
  gasnet_handle_t g = gasnet_put_nb_bulk (pe,
					  (void *) dst,
					  (void *) src,
					  len);
  n = nb_table_add (g);
  return n;
}

void *
__shmem_comms_put_nb (void *dst, void *src, size_t len, int pe)
{
  void *h;
  void *their_dst = __shmem_symmetric_addr_lookup (dst, pe);    

#if defined(HAVE_MANAGED_SEGMENTS)
  if (__shmem_symmetric_is_globalvar (their_dst))
    {
      __shmem_comms_globalvar_put_request (their_dst, src, len, pe);
      h = NULL;			/* masquerade as _nb for now */
    }
  else
    {
      h = put_nb_helper (their_dst, src, len, pe);
    }
#else
      h = put_nb_helper (their_dst, src, len, pe);
#endif /* HAVE_MANAGED_SEGMENTS */
  return h;
}

static
void *
get_nb_helper (void *dst, void *src, size_t len, int pe)
{
  void *n;
  gasnet_handle_t g = gasnet_get_nb_bulk ((void *) dst,
					  pe,
					  (void *) src,
					  len);
  n = nb_table_add (g);
  return n;
}

void *
__shmem_comms_get_nb (void *dst, void *src, size_t len, int pe)
{
  void *h;
  void *their_src = __shmem_symmetric_addr_lookup (src, pe);

#if defined(HAVE_MANAGED_SEGMENTS)
  if (__shmem_symmetric_is_globalvar (their_src))
    {
      __shmem_comms_globalvar_get_request (dst, their_src, len, pe);
      h = NULL;			/* masquerade for now */
    }
  else
    {
      h = get_nb_helper (dst, their_src, len, pe);
    }
#else
  h = get_nb_helper (dst, their_src, len, pe);
#endif /* HAVE_MANAGED_SEGMENTS */
  return h;
}

/**
 * wait for the handle to be completed
 */
void
__shmem_comms_wait_nb (void *h)
{
  if (h != NULL)
    {
      nb_table_t *n = (nb_table_t *) h;

      gasnet_wait_syncnb (n->handle);
      LOAD_STORE_FENCE;

      /* remove from handle table */
      HASH_DEL (nb_table, n);
      free (n);
    }
  else
    {
      __shmem_comms_quiet_request (); /* no specific handle, so quiet for all */
    }
}

/**
 * check to see if the handle has been completed.  Return 1 if so, 0
 * if not
 */
int
__shmem_comms_test_nb (void *h)
{
  if (h != NULL)
    {
      nb_table_t *n = (nb_table_t *) h;
      nb_table_t *res;
      int s;

      /* have we already waited on this handle? */
      HASH_FIND_NB_TABLE (nb_table, n, res);
      if (res == NULL)
	{
	  return 1;		/* cleared => complete */
	}

      /* if gasnet says "ok", then complete */
      s = gasnet_try_syncnb (n->handle);
      return (s == GASNET_OK) ? 1 : 0;
    }
  else
    {
      return 1;			/* no handle, carry on */
    }
}

/**
 * called by mainline to fence off outstanding requests
 *
 * chances here for fence/quiet differentiation and optimization
 */

void
__shmem_comms_quiet_request (void)
{
  GASNET_WAIT_PUTS ();
  atomic_wait_put_zero ();

  nb_table_wait ();

  LOAD_STORE_FENCE;
  return;
}

void
__shmem_comms_fence_request (void)
{
  __shmem_comms_quiet_request ();
}


/**
 * ---------------------------------------------------------------------------
 *
 * start of handlers
 */

static gasnet_handlerentry_t
handlers[] =
  {
#if ! defined(HAVE_MANAGED_SEGMENTS)
    {GASNET_HANDLER_SETUP_OUT,          handler_segsetup_out},
    {GASNET_HANDLER_SETUP_BAK,          handler_segsetup_bak},
#endif /* ! HAVE_MANAGED_SEGMENTS */
    {GASNET_HANDLER_SWAP_OUT, 		handler_swap_out},
    {GASNET_HANDLER_SWAP_BAK, 		handler_swap_bak},
    {GASNET_HANDLER_CSWAP_OUT,	        handler_cswap_out},
    {GASNET_HANDLER_CSWAP_BAK,	        handler_cswap_bak},
    {GASNET_HANDLER_FADD_OUT, 		handler_fadd_out},
    {GASNET_HANDLER_FADD_BAK, 		handler_fadd_bak},
    {GASNET_HANDLER_FINC_OUT, 		handler_finc_out},
    {GASNET_HANDLER_FINC_BAK, 		handler_finc_bak},
    {GASNET_HANDLER_ADD_OUT, 		handler_add_out},
    {GASNET_HANDLER_ADD_BAK, 		handler_add_bak},
    {GASNET_HANDLER_INC_OUT, 		handler_inc_out},
    {GASNET_HANDLER_INC_BAK, 		handler_inc_bak},
    {GASNET_HANDLER_PING_OUT, 		handler_ping_out},
    {GASNET_HANDLER_PING_BAK, 		handler_ping_bak},
#if 0				/* not used */
    {GASNET_HANDLER_QUIET_OUT,          handler_quiet_out},
    {GASNET_HANDLER_QUIET_BAK,          handler_quiet_bak},
#endif /* not used */
    {GASNET_HANDLER_XOR_OUT,            handler_xor_out},
    {GASNET_HANDLER_XOR_BAK,            handler_xor_bak},
#if defined(HAVE_MANAGED_SEGMENTS)
    {GASNET_HANDLER_GLOBALVAR_PUT_OUT, 	handler_globalvar_put_out},
    {GASNET_HANDLER_GLOBALVAR_PUT_BAK, 	handler_globalvar_put_bak},
    {GASNET_HANDLER_GLOBALVAR_GET_OUT, 	handler_globalvar_get_out},
    {GASNET_HANDLER_GLOBALVAR_GET_BAK, 	handler_globalvar_get_bak},
#endif /* HAVE_MANAGED_SEGMENTS */
  };
static const int nhandlers = TABLE_SIZE (handlers);

/**
 * end of handlers
 */

/**
 * First parse out the process' command-line.  This is important for
 * the UDP conduit, in which the number of PEs comes from the
 * command-line rather than a launcher program.
 */

static int argc;
static char **argv;

static const char *cmdline = "/proc/self/cmdline";
static const char *cmdline_fmt = "/proc/%ld/cmdline";

static void
parse_cmdline(void)
{
  FILE *fp;
  char buf[1024];		/* TODO: arbitrary size */
  char *p = buf;
  int i = 0;
  int c;

  argc = 0;

  /*
   * try to find this process' command-line:
   * either from short-cut, or from pid
   */
  fp = fopen(cmdline, "r");
  if (fp == NULL)
    {
      char buf[MAXPATHLEN];
      snprintf (buf, MAXPATHLEN, cmdline_fmt, getpid ());
      fp = fopen(buf, "r");
      if (fp == NULL)
	{
	  __shmem_trace (SHMEM_LOG_FATAL,
			 "could not discover process' command-line (%s)",
			 strerror (errno)
			 );
	  /* NOT REACHED */
	}
    }
  
  /* first count the number of nuls in cmdline to see how many args */
  while ((c = fgetc(fp)) != EOF)
    {
      if (c == '\0')
        {
          argc += 1;
        }
    }
  rewind(fp);

  argv = (char **) malloc ((argc + 1) * sizeof (*argv));
  if (argv == (char **) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: unable to allocate memory for faked command-line arguments"
		     );
      /* NOT REACHED */
    }

  while (1)
    {
      int c = fgetc(fp);
      switch (c)
	{
	case EOF:		/* end of args */
	  argv[i] = NULL;
	  goto end;
	  break;
	case '\0':		/* end of this arg */
	  *p = c;
	  argv[i++] = strdup(buf);
	  p = buf;
	  break;
	default:		/* copy out char in this arg  */
	  *p++ = c;
	  break;
	}
    }
 end:
  fclose(fp);
}

/**
 * GASNet does this timeout thing if its collective routines
 * (e.g. barrier) go idle, so make this as long as possible
 */

static void
maximize_gasnet_timeout (void)
{
  char buf[32];
  snprintf (buf, 32, "%d", INT_MAX - 1);
  setenv ("GASNET_EXITTIMEOUT", buf, 1);
}


/**
 * -----------------------------------------------------------------------
 *
 * This is where the communications layer gets set up and torn down
 */

void
__shmem_comms_init (void)
{
  parse_cmdline ();

  maximize_gasnet_timeout ();

  /*
   * let's get gasnet up and running
   */
  GASNET_SAFE (gasnet_init (&argc, &argv));

  /*
   * now we can ask about the node count & heap
   */
  SET_STATE (mype, __shmem_comms_mynode ());
  SET_STATE (numpes, __shmem_comms_nodes ());
  SET_STATE (heapsize, __shmem_comms_get_segment_size ());

  /*
   * not guarding the attach for different gasnet models,
   * since last 2 params are ignored if not needed
   */
  GASNET_SAFE (gasnet_attach (handlers, nhandlers, GET_STATE (heapsize), 0));

  __shmem_trace (SHMEM_LOG_INIT,
		 "gasnet attached, %d handlers registered, heap = %ld bytes",
		 nhandlers,
		 GET_STATE (heapsize)
		 );

  __shmem_comms_set_waitmode (SHMEM_COMMS_SPINBLOCK);

  __shmem_service_init ();

  /*
   * make sure all nodes are up to speed before "declaring"
   * initialization done
   */
  __shmem_comms_barrier_all ();

  __shmem_trace (SHMEM_LOG_INIT,
		 "communication layer initialization complete"
		 );

  /* Up and running! */
}

/**
 * bail out of run-time with STATUS error code
 */
void
__shmem_comms_exit (int status)
{
  __shmem_comms_barrier_all ();

  gasnet_exit (status);
}

/**
 * make sure everyone finishes stuff, then exit with STATUS.
 */
void
__shmem_comms_finalize (int status)
{
  __shmem_service_finalize ();

  if (argv != NULL)
    {
      int i;
      for (i = 0; i < argc; i += 1)
	{
	  if (argv[i] != NULL)
	    {
	      free (argv[i]);
	    }
	}
      free (argv);
    }

  __shmem_comms_exit (status);
}
