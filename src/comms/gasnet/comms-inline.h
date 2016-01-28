/*
 *
 * Copyright (c) 2011 - 2015
 *   University of Houston System and UT-Battelle, LLC.
 * Copyright (c) 2009 - 2015
 *   Silicon Graphics International Corp.  SHMEM is copyrighted
 *   by Silicon Graphics International Corp. (SGI) The OpenSHMEM API
 *   (shmem) is released by Open Source Software Solutions, Inc., under an
 *   agreement with Silicon Graphics International Corp. (SGI).
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
 * o Neither the name of the University of Houston System,
 *   UT-Battelle, LLC. nor the names of its contributors may be used to
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
 * non-static that starts with "shmemi_comms_"
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

#include "uthash.h"

#include "state.h"
#include "memalloc.h"
#include "atomic.h"
#include "ping.h"
#include "exe.h"
#include "globalvar.h"
#include "clock.h"

#include "barrier.h"
#include "barrier-all.h"
#include "broadcast.h"
#include "collect.h"
#include "fcollect.h"

#include "trace.h"
#include "utils.h"
#include "unitparse.h"

#include "shmemx.h"

#include "comms-shared.h"


/**
 * --------------- real work starts here ---------------------
 *
 */

static inline void comms_bailout (char *fmt, ...);
static inline void shmemi_comms_exit (int status);
static inline void *shmemi_symmetric_addr_lookup (void *dest, int pe);

/**
 * trap gasnet errors gracefully
 *
 */
#define GASNET_SAFE(fncall)                                                \
    do {                                                                   \
           const int _retval = fncall ;                                    \
           if (_retval != GASNET_OK) {                                     \
               comms_bailout ("error calling: %s at %s:%i, %s (%s)\n",     \
                              #fncall, __FILE__, __LINE__,                 \
                              gasnet_ErrorName (_retval),                  \
                              gasnet_ErrorDesc (_retval)                   \
                              );                                           \
           }                                                               \
       } while(0)


/* bail.c */

#define MSG_BUF_SIZE 256

/**
 * Handle error messages while initializing the comms layer.  We don't
 * have access to the trace sub-system yet, since it depends on comms
 * being up to get PE and other informational output
 */

static inline void
comms_bailout (char *fmt, ...)
{
    char tmp1[MSG_BUF_SIZE];
    char tmp2[MSG_BUF_SIZE];    /* incoming args */
    va_list ap;

    strncpy (tmp1, "COMMS ERROR: ", MSG_BUF_SIZE);

    va_start (ap, fmt);
    vsnprintf (tmp2, MSG_BUF_SIZE, fmt, ap);
    va_end (ap);

    strncat (tmp1, tmp2, strlen (tmp2));
    strncat (tmp1, "\n", 1);

    fputs (tmp1, stderr);
    fflush (stderr);

    shmemi_comms_exit (1);
}

/* end: bail.c */

/* locality query */

static bool thread_starter = false;

static inline bool
shmemi_thread_starter (void)
{
    const int me = GET_STATE (mype);
    const int *where = GET_STATE (locp);

    /* PE 0 always starts a thread */
    if (me == 0) {
        return true;
    }

    /* only start thread if I am first PE on a host */
    if (where[me - 1] < where[me]) {
        return true;
    }

    return false;
}

/**
 * get some hopefully-interesting locality information.
 *
 */
static inline void
place_init (void)
{
    const int n = GET_STATE (numpes);
    gasnet_nodeinfo_t *gnip;
    int i;

    gnip = (gasnet_nodeinfo_t *) malloc (n * sizeof(*gnip));
    if (gnip == NULL) {
        shmemi_trace (SHMEM_LOG_FATAL,
                      "internal error: cannot allocate memory for locality queries");
        return;
        /* NOT REACHED */
    }

    GASNET_SAFE (gasnet_getNodeInfo (gnip, n));

    SET_STATE (locp, (int *) malloc (n * sizeof(int)));
    if (GET_STATE (locp) == NULL) {
        shmemi_trace (SHMEM_LOG_FATAL,
                      "internal error: cannot allocate memory for locality queries");
        return;
        /* NOT REACHED */
    }

    /*
     * populate the neighborhood table - we just record the GASNet
     * node to which each PE belongs (if different, we assume they
     * can't share memory).
     */
    for (i = 0; i < n; i += 1) {
        SET_STATE (locp[i], gnip[i].host);
    }

    free (gnip);

    /*
     * TODO: free up the neighborhood table on finalize
     */
}

/* end: locality query */

/* service.c */

/**
 * Do network service.  When code is not engaged in shmem calls,
 * something needs to provide communication access so that operations
 * where "this" PE is a passive target can continue
 */

/**
 * choose thread implementation
 */

#undef SHMEM_USE_QTHREADS

#if defined(SHMEM_USE_QTHREADS)

#include <qthread.h>

typedef aligned_t shmem_thread_return_t;
typedef qthread_f shmem_thread_t;

static shmem_thread_return_t thr_ret;

#else

/* defaulting to pthreads */

#define SHMEM_USE_PTHREADS 1

#include <pthread.h>

typedef void *shmem_thread_return_t;
typedef pthread_t shmem_thread_t;

/**
 * new thread for progress-o-matic
 */

static shmem_thread_t thr;

#endif /* threading model */

/**
 * for hi-res timer
 */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309
#endif /* _POSIX_C_SOURCE */
#include <time.h>

/**
 * for refractory back-off
 */

static long delay = 1000L;      /* ns */
static struct timespec delayspec;

/**
 * polling sentinel
 */

static volatile bool done = false;

/**
 * Does comms. service until told not to
 */

static shmem_thread_return_t
start_service (void *unused)
{
    do {
        gasnet_AMPoll ();
        pthread_yield ();
        nanosleep (&delayspec, NULL);   /* back off */
    }
    while (!done);

    return (shmem_thread_return_t) 0;
}

/**
 * assume initially we need to manage progress ourselves
 */
static bool use_conduit_thread = false;

/**
 * tell a PE how to contend for updates
 *
 */
static inline void
waitmode_init (void)
{
    /*
     * this gives best performance in all cases observed by the author
     * (@ UH).  Could make this programmable.
     */
    gasnet_set_waitmode (GASNET_WAIT_SPINBLOCK);
}

/**
 * start the servicer
 */

static inline void
shmemi_service_init (void)
{
    /*
     * Zap this code for now.  Problems with IBV conduit thread if all
     * PEs on one physical node.
     *
     */
#if 0
#if defined(GASNETC_IBV_RCV_THREAD) &&                          \
  (defined(GASNET_CONDUIT_IBV) || defined(GASNET_CONDUIT_VAPI))
    /*
     * if we have an IBV progress thread configured, then check env for
     * GASNET_RCV_THREAD.
     *
     * With no env var, let ibv conduit handle things...
     *
     * If set to [0nN] (false), we start our own progress thread
     * If set to [1yY] (true), the conduit handles progress
     *
     * Any other value, assume true but make a note.  NB with 1.20.2,
     * GASNet itself traps other values and aborts.
     *
     */

    const char *grt_str = "GASNET_RCV_THREAD";
    char *rtv = shmemi_comms_getenv (grt_str);
    if (EXPR_LIKELY (rtv == NULL)) {
        use_conduit_thread = true;
    }
    else {
        switch (*rtv) {
        case '0':
        case 'n':
        case 'N':
            use_conduit_thread = false;
            break;
        case '1':
        case 'y':
        case 'Y':
            use_conduit_thread = true;
            break;
        default:
            use_conduit_thread = true;
            break;
        }
    }
#endif /* defined(GASNETC_IBV_RCV_THREAD) && (defined(GASNET_CONDUIT_IBV) ||
          defined(GASNET_CONDUIT_VAPI)) */
#endif /* commented out */

    if (!use_conduit_thread) {
        delayspec.tv_sec = (time_t) 0;
        delayspec.tv_nsec = delay;

#if defined(GASNET_CONDUIT_MPI)
        thread_starter = true;
#else
        thread_starter = shmemi_thread_starter ();
#endif /* GASNET_CONDUIT_MPI */

        if (thread_starter) {
#if defined(SHMEM_USE_PTHREADS)
            const int s = pthread_create (&thr, NULL, start_service, (void *) 0);
#elif defined(SHMEM_USE_QTHREADS)
            qthread_initialize ();

            const int s = qthread_fork (start_service, (void *) 0, &thr_ret);
#endif

            if (EXPR_UNLIKELY (s != 0)) {
                comms_bailout
                    ("internal error: progress thread creation failed (%s)",
                     strerror (s)
                     );
                /* NOT REACHED */
            }
            waitmode_init ();
        }
    }
}

/**
 * stop the servicer
 */

static inline void
shmemi_service_finalize (void)
{
    if (!use_conduit_thread) {
        done = true;

        if (thread_starter) {
#if defined(SHMEM_USE_PTHREADS)
            const int s = pthread_join (thr, NULL);

            if (EXPR_UNLIKELY (s != 0)) {
                comms_bailout
                    ("internal error: progress thread termination failed (%s)",
                     strerror (s)
                     );
                /* NOT REACHED */
            }
#elif defined(SHMEM_USE_QTHREADS)
            /**
             * not sure if need readFF() here
             */
            qthread_finalize ();
#endif
        }
    }
}

/* end: service.c */


/**
 * which node (PE) am I?
 */
static inline int
shmemi_comms_mynode (void)
{
    return (int) gasnet_mynode ();
}

/**
 * how many nodes (PEs) take part in this program?
 */
static inline int
shmemi_comms_nodes (void)
{
    return (int) gasnet_nodes ();
}


/**
 * ---------------------------------------------------------------------------
 *
 * global barrier done through gasnet
 *
 */

static inline void
shmemi_comms_barrier_all (void)
{
    /* GASNET_BEGIN_FUNCTION(); */

    /* use gasnet's global barrier */
    gasnet_barrier_notify (barcount, barflag);
    GASNET_SAFE (gasnet_barrier_wait (barcount, barflag));

    barcount += 1;
}

/**
 * gasnet put model: this is just for testing different put
 * emulations; generally we want the nbi routines to get performance.
 *
 */

#define USING_IMPLICIT_HANDLES 1

#if USING_IMPLICIT_HANDLES
#define GASNET_PUT(pe, dst, src, len)      gasnet_put_nbi (pe, dst, src, len)
#define GASNET_PUT_BULK(pe, dst, src, len) gasnet_put_nbi_bulk (pe, dst, src, len)
#define GASNET_PUT_VAL(pe, dst, src, len)  gasnet_put_nbi_val (pe, dst, src, len)
#define GASNET_WAIT_PUTS()                 gasnet_wait_syncnbi_puts ()
#define GASNET_WAIT_ALL()                  gasnet_wait_syncnbi_all ()
#else
#define GASNET_PUT(pe, dst, src, len)      gasnet_put (pe, dst, src, len)
#define GASNET_PUT_BULK(pe, dst, src, len) gasnet_put_bulk (pe, dst, src, len)
#define GASNET_PUT_VAL(pe, dst, src, len)  gasnet_put_val (pe, dst, src, len)
#define GASNET_WAIT_PUTS()
#define GASNET_WAIT_ALL()
#endif /* USING_IMPLICIT_HANDLES */

#define GASNET_GET(dst, pe, src, len)      gasnet_get (dst, pe, src, len)
#define GASNET_GET_BULK(dst, pe, src, len) gasnet_get_bulk (dst, pe, src, len)

/**
 * ---------------------------------------------------------------------------
 *
 * lookup where another PE stores things
 *
 */

/**
 * where the symmetric memory lives on the given PE
 */
#define SHMEM_SYMMETRIC_HEAP_BASE(p) (seginfo_table[(p)].addr)
#define SHMEM_SYMMETRIC_HEAP_SIZE(p) (seginfo_table[(p)].size)

/**
 * translate my "dest" to corresponding address on PE "pe"
 */
static inline void *
shmemi_symmetric_addr_lookup (void *dest, int pe)
{
    /* globals are in same place everywhere */
    if (shmemi_symmetric_is_globalvar (dest)) {
        return dest;
    }

    /* symmetric if inside of heap */
    {
        int me = GET_STATE (mype);
        size_t al = (size_t) SHMEM_SYMMETRIC_HEAP_BASE (me); /* lower bound */
        size_t aao = (size_t) dest; /* my addr as offset */
        long offset = aao - al;

        /* trap addresses outside the heap */
        if (offset < 0) {
            return NULL;
        }
        if (offset > SHMEM_SYMMETRIC_HEAP_SIZE (me)) {
            return NULL;
        }

        /* and where it is in the remote heap */
        return SHMEM_SYMMETRIC_HEAP_BASE (pe) + offset;
    }
}

/*
 * --------------------------------------------------------------
 *
 * GASNet allows applications to use handler codes 128-255.
 *
 * See http://gasnet.cs.berkeley.edu/dist/docs/gasnet.html, under
 * description of gasnet_attach ()
 */

#define AMO_HANDLER_DEF(Op, Name)               \
    GASNET_HANDLER_##Op##_out_##Name,           \
    GASNET_HANDLER_##Op##_bak_##Name

enum
{
    GASNET_HANDLER_setup_out = 128,
    GASNET_HANDLER_setup_bak,

    AMO_HANDLER_DEF (swap, int),
    AMO_HANDLER_DEF (swap, long),
    AMO_HANDLER_DEF (swap, longlong),
    AMO_HANDLER_DEF (swap, float),
    AMO_HANDLER_DEF (swap, double),

    AMO_HANDLER_DEF (cswap, int),
    AMO_HANDLER_DEF (cswap, long),
    AMO_HANDLER_DEF (cswap, longlong),

    AMO_HANDLER_DEF (fadd, int),
    AMO_HANDLER_DEF (fadd, long),
    AMO_HANDLER_DEF (fadd, longlong),

    AMO_HANDLER_DEF (finc, int),
    AMO_HANDLER_DEF (finc, long),
    AMO_HANDLER_DEF (finc, longlong),

    AMO_HANDLER_DEF (add, int),
    AMO_HANDLER_DEF (add, long),
    AMO_HANDLER_DEF (add, longlong),

    AMO_HANDLER_DEF (inc, int),
    AMO_HANDLER_DEF (inc, long),
    AMO_HANDLER_DEF (inc, longlong),

    AMO_HANDLER_DEF (xor, int),
    AMO_HANDLER_DEF (xor, long),
    AMO_HANDLER_DEF (xor, longlong),

    AMO_HANDLER_DEF (fetch, int),
    AMO_HANDLER_DEF (fetch, long),
    AMO_HANDLER_DEF (fetch, longlong),

    AMO_HANDLER_DEF (set, int),
    AMO_HANDLER_DEF (set, long),
    AMO_HANDLER_DEF (set, longlong),

    GASNET_HANDLER_globalvar_put_out,
    GASNET_HANDLER_globalvar_put_bak,
    GASNET_HANDLER_globalvar_get_out,
    GASNET_HANDLER_globalvar_get_bak,

    GASNET_HANDLER_globalexit_out
    /* no reply partner for global_exit */
};

/**
 * can't just call getenv, it might not pass through environment
 * info to other nodes from launch.
 */
static inline char *
shmemi_comms_getenv (const char *name)
{
    return gasnet_getenv (name);
}

/**
 * work out how big the symmetric segment areas should be.
 *
 * Either from environment setting, or default value from
 * implementation
 */
static inline size_t
shmemi_comms_get_segment_size (void)
{
    char *mlss_str = shmemi_comms_getenv ("SHMEM_SYMMETRIC_HEAP_SIZE");
    size_t retval;
    int ok;

    if (EXPR_LIKELY (mlss_str == (char *) NULL)) {
#ifdef HAVE_MANAGED_SEGMENTS
        return (size_t) gasnet_getMaxLocalSegmentSize ();
#else
        return DEFAULT_HEAP_SIZE;
#endif
    }

    shmemi_parse_size (mlss_str, &retval, &ok);
    if (EXPR_LIKELY (ok)) {
        /* make sure aligned to page size multiples */
        const size_t mod = retval % GASNET_PAGESIZE;

        if (EXPR_UNLIKELY (mod != 0)) {
            const size_t div = retval / GASNET_PAGESIZE;
            retval = (div + 1) * GASNET_PAGESIZE;
        }

        return retval;
        /* NOT REACHED */
    }

    comms_bailout ("Unusable symmetric heap size \"%s\"", mlss_str);
    /* NOT REACHED */
    return 0;
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
 * unpack buf from sender PE and store seg info locally.  Ack. receipt.
 */
static void
handler_segsetup_out (gasnet_token_t token, void *buf, size_t bufsiz)
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

    gasnet_AMReplyMedium0 (token, GASNET_HANDLER_setup_bak, (void *) NULL, 0);
}

/**
 * record receipt ack.  We only need to count the number of replies
 */
static void
handler_segsetup_bak (gasnet_token_t token, void *buf, size_t bufsiz)
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

static inline void
shmemi_symmetric_memory_init (void)
{
    const int me = GET_STATE (mype);
    const int npes = GET_STATE (numpes);

    /*
     * calloc zeroes for us
     */
    seginfo_table =
        (gasnet_seginfo_t *) calloc (npes, sizeof (gasnet_seginfo_t));
    if (EXPR_UNLIKELY (seginfo_table == (gasnet_seginfo_t *) NULL)) {
        comms_bailout ("could not allocate GASNet segments (%s)",
                       strerror (errno)
            );
        /* NOT REACHED */
    }

    /*
     * prep the segments for use across all PEs
     *
     */

    {
#ifdef HAVE_MANAGED_SEGMENTS

        /* gasnet handles the segment allocation for us */
        GASNET_SAFE (gasnet_getSegmentInfo (seginfo_table, npes));

#else

        const size_t heapsize = GET_STATE (heapsize);
        int pm_r;

        /* allocate the heap - has to be pagesize aligned */
        pm_r = posix_memalign (&great_big_heap, GASNET_PAGESIZE, heapsize);
        if (EXPR_UNLIKELY (pm_r != 0)) {
            comms_bailout ("unable to allocate symmetric heap (%s)",
                           strerror (pm_r)
                );
            /* NOT REACHED */
        }

        /* everyone has their local info before exchanging messages */
        shmemi_comms_barrier_all ();

        /* store my own heap entry */
        seginfo_table[me].addr = great_big_heap;
        seginfo_table[me].size = heapsize;

        {
            gasnet_seginfo_t gs;
            int pe;

            gs.addr = great_big_heap;
            gs.size = heapsize;

            for (pe = 0; pe < npes; pe += 1) {
                /* send to everyone else */
                if (EXPR_LIKELY (me != pe)) {
                    gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_setup_out,
                                             &gs, sizeof (gs)
                        );
                }
            }

            /* now wait on the AM replies (0-based AND don't count myself) */
            GASNET_BLOCKUNTIL (seg_setup_replies_received == npes - 1);
        }

#endif /* HAVE_MANAGED_SEGMENTS */
    }

    /* initialize my heap */
    shmemi_mem_init (seginfo_table[me].addr, seginfo_table[me].size);

    /* and make sure everyone is up-to-speed */
    /* shmemi_comms_barrier_all (); */

}

/**
 * shut down the memory allocation handler
 */
static inline void
shmemi_symmetric_memory_finalize (void)
{
    shmemi_mem_finalize ();
#if ! defined(HAVE_MANAGED_SEGMENTS)
    free (great_big_heap);
#endif /* HAVE_MANAGED_SEGMENTS */
}

/**
 * -- atomics handlers ---------------------------------------------------------
 */

/*
 * to wait on remote updates
 */

#define VOLATILIZE(Type, Var) (* ( volatile Type *) (Var))

#define COMMS_WAIT_TYPE(Name, Type, OpName, Op)                         \
    static inline void                                                  \
    shmemi_comms_wait_##Name##_##OpName (volatile Type *var, Type cmp_value) \
    {                                                                   \
        GASNET_BLOCKUNTIL ( VOLATILIZE (Type, var) Op cmp_value );      \
    }

COMMS_WAIT_TYPE (short, short, eq, ==);
COMMS_WAIT_TYPE (int, int, eq, ==);
COMMS_WAIT_TYPE (long, long, eq, ==);
COMMS_WAIT_TYPE (longlong, long long, eq, ==);

COMMS_WAIT_TYPE (short, short, ne, !=);
COMMS_WAIT_TYPE (int, int, ne, !=);
COMMS_WAIT_TYPE (long, long, ne, !=);
COMMS_WAIT_TYPE (longlong, long long, ne, !=);

COMMS_WAIT_TYPE (short, short, gt, >);
COMMS_WAIT_TYPE (int, int, gt, >);
COMMS_WAIT_TYPE (long, long, gt, >);
COMMS_WAIT_TYPE (longlong, long long, gt, >);

COMMS_WAIT_TYPE (short, short, le, <=);
COMMS_WAIT_TYPE (int, int, le, <=);
COMMS_WAIT_TYPE (long, long, le, <=);
COMMS_WAIT_TYPE (longlong, long long, le, <=);

COMMS_WAIT_TYPE (short, short, lt, <);
COMMS_WAIT_TYPE (int, int, lt, <);
COMMS_WAIT_TYPE (long, long, lt, <);
COMMS_WAIT_TYPE (longlong, long long, lt, <);

COMMS_WAIT_TYPE (short, short, ge, >=);
COMMS_WAIT_TYPE (int, int, ge, >=);
COMMS_WAIT_TYPE (long, long, ge, >=);
COMMS_WAIT_TYPE (longlong, long long, ge, >=);

#define WAIT_ON_COMPLETION(Cond)   GASNET_BLOCKUNTIL (Cond)

/* TODO: need a handler per-datatype to get the correct hander lock.
   We can do this easily with a template for the out/bak RPCs and the
   request generator itself. */

/**
 * called by remote PE to do the swap.  Store new value, send back old value
 */
#define AMO_SWAP_BAK_EMIT(Name, Type)                                       \
    static void                                                         \
    handler_swap_out_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        Type old;                                                       \
        amo_payload_##Name##_t *pp = (amo_payload_##Name##_t *) buf;    \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* save and update */                                           \
        old = *(pp->r_symm_addr);                                       \
        *(pp->r_symm_addr) = pp->value;                                 \
        pp->value = old;                                                \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
                                                                        \
        /* return updated payload */                                    \
        gasnet_AMReplyMedium0 (token, GASNET_HANDLER_swap_bak_##Name,   \
                               buf, bufsiz);                            \
    }

AMO_SWAP_BAK_EMIT (int, int);
AMO_SWAP_BAK_EMIT (long, long);
AMO_SWAP_BAK_EMIT (longlong, long long);
AMO_SWAP_BAK_EMIT (float, float);
AMO_SWAP_BAK_EMIT (double, double);

/**
 * called by swap invoker when old value returned by remote PE
 */
#define AMO_SWAP_OUT_EMIT(Name, Type)                                   \
    static void                                                         \
    handler_swap_bak_##Name (gasnet_token_t token, void *buf, size_t bufsiz)   \
    {                                                                   \
        amo_payload_##Name##_t *pp = (amo_payload_##Name##_t *) buf;    \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* save returned value */                                       \
        *(pp->value_addr) = pp->value;                                  \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
                                                                        \
        /* done it */                                                   \
        *(pp->completed_addr) = 1;                                      \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
    }

AMO_SWAP_OUT_EMIT (int, int);
AMO_SWAP_OUT_EMIT (long, long);
AMO_SWAP_OUT_EMIT (longlong, long long);
AMO_SWAP_OUT_EMIT (float, float);
AMO_SWAP_OUT_EMIT (double, double);

/*
 * TODO:
 *
 * This, as in all the atomic handlers, is where the opportunity gap
 * is.  We could do useful things between firing off the request and
 * waiting for the completion notification.  So split this out into
 * a post and wait/poll pair, post returning a handle for the atomic
 * op. in progress.
 */

/**
 * perform the swap
 */
#define AMO_SWAP_REQ_EMIT(Name, Type)                                   \
    static inline Type                                                  \
    shmemi_comms_swap_request_##Name (Type *target, Type value, int pe) \
    {                                                                   \
        amo_payload_##Name##_t *p =                                     \
            (amo_payload_##Name##_t *) malloc (sizeof (*p));            \
        if (EXPR_UNLIKELY (p == NULL)) {                                \
            comms_bailout                                               \
                ("internal error: unable to allocate swap payload memory"); \
        }                                                               \
        /* build payload to send */                                     \
        p->r_symm_addr = shmemi_symmetric_addr_lookup (target, pe);     \
                                                                        \
        p->value = value;                                               \
        p->value_addr = &(p->value);                                    \
                                                                        \
        p->completed = 0;                                               \
        p->completed_addr = &(p->completed);                            \
                                                                        \
        /* fire off request */                                          \
        gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_swap_out_##Name,    \
                                 p, sizeof (*p));                       \
                                                                        \
        WAIT_ON_COMPLETION (p->completed);                              \
        free (p);                                                       \
        return p->value;                                                \
    }

AMO_SWAP_REQ_EMIT (int, int);
AMO_SWAP_REQ_EMIT (long, long);
AMO_SWAP_REQ_EMIT (longlong, long long);
AMO_SWAP_REQ_EMIT (float, float);
AMO_SWAP_REQ_EMIT (double, double);

/**
 * called by remote PE to do the swap.  Store new value if cond
 * matches, send back old value in either case
 */
#define AMO_CSWAP_OUT_EMIT(Name, Type)                                  \
    static void                                                         \
    handler_cswap_out_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        Type old;                                                       \
        amo_payload_##Name##_t *pp =                                    \
            (amo_payload_##Name##_t *) buf;                             \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* save current target */                                       \
        old = *(pp->r_symm_addr);                                       \
                                                                        \
        /* update value if cond matches */                              \
        if (pp->cond == old) {                                          \
            *(pp->r_symm_addr) = pp->value;                             \
        }                                                               \
                                                                        \
        /* return value */                                              \
        pp->value = old;                                                \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
                                                                        \
        /* return updated payload */                                    \
        gasnet_AMReplyMedium0 (token, GASNET_HANDLER_cswap_bak_##Name,  \
                               buf, bufsiz);                            \
    }

AMO_CSWAP_OUT_EMIT (int, int);
AMO_CSWAP_OUT_EMIT (long, long);
AMO_CSWAP_OUT_EMIT (longlong, long long);

/**
 * called by swap invoker when old value returned by remote PE
 * (same as swap_bak for now)
 */
#define AMO_CSWAP_BAK_EMIT(Name, Type)                                  \
    static void                                                         \
    handler_cswap_bak_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        amo_payload_##Name##_t *pp =                                    \
            (amo_payload_##Name##_t *) buf;                             \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* save returned value */                                       \
        (*pp->value_addr) = pp->value;                                  \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
                                                                        \
        /* done it */                                                   \
        *(pp->completed_addr) = 1;                                      \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
    }

AMO_CSWAP_BAK_EMIT (int, int);
AMO_CSWAP_BAK_EMIT (long, long);
AMO_CSWAP_BAK_EMIT (longlong, long long);

/**
 * perform the conditional swap
 */
#define AMO_CSWAP_REQ_EMIT(Name, Type)                                  \
    static inline Type                                                  \
    shmemi_comms_cswap_request_##Name (Type *target, Type cond,         \
                                       Type value,                      \
                                       int pe)                          \
    {                                                                   \
        amo_payload_##Name##_t *cp =                                    \
            (amo_payload_##Name##_t *) malloc (sizeof (*cp));           \
        if (EXPR_UNLIKELY (cp == NULL)) {                               \
            comms_bailout                                               \
                ("internal error: unable to allocate conditional swap payload memory"); \
        }                                                               \
        /* build payload to send */                                     \
        cp->r_symm_addr = shmemi_symmetric_addr_lookup (target, pe);    \
                                                                        \
        cp->value = value;                                              \
        cp->value_addr = &(cp->value);                                  \
        cp->cond = cond;                                                \
                                                                        \
        cp->completed = 0;                                              \
        cp->completed_addr = &(cp->completed);                          \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
        /* fire off request */                                          \
        gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_cswap_out_##Name,   \
                                 cp, sizeof (*cp));                     \
                                                                        \
        WAIT_ON_COMPLETION (cp->completed);                             \
                                                                        \
        free (cp);                                                      \
                                                                        \
        return cp->value;                                               \
    }

AMO_CSWAP_REQ_EMIT (int, int);
AMO_CSWAP_REQ_EMIT (long, long);
AMO_CSWAP_REQ_EMIT (longlong, long long);

/**
 * fetch/add
 */

/**
 * called by remote PE to do the fetch and add.  Store new value, send
 * back old value
 */
#define AMO_FADD_OUT_EMIT(Name, Type)           \
    static void                                                         \
    handler_fadd_out_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        Type old;                                                       \
        amo_payload_##Name##_t *pp =                                    \
            (amo_payload_##Name##_t *) buf;                             \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* save and update */                                           \
        old = *(pp->r_symm_addr);                                       \
        *(pp->r_symm_addr) += pp->value;                                \
        pp->value = old;                                                \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
                                                                        \
        /* return updated payload */                                    \
        gasnet_AMReplyMedium0 (token, GASNET_HANDLER_fadd_bak_##Name,   \
                               buf, bufsiz);                            \
    }

AMO_FADD_OUT_EMIT (int, int);
AMO_FADD_OUT_EMIT (long, long);
AMO_FADD_OUT_EMIT (longlong, long long);

/**
 * called by fadd invoker when old value returned by remote PE
 */
#define AMO_FADD_BAK_EMIT(Name, Type)                                   \
    static void                                                         \
    handler_fadd_bak_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        amo_payload_##Name##_t *pp =                                    \
            (amo_payload_##Name##_t *) buf;                             \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* save returned value */                                       \
        *(pp->value_addr) = pp->value;                                  \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
                                                                        \
        /* done it */                                                   \
        *(pp->completed_addr) = 1;                                      \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
    }

AMO_FADD_BAK_EMIT (int, int);
AMO_FADD_BAK_EMIT (long, long);
AMO_FADD_BAK_EMIT (longlong, long long);

/**
 * perform the fetch-and-add
 */
#define AMO_FADD_REQ_EMIT(Name, Type)                                   \
    static inline Type                                                  \
    shmemi_comms_fadd_request_##Name (Type *target, Type value, int pe) \
    {                                                                   \
        amo_payload_##Name##_t *p =                                     \
            (amo_payload_##Name##_t *) malloc (sizeof (*p));            \
        if (EXPR_UNLIKELY (p == NULL)) {                                \
            comms_bailout                                               \
                ("internal error: unable to allocate fetch-and-add payload memory"); \
        }                                                               \
        /* build payload to send */                                     \
        p->r_symm_addr = shmemi_symmetric_addr_lookup (target, pe);     \
                                                                        \
        p->value = value;                                               \
        p->value_addr = &(p->value);                                    \
                                                                        \
        p->completed = 0;                                               \
        p->completed_addr = &(p->completed);                            \
                                                                        \
        /* fire off request */                                          \
        gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_fadd_out_##Name,    \
                                 p, sizeof (*p));                       \
                                                                        \
        WAIT_ON_COMPLETION (p->completed);                              \
                                                                        \
        free (p);                                                       \
                                                                        \
        return p->value;                                                \
    }

AMO_FADD_REQ_EMIT (int, int);
AMO_FADD_REQ_EMIT (long, long);
AMO_FADD_REQ_EMIT (longlong, long long);

/**
 * fetch/increment
 */

/**
 * called by remote PE to do the fetch and increment.  Store new
 * value, send back old value
 */
#define AMO_FINC_OUT_EMIT(Name, Type)                                   \
    static void                                                         \
    handler_finc_out_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        Type old;                                                       \
        amo_payload_##Name##_t *pp = (amo_payload_##Name##_t *) buf;    \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* save and update */                                           \
        old = *(pp->r_symm_addr);                                       \
        *(pp->r_symm_addr) += 1;                                        \
        pp->value = old;                                                \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
                                                                        \
        /* return updated payload */                                    \
        gasnet_AMReplyMedium0 (token, GASNET_HANDLER_finc_bak_##Name,   \
                               buf, bufsiz);                            \
    }

AMO_FINC_OUT_EMIT (int, int);
AMO_FINC_OUT_EMIT (long, long);
AMO_FINC_OUT_EMIT (longlong, long long);

/**
 * called by finc invoker when old value returned by remote PE
 */
#define AMO_FINC_BAK_EMIT(Name, Type)                                       \
    static void                                                         \
    handler_finc_bak_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        amo_payload_##Name##_t *pp = (amo_payload_##Name##_t *) buf;    \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* save returned value */                                       \
        *(pp->value_addr) = pp->value;                                  \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
                                                                        \
        /* done it */                                                   \
        *(pp->completed_addr) = 1;                                      \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
    }

AMO_FINC_BAK_EMIT (int, int);
AMO_FINC_BAK_EMIT (long, long);
AMO_FINC_BAK_EMIT (longlong, long long);

/**
 * perform the fetch-and-increment
 */
#define AMO_FINC_REQ_EMIT(Name, Type)                                       \
    static inline Type                                                  \
    shmemi_comms_finc_request_##Name (Type *target, int pe)                     \
    {                                                                   \
        amo_payload_##Name##_t *p =                                     \
            (amo_payload_##Name##_t *) malloc (sizeof (*p));            \
        if (EXPR_UNLIKELY (p == NULL)) {                                \
            comms_bailout                                               \
                ("internal error: unable to allocate fetch-and-increment payload memory"); \
        }                                                               \
        /* build payload to send */                                     \
        p->r_symm_addr = shmemi_symmetric_addr_lookup (target, pe);     \
                                                                        \
        p->value_addr = &(p->value);                                    \
                                                                        \
        p->completed = 0;                                               \
        p->completed_addr = &(p->completed);                            \
                                                                        \
        /* fire off request */                                          \
        gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_finc_out_##Name,    \
                                 p, sizeof (*p));                       \
                                                                        \
        WAIT_ON_COMPLETION (p->completed);                              \
                                                                        \
        free (p);                                                       \
                                                                        \
        return p->value;                                                \
    }

AMO_FINC_REQ_EMIT (int, int);
AMO_FINC_REQ_EMIT (long, long);
AMO_FINC_REQ_EMIT (longlong, long long);

/**
 * remote add
 */

/**
 * called by remote PE to do the remote add.
 */
#define AMO_ADD_OUT_EMIT(Name, Type)                                    \
    static void                                                         \
    handler_add_out_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        amo_payload_##Name##_t *pp =                                    \
            (amo_payload_##Name##_t *) buf;                             \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* save and update */                                           \
        *(pp->r_symm_addr) += pp->value;                                \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
                                                                        \
        /* return updated payload */                                    \
        gasnet_AMReplyMedium0 (token, GASNET_HANDLER_add_bak_##Name,    \
                               buf, bufsiz);                            \
    }

AMO_ADD_OUT_EMIT (int, int);
AMO_ADD_OUT_EMIT (long, long);
AMO_ADD_OUT_EMIT (longlong, long long);

/**
 * called by remote add invoker when store done
 */
#define AMO_ADD_BAK_EMIT(Name, Type)                                    \
    static void                                                         \
    handler_add_bak_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        amo_payload_##Name##_t *pp =                                    \
            (amo_payload_##Name##_t *) buf;                             \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* done it */                                                   \
        *(pp->completed_addr) = 1;                                      \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
    }

AMO_ADD_BAK_EMIT (int, int);
AMO_ADD_BAK_EMIT (long, long);
AMO_ADD_BAK_EMIT (longlong, long long);

/**
 * perform the add
 */
#define AMO_ADD_REQ_EMIT(Name, Type)                                    \
    static inline void                                                  \
    shmemi_comms_add_request_##Name (Type *target, Type value, int pe)  \
    {                                                                   \
     amo_payload_##Name##_t *p =                                        \
            (amo_payload_##Name##_t *) malloc (sizeof (*p));            \
        if (EXPR_UNLIKELY (p == NULL)) {                                \
            comms_bailout                                               \
                ("internal error: unable to allocate remote add payload memory"); \
        }                                                               \
        /* build payload to send */                                     \
        p->r_symm_addr = shmemi_symmetric_addr_lookup (target, pe);     \
                                                                        \
        p->value = value;                                               \
        p->value_addr = &(p->value);                                    \
                                                                        \
        p->completed = 0;                                               \
        p->completed_addr = &(p->completed);                            \
                                                                        \
        /* fire off request */                                          \
        gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_add_out_##Name,     \
                                 p, sizeof (*p));                       \
                                                                        \
        WAIT_ON_COMPLETION (p->completed);                              \
                                                                        \
        free (p);                                                       \
    }

AMO_ADD_REQ_EMIT (int, int);
AMO_ADD_REQ_EMIT (long, long);
AMO_ADD_REQ_EMIT (longlong, long long);

/**
 * remote increment
 */

/**
 * called by remote PE to do the remote increment
 */
#define AMO_INC_OUT_EMIT(Name, Type)                                    \
    static void                                                         \
    handler_inc_out_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        amo_payload_##Name##_t *pp =                                    \
            (amo_payload_##Name##_t *) buf;                             \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* save and update */                                           \
        *(pp->r_symm_addr) += 1;                                        \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
                                                                        \
        /* return updated payload */                                    \
        gasnet_AMReplyMedium0 (token, GASNET_HANDLER_inc_bak_##Name,    \
                               buf, bufsiz);                            \
    }

AMO_INC_OUT_EMIT (int, int);
AMO_INC_OUT_EMIT (long, long);
AMO_INC_OUT_EMIT (longlong, long long);

/**
 * called by remote increment invoker when store done
 */
#define AMO_INC_BAK_EMIT(Name, Type)                                    \
    static void                                                         \
    handler_inc_bak_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        amo_payload_##Name##_t *pp =                                    \
            (amo_payload_##Name##_t *) buf;                             \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* done it */                                                   \
        *(pp->completed_addr) = 1;                                      \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
    }

AMO_INC_BAK_EMIT (int, int);
AMO_INC_BAK_EMIT (long, long);
AMO_INC_BAK_EMIT (longlong, long long);

/**
 * perform the increment
 */
#define AMO_INC_REQ_EMIT(Name, Type)                                    \
    static inline void                                                  \
    shmemi_comms_inc_request_##Name (Type *target, int pe)                      \
    {                                                                   \
        amo_payload_##Name##_t *p =                                     \
            (amo_payload_##Name##_t *) malloc (sizeof (*p));            \
        if (EXPR_UNLIKELY (p == NULL)) {                                \
            comms_bailout                                               \
                ("internal error: unable to allocate remote increment payload memory"); \
        }                                                               \
        /* build payload to send */                                     \
                                                                        \
        p->r_symm_addr = shmemi_symmetric_addr_lookup (target, pe);     \
                                                                        \
        p->completed = 0;                                               \
        p->completed_addr = &(p->completed);                            \
                                                                        \
        /* fire off request */                                          \
        gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_inc_out_##Name,     \
                                 p, sizeof (*p));                       \
                                                                        \
        WAIT_ON_COMPLETION (p->completed);                              \
                                                                        \
        free (p);                                                       \
    }

AMO_INC_REQ_EMIT (int, int);
AMO_INC_REQ_EMIT (long, long);
AMO_INC_REQ_EMIT (longlong, long long);


#if defined(HAVE_FEATURE_EXPERIMENTAL)

/**
 * Proposed by IBM Zurich
 *
 * remote xor
 */

/**
 * called by remote PE to do the remote xor
 */
#define AMO_XOR_OUT_EMIT(Name, Type)                                    \
    static void                                                         \
    handler_xor_out_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        amo_payload_##Name##_t *pp =                                    \
            (amo_payload_##Name##_t *) buf;                             \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* save and update */                                           \
        *(pp->r_symm_addr) ^= pp->value;                                \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
                                                                        \
        /* return updated payload */                                    \
        gasnet_AMReplyMedium0 (token, GASNET_HANDLER_xor_bak_##Name,    \
                               buf, bufsiz);                            \
    }

AMO_XOR_OUT_EMIT (int, int);
AMO_XOR_OUT_EMIT (long, long);
AMO_XOR_OUT_EMIT (longlong, long long);

/**
 * called by remote xor invoker when store done
 */
#define AMO_XOR_BAK_EMIT(Name, Type)                                    \
    static void                                                         \
    handler_xor_bak_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        amo_payload_##Name##_t *pp =                                    \
            (amo_payload_##Name##_t *) buf;                             \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* done it */                                                   \
        *(pp->completed_addr) = 1;                                      \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
    }

AMO_XOR_BAK_EMIT (int, int);
AMO_XOR_BAK_EMIT (long, long);
AMO_XOR_BAK_EMIT (longlong, long long);

/**
 * perform the xor
 */
#define AMO_XOR_REQ_EMIT(Name, Type)                                    \
    static inline void                                                  \
    shmemi_comms_xor_request_##Name (Type *target, Type value, int pe)          \
    {                                                                   \
        amo_payload_##Name##_t *p =                                     \
            (amo_payload_##Name##_t *) malloc (sizeof (*p));            \
        if (EXPR_UNLIKELY (p == NULL)) {                                \
            comms_bailout                                               \
                ("internal error: unable to allocate remote exclusive-or payload memory"); \
        }                                                               \
        /* build payload to send */                                     \
        p->r_symm_addr = shmemi_symmetric_addr_lookup (target, pe);     \
                                                                        \
        p->value = value;                                               \
        p->value_addr = &(p->value);                                    \
                                                                        \
        p->completed = 0;                                               \
        p->completed_addr = &(p->completed);                            \
                                                                        \
        /* fire off request */                                          \
        gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_xor_out_##Name,     \
                                 p, sizeof (*p));                       \
                                                                        \
        WAIT_ON_COMPLETION (p->completed);                              \
                                                                        \
        free (p);                                                       \
    }

AMO_XOR_REQ_EMIT (int, int);
AMO_XOR_REQ_EMIT (long, long);
AMO_XOR_REQ_EMIT (longlong, long long);


/**
 * fetch
 */

/**
 * called by remote PE to do the fetch and add.  Store new value, send
 * back old value
 */
#define AMO_FETCH_OUT_EMIT(Name, Type)                                  \
    static void                                                         \
    handler_fetch_out_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        amo_payload_##Name##_t *pp =                                    \
            (amo_payload_##Name##_t *) buf;                             \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        pp->value = *(pp->r_symm_addr);                                 \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
                                                                        \
        /* return updated payload */                                    \
        gasnet_AMReplyMedium0 (token, GASNET_HANDLER_fetch_bak_##Name,  \
                               buf, bufsiz);                            \
    }

AMO_FETCH_OUT_EMIT (int, int);
AMO_FETCH_OUT_EMIT (long, long);
AMO_FETCH_OUT_EMIT (longlong, long long);

/**
 * called by fetch invoker when value returned by remote PE
 */
#define AMO_FETCH_BAK_EMIT(Name, Type)                                  \
    static void                                                         \
    handler_fetch_bak_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        amo_payload_##Name##_t *pp =                                    \
            (amo_payload_##Name##_t *) buf;                             \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* save returned value */                                       \
        *(pp->value_addr) = pp->value;                                  \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
                                                                        \
        /* done it */                                                   \
        *(pp->completed_addr) = 1;                                      \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
    }

AMO_FETCH_BAK_EMIT (int, int);
AMO_FETCH_BAK_EMIT (long, long);
AMO_FETCH_BAK_EMIT (longlong, long long);

/**
 * perform the fetch
 */
#define AMO_FETCH_REQ_EMIT(Name, Type)                                  \
    static inline Type                                                  \
    shmemi_comms_fetch_request_##Name (Type *target, int pe)            \
    {                                                                   \
        amo_payload_##Name##_t *p =                                     \
            (amo_payload_##Name##_t *) malloc (sizeof (*p));            \
        if (EXPR_UNLIKELY (p == NULL)) {                                \
            comms_bailout                                               \
                ("internal error: unable to allocate fetch payload memory"); \
        }                                                               \
        /* build payload to send */                                     \
        p->r_symm_addr = shmemi_symmetric_addr_lookup (target, pe);     \
                                                                        \
        p->value_addr = &(p->value);                                    \
                                                                        \
        p->completed = 0;                                               \
        p->completed_addr = &(p->completed);                            \
                                                                        \
        /* fire off request */                                          \
        gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_fetch_out_##Name,   \
                                 p, sizeof (*p));                       \
                                                                        \
        WAIT_ON_COMPLETION (p->completed);                              \
                                                                        \
        free (p);                                                       \
                                                                        \
        return p->value;                                                \
    }

AMO_FETCH_REQ_EMIT (int, int);
AMO_FETCH_REQ_EMIT (long, long);
AMO_FETCH_REQ_EMIT (longlong, long long);

/**
 * called by remote PE to do the set
 */
#define AMO_SET_OUT_EMIT(Name, Type)                                    \
    static void                                                         \
    handler_set_out_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        amo_payload_##Name##_t *pp =                                    \
            (amo_payload_##Name##_t *) buf;                             \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        *(pp->r_symm_addr) = pp->value;                                 \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
                                                                        \
        /* return updated payload */                                    \
        gasnet_AMReplyMedium0 (token, GASNET_HANDLER_set_bak_##Name,   \
                               buf, bufsiz);                            \
    }

AMO_SET_OUT_EMIT (int, int);
AMO_SET_OUT_EMIT (long, long);
AMO_SET_OUT_EMIT (longlong, long long);

/**
 * called by set invoker when remote PE replies
 */
#define AMO_SET_BAK_EMIT(Name, Type)                                    \
    static void                                                         \
    handler_set_bak_##Name (gasnet_token_t token, void *buf, size_t bufsiz) \
    {                                                                   \
        amo_payload_##Name##_t *pp =                                    \
            (amo_payload_##Name##_t *) buf;                             \
                                                                        \
        gasnet_hsl_lock (&amo_lock_##Name);                             \
                                                                        \
        /* save returned value */                                       \
        *(pp->value_addr) = pp->value;                                  \
                                                                        \
        LOAD_STORE_FENCE ();                                            \
                                                                        \
        /* done it */                                                   \
        *(pp->completed_addr) = 1;                                      \
                                                                        \
        gasnet_hsl_unlock (&amo_lock_##Name);                           \
    }

AMO_SET_BAK_EMIT (int, int);
AMO_SET_BAK_EMIT (long, long);
AMO_SET_BAK_EMIT (longlong, long long);

/**
 * perform the fetch-and-add
 */
#define AMO_SET_REQ_EMIT(Name, Type)                                    \
    static inline void                                                  \
    shmemi_comms_set_request_##Name (Type *target, Type value, int pe)  \
    {                                                                   \
        amo_payload_##Name##_t *p =                                     \
            (amo_payload_##Name##_t *) malloc (sizeof (*p));            \
        if (EXPR_UNLIKELY (p == NULL)) {                                \
            comms_bailout                                               \
                ("internal error: unable to allocate set payload memory"); \
        }                                                               \
        /* build payload to send */                                     \
        p->r_symm_addr = shmemi_symmetric_addr_lookup (target, pe);     \
                                                                        \
        p->value = value;                                               \
        p->value_addr = &(p->value);                                    \
                                                                        \
        p->completed = 0;                                               \
        p->completed_addr = &(p->completed);                            \
                                                                        \
        /* fire off request */                                          \
        gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_set_out_##Name,     \
                                 p, sizeof (*p));                       \
                                                                        \
        WAIT_ON_COMPLETION (p->completed);                              \
                                                                        \
        free (p);                                                       \
    }

AMO_SET_REQ_EMIT (int, int);
AMO_SET_REQ_EMIT (long, long);
AMO_SET_REQ_EMIT (longlong, long long);

#endif /* HAVE_FEATURE_EXPERIMENTAL */

/**
 * ---------------------------------------------------------------------------
 */

/**
 * perform the ping
 *
 * TODO: JUST RETURN TRUE FOR NOW IF GOOD PE, NEED TO WORK ON PROGRESS LOGIC
 *
 */
static inline int
shmemi_comms_ping_request (int pe)
{
    if ( (pe >= 0) && (pe < GET_STATE(numpes)) ) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * ---------------------------------------------------------------------------
 */

#if defined(HAVE_MANAGED_SEGMENTS)

/* TODO non-static */

/**
 * atomic counters
 */
static volatile unsigned long put_counter = 0L;
static volatile unsigned long get_counter = 0L;

static gasnet_hsl_t put_counter_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t get_counter_lock = GASNET_HSL_INITIALIZER;

static inline void
atomic_inc_put_counter (void)
{
    gasnet_hsl_lock (&put_counter_lock);
    put_counter += 1L;
    gasnet_hsl_unlock (&put_counter_lock);
}

static inline void
atomic_dec_put_counter (void)
{
    gasnet_hsl_lock (&put_counter_lock);
    put_counter -= 1L;
    gasnet_hsl_unlock (&put_counter_lock);
}

static inline void
atomic_wait_put_zero (void)
{
    WAIT_ON_COMPLETION (put_counter == 0L);
}

static inline void
atomic_inc_get_counter (void)
{
    gasnet_hsl_lock (&get_counter_lock);
    get_counter += 1L;
    gasnet_hsl_unlock (&get_counter_lock);
}

static inline void
atomic_dec_get_counter (void)
{
    gasnet_hsl_lock (&get_counter_lock);
    get_counter -= 1L;
    gasnet_hsl_unlock (&get_counter_lock);
}

static inline void
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

static inline void
allocate_buffer_and_check (void **buf, size_t siz)
{
    int r = posix_memalign (buf, GASNET_PAGESIZE, siz);
    switch (r) {
    case 0:
        /* all ok, return */
        break;
    case EINVAL:
        comms_bailout
            ("internal error: global variable payload not aligned correctly");
        /* NOT REACHED */
        break;
    case ENOMEM:
        comms_bailout
            ("internal error: no memory to allocate global variable payload");
        /* NOT REACHED */
        break;
    default:
        comms_bailout
            ("internal error: unknown error with global variable payload (posix_memalign returned %d)",
             r);
        /* NOT REACHED */
        break;
    }
}

#if defined(HAVE_MANAGED_SEGMENTS)

/**
 * Puts
 */

/**
 * called by remote PE to grab and write to its variable
 */
static void
handler_globalvar_put_out (gasnet_token_t token, void *buf, size_t bufsiz)
{
    globalvar_payload_t *pp = (globalvar_payload_t *) buf;
    void *data = buf + sizeof (*pp);

    memmove (pp->target, data, pp->nbytes);
    LOAD_STORE_FENCE ();

    /* return ack, just need the control structure */
    gasnet_AMReplyMedium0 (token, GASNET_HANDLER_globalvar_put_bak,
                           buf, sizeof (*pp)
        );
}

/**
 * invoking PE just has to ack remote write
 */
static void
handler_globalvar_put_bak (gasnet_token_t token, void *buf, size_t bufsiz)
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
static inline void
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
    p->source = NULL;           /* not used in put */
    p->target = target + offset;    /* on the other PE */
    p->completed = 0;
    p->completed_addr = &(p->completed);

    atomic_inc_put_counter ();

    /* data added after control structure */
    memmove (data, source + offset, bytes_to_send);
    LOAD_STORE_FENCE ();

    gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_globalvar_put_out, p, bufsize);

    WAIT_ON_COMPLETION (p->completed);

    atomic_dec_put_counter ();
}

/**
 * perform the put to a global variable
 */
static inline void
shmemi_comms_globalvar_put_request (void *target, void *source,
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

    if (EXPR_LIKELY (nchunks > 0)) {
        size_t i;

        for (i = 0; i < nchunks; i += 1) {
            put_a_chunk (put_buf, alloc_size,
                         target, source, offset, payload_size, pe);
            offset += payload_size;
        }
    }

    if (EXPR_LIKELY (rem_send > 0)) {
        payload_size = rem_send;

        put_a_chunk (put_buf, alloc_size,
                     target, source, offset, payload_size, pe);
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
handler_globalvar_get_out (gasnet_token_t token, void *buf, size_t bufsiz)
{
    globalvar_payload_t *pp = (globalvar_payload_t *) buf;
    globalvar_payload_t *datap = buf + sizeof (*pp);

    /* fetch from remote global var into payload */
    memmove (datap, pp->source, pp->nbytes);
    LOAD_STORE_FENCE ();

    /* return ack, copied data is returned */
    gasnet_AMReplyMedium0 (token, GASNET_HANDLER_globalvar_get_bak,
                           buf, bufsiz);
}

/**
 * called by invoking PE to write fetched data
 */
static void
handler_globalvar_get_bak (gasnet_token_t token, void *buf, size_t bufsiz)
{
    globalvar_payload_t *pp = (globalvar_payload_t *) buf;

    /* write back payload data here */
    memmove (pp->target, buf + sizeof (*pp), pp->nbytes);
    LOAD_STORE_FENCE ();

    *(pp->completed_addr) = 1;
}

/**
 * Generate the active message to do a get from a global variable.
 *
 */
static inline void
get_a_chunk (globalvar_payload_t * p, size_t bufsize,
             void *target, void *source,
             size_t offset, size_t bytes_to_send, int pe)
{
    /*
     * build payload to send
     * (global var is trivially symmetric here, no translation needed)
     */
    p->nbytes = bytes_to_send;
    p->source = source + offset;    /* on the other PE */
    p->target = target + offset;    /* track my local writes upon return */
    p->completed = 0;
    p->completed_addr = &(p->completed);

    atomic_inc_get_counter ();

    gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_globalvar_get_out, p, bufsize);

    WAIT_ON_COMPLETION (p->completed);

    atomic_dec_get_counter ();
}

/**
 * perform the get from a global variable
 */

static inline void
shmemi_comms_globalvar_get_request (void *target, void *source,
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

    if (EXPR_LIKELY (nchunks > 0)) {
        size_t i;

        payload_size = max_data;

        for (i = 0; i < nchunks; i += 1) {
            get_a_chunk (get_buf, alloc_size,
                         target, source, offset, payload_size, pe);
            offset += payload_size;
        }
    }

    if (EXPR_LIKELY (rem_send > 0)) {
        payload_size = rem_send;

        get_a_chunk (get_buf, alloc_size,
                     target, source, offset, payload_size, pe);
    }

    free (get_buf);
}

#endif /* HAVE_MANAGED_SEGMENTS */

/**
 * ---------------------------------------------------------------------------
 */

static inline void
shmemi_comms_put (void *dst, void *src, size_t len, int pe)
{
#if defined(HAVE_MANAGED_SEGMENTS)
    if (shmemi_symmetric_is_globalvar (dst)) {
        shmemi_comms_globalvar_put_request (dst, src, len, pe);
    }
    else {
        void *their_dst = shmemi_symmetric_addr_lookup (dst, pe);
        GASNET_PUT (pe, their_dst, src, len);
    }
#else
    void *their_dst = shmemi_symmetric_addr_lookup (dst, pe);
    GASNET_PUT (pe, their_dst, src, len);
#endif /* HAVE_MANAGED_SEGMENTS */
}

static inline void
shmemi_comms_put_bulk (void *dst, void *src, size_t len, int pe)
{
#if defined(HAVE_MANAGED_SEGMENTS)
    if (shmemi_symmetric_is_globalvar (dst)) {
        shmemi_comms_globalvar_put_request (dst, src, len, pe);
    }
    else {
        void *their_dst = shmemi_symmetric_addr_lookup (dst, pe);
        GASNET_PUT_BULK (pe, their_dst, src, len);
    }
#else
    void *their_dst = shmemi_symmetric_addr_lookup (dst, pe);
    GASNET_PUT_BULK (pe, their_dst, src, len);
#endif /* HAVE_MANAGED_SEGMENTS */
}

static inline void
shmemi_comms_get (void *dst, void *src, size_t len, int pe)
{
#if defined(HAVE_MANAGED_SEGMENTS)
    if (shmemi_symmetric_is_globalvar (src)) {
        shmemi_comms_globalvar_get_request (dst, src, len, pe);
    }
    else {
        void *their_src = shmemi_symmetric_addr_lookup (src, pe);
        GASNET_GET (dst, pe, their_src, len);
    }
#else
    void *their_src = shmemi_symmetric_addr_lookup (src, pe);
    GASNET_GET (dst, pe, their_src, len);
#endif /* HAVE_MANAGED_SEGMENTS */
}

static inline void
shmemi_comms_get_bulk (void *dst, void *src, size_t len, int pe)
{
#if defined(HAVE_MANAGED_SEGMENTS)
    if (shmemi_symmetric_is_globalvar (src)) {
        shmemi_comms_globalvar_get_request (dst, src, len, pe);
    }
    else {
        void *their_src = shmemi_symmetric_addr_lookup (src, pe);
        GASNET_GET_BULK (dst, pe, their_src, len);
    }
#else
    void *their_src = shmemi_symmetric_addr_lookup (src, pe);
    GASNET_GET_BULK (dst, pe, their_src, len);
#endif /* HAVE_MANAGED_SEGMENTS */
}

/**
 * not completely sure about using longs in these two:
 * it's big enough and hides the gasnet type: is that good enough?
 */

static inline void
shmemi_comms_put_val (void *dst, long src, size_t len, int pe)
{
#if defined(HAVE_MANAGED_SEGMENTS)
    if (shmemi_symmetric_is_globalvar (dst)) {
        shmemi_comms_globalvar_put_request (dst, &src, len, pe);
    }
    else {
        void *their_dst = shmemi_symmetric_addr_lookup (dst, pe);
        GASNET_PUT_VAL (pe, their_dst, src, len);
    }
#else
    void *their_dst = shmemi_symmetric_addr_lookup (dst, pe);
    GASNET_PUT_VAL (pe, their_dst, src, len);
#endif /* HAVE_MANAGED_SEGMENTS */
}

static inline long
shmemi_comms_get_val (void *src, size_t len, int pe)
{
    long retval;
#if defined(HAVE_MANAGED_SEGMENTS)
    if (shmemi_symmetric_is_globalvar (src)) {
        shmemi_comms_globalvar_get_request (&retval, src, len, pe);
    }
    else {
        void *their_src = shmemi_symmetric_addr_lookup (src, pe);
        retval = gasnet_get_val (pe, their_src, len);
    }
#else
    void *their_src = shmemi_symmetric_addr_lookup (src, pe);
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
    gasnet_handle_t handle;     /* the handle for the NB op. */
    /*
     * might want to put something else here to record more information
     */
    UT_hash_handle hh;          /* makes this structure hashable */
} nb_table_t;

static nb_table_t *nb_table = NULL;

/**
 * couple of simple helper macros
 */

#define HASH_FIND_NB_TABLE(head, findnb, out)                 \
  HASH_FIND(hh, head, findnb, sizeof (gasnet_handle_t), out)

#define HASH_ADD_NB_TABLE(head, nbfield, add)                 \
  HASH_ADD(hh, head, nbfield, sizeof (gasnet_handle_t), add)

/**
 * add handle into hash table
 */

static inline void *
nb_table_add (gasnet_handle_t h)
{
    nb_table_t *n = malloc (sizeof (*n));
    if (EXPR_UNLIKELY (n == (nb_table_t *) NULL)) {
        comms_bailout
            ("internal error: unable to alloate memory for non-blocking table");
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
static inline void
nb_table_wait (void)
{
    gasnet_handle_t *g;
    nb_table_t *current, *tmp;
    unsigned int n = HASH_COUNT (nb_table);
    unsigned int i = 0;

    g = malloc (n * sizeof (*g));
    if (EXPR_UNLIKELY (g != NULL)) {
        HASH_ITER (hh, nb_table, current, tmp) {
            g[i] = current->handle;
            i += 1;
        }
        gasnet_wait_syncnb_all (g, n);
        free (g);
    }
    else {
        HASH_ITER (hh, nb_table, current, tmp) {
            gasnet_wait_syncnb (current->handle);
        }
    }
}

static inline void *
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

/**
 * called by mainline to fence off outstanding requests
 *
 * chances here for fence/quiet differentiation and optimization, but
 * we'll just fence <=> quiet
 */

static inline void
do_fencequiet (void)
{
    atomic_wait_put_zero ();
    GASNET_WAIT_PUTS ();
    nb_table_wait ();

    LOAD_STORE_FENCE ();
    return;
}

static inline void
shmemi_comms_quiet_request (void)
{
    do_fencequiet ();
}

static inline void
shmemi_comms_fence_request (void)
{
    do_fencequiet ();
}

/**
 * fence and quiet tests just call fence/quiet and then report success
 *
 */

static inline int
shmemi_fence_test (void)
{
    shmemi_comms_fence_request ();
    return 1;
}

static inline int
shmemi_quiet_test (void)
{
    shmemi_comms_quiet_request ();
    return 1;
}

/**
 * "nb" puts and gets
 *
 */

static inline void
shmemi_comms_put_nb (void *dst, void *src, size_t len, int pe,
                     shmemx_request_handle_t * desc)
{
#if defined(HAVE_MANAGED_SEGMENTS)
    if (shmemi_symmetric_is_globalvar (dst)) {
        shmemi_comms_globalvar_put_request (dst, src, len, pe);
        *desc = NULL;           /* masquerade as _nb for now */
    }
    else {
        void *their_dst = shmemi_symmetric_addr_lookup (dst, pe);
        *desc = put_nb_helper (their_dst, src, len, pe);
    }
#else
    void *their_dst = shmemi_symmetric_addr_lookup (dst, pe);
    *desc = put_nb_helper (their_dst, src, len, pe);
#endif /* HAVE_MANAGED_SEGMENTS */
}

static inline void *
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

static inline void
shmemi_comms_get_nb (void *dst, void *src, size_t len, int pe,
                     shmemx_request_handle_t * desc)
{
#if defined(HAVE_MANAGED_SEGMENTS)
    if (shmemi_symmetric_is_globalvar (src)) {
        shmemi_comms_globalvar_get_request (dst, src, len, pe);
        *desc = NULL;           /* masquerade for now */
    }
    else {
        void *their_src = shmemi_symmetric_addr_lookup (src, pe);
        *desc = get_nb_helper (dst, their_src, len, pe);
    }
#else
    void *their_src = shmemi_symmetric_addr_lookup (src, pe);
    *desc = get_nb_helper (dst, their_src, len, pe);
#endif /* HAVE_MANAGED_SEGMENTS */
}

/**
 * wait for the handle to be completed
 */
static inline void
shmemi_comms_wait_req (shmemx_request_handle_t desc)
{
    if (desc != NULL) {
        nb_table_t *n = (nb_table_t *) desc;

        gasnet_wait_syncnb (n->handle);
        LOAD_STORE_FENCE ();

        /* remove from handle table */
        HASH_DEL (nb_table, n);
        free (n);
    }
    else {
        shmemi_comms_quiet_request ();  /* no specific handle, so quiet for all
                                         */
    }
}

/**
 * check to see if the handle has been completed.  Return 1 if so, 0
 * if not
 */
static inline void
shmemi_comms_test_req (shmemx_request_handle_t desc, int *flag)
{
    if (desc != NULL) {
        nb_table_t *n = (nb_table_t *) desc;
        nb_table_t *res;
        int s;

        /* have we already waited on this handle? */
        HASH_FIND_NB_TABLE (nb_table, n, res);
        if (res == NULL) {
            *flag = 1;          /* cleared => complete */
        }

        /* if gasnet says "ok", then complete */
        s = gasnet_try_syncnb (n->handle);
        *flag = (s == GASNET_OK) ? 1 : 0;
    }
    else {
        *flag = 1;              /* no handle, carry on */
    }
}

/* global exit */

/**
 * called by remote PE when global_exit demanded
 */
static void
handler_globalexit_out (gasnet_token_t token, void *buf, size_t bufsiz)
{
    int status = *(int *) buf;

    shmemi_comms_fence_request ();

    _exit (status);
}

/**
 * called by initiator PE of global_exit
 *
 * TODO: tree-based setup would be more scalable.
 */
static void
shmemi_comms_globalexit_request (int status)
{
    const int me = GET_STATE (mype);
    const int npes = GET_STATE (numpes);
    int pe;

    for (pe = 0; pe < npes; pe += 1) {
        /* send to everyone else */
        if (EXPR_LIKELY (me != pe)) {
            gasnet_AMRequestMedium0 (pe, GASNET_HANDLER_globalexit_out,
                                     &status, sizeof (status)
                );
        }
    }

    shmemi_comms_fence_request ();

    _exit (status);
}

/* end: global exit */


/**
 * ---------------------------------------------------------------------------
 *
 * start of handlers
 */

#define AMO_HANDLER_LOOKUP(Op, Name) \
    {GASNET_HANDLER_##Op##_out_##Name, handler_##Op##_out_##Name}, \
    {GASNET_HANDLER_##Op##_bak_##Name, handler_##Op##_bak_##Name}

static gasnet_handlerentry_t handlers[] = {
#if ! defined(HAVE_MANAGED_SEGMENTS)
    {GASNET_HANDLER_setup_out, handler_segsetup_out},
    {GASNET_HANDLER_setup_bak, handler_segsetup_bak},
#endif /* ! HAVE_MANAGED_SEGMENTS */

    AMO_HANDLER_LOOKUP (swap, int),
    AMO_HANDLER_LOOKUP (swap, long),
    AMO_HANDLER_LOOKUP (swap, longlong),
    AMO_HANDLER_LOOKUP (swap, float),
    AMO_HANDLER_LOOKUP (swap, double),

    AMO_HANDLER_LOOKUP (cswap, int),
    AMO_HANDLER_LOOKUP (cswap, long),
    AMO_HANDLER_LOOKUP (cswap, longlong),

    AMO_HANDLER_LOOKUP (fadd, int),
    AMO_HANDLER_LOOKUP (fadd, long),
    AMO_HANDLER_LOOKUP (fadd, longlong),

    AMO_HANDLER_LOOKUP (finc, int),
    AMO_HANDLER_LOOKUP (finc, long),
    AMO_HANDLER_LOOKUP (finc, longlong),

    AMO_HANDLER_LOOKUP (add, int),
    AMO_HANDLER_LOOKUP (add, long),
    AMO_HANDLER_LOOKUP (add, longlong),

    AMO_HANDLER_LOOKUP (inc, int),
    AMO_HANDLER_LOOKUP (inc, long),
    AMO_HANDLER_LOOKUP (inc, longlong),


#if defined(HAVE_FEATURE_EXPERIMENTAL)
    AMO_HANDLER_LOOKUP (xor, int),
    AMO_HANDLER_LOOKUP (xor, long),
    AMO_HANDLER_LOOKUP (xor, longlong),

    AMO_HANDLER_LOOKUP (fetch, int),
    AMO_HANDLER_LOOKUP (fetch, long),
    AMO_HANDLER_LOOKUP (fetch, longlong),

    AMO_HANDLER_LOOKUP (set, int),
    AMO_HANDLER_LOOKUP (set, long),
    AMO_HANDLER_LOOKUP (set, longlong),
#endif /* HAVE_FEATURE_EXPERIMENTAL */

#if defined(HAVE_MANAGED_SEGMENTS)
    {GASNET_HANDLER_globalvar_put_out, handler_globalvar_put_out},
    {GASNET_HANDLER_globalvar_put_bak, handler_globalvar_put_bak},
    {GASNET_HANDLER_globalvar_get_out, handler_globalvar_get_out},
    {GASNET_HANDLER_globalvar_get_bak, handler_globalvar_get_bak},
#endif /* HAVE_MANAGED_SEGMENTS */
    {GASNET_HANDLER_globalexit_out, handler_globalexit_out}
    /* no reply partner for global_exit */
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

static int argc = 0;
static char **argv = NULL;

static const char *cmdline = "/proc/self/cmdline";
static const char *cmdline_fmt = "/proc/%ld/cmdline";

static inline void
parse_cmdline (void)
{
    FILE *fp;
    char an_arg[1024];          /* TODO: arbitrary size */
    char *p = an_arg;
    int i = 0;
    int c;

    /*
     * try to find this process' command-line:
     * either from short-cut, or from pid
     */
    fp = fopen (cmdline, "r");
    if (EXPR_UNLIKELY (fp == NULL)) {
        char pidname[MAXPATHLEN];
        snprintf (pidname, MAXPATHLEN, cmdline_fmt, getpid ());
        fp = fopen (pidname, "r");
        if (EXPR_UNLIKELY (fp == NULL)) {
            comms_bailout ("could not discover process' command-line (%s)",
                           strerror (errno)
                );
            /* NOT REACHED */
        }
    }

    /* first count the number of nuls in cmdline to see how many args */
    while ((c = fgetc (fp)) != EOF) {
        if (c == '\0') {
            argc += 1;
        }
    }
    rewind (fp);

    argv = (char **) malloc ((argc + 1) * sizeof (*argv));
    if (EXPR_UNLIKELY (argv == (char **) NULL)) {
        comms_bailout
            ("internal error: unable to allocate memory for faked command-line arguments");
        /* NOT REACHED */
    }

    while (1) {
        int c = fgetc (fp);
        switch (c) {
        case EOF:              /* end of args */
            argv[i] = NULL;
            goto end;
            break;
        case '\0':             /* end of this arg */
            *p = c;
            argv[i++] = strdup (an_arg);    /* unchecked return */
            p = an_arg;
            break;
        default:               /* copy out char in this arg */
            *p++ = c;
            break;
        }
    }
  end:
    fclose (fp);
}

static inline void
release_cmdline (void)
{
    if (argv != NULL) {
        int i;
        for (i = 0; i < argc; i += 1) {
            if (argv[i] != NULL) {
                free (argv[i]);
            }
        }
    }
}

/**
 * GASNet does this timeout thing if its collective routines
 * (e.g. barrier) go idle, so make this as long as possible
 */
static inline void
maximize_gasnet_timeout (void)
{
    char buf[32];
    snprintf (buf, 32, "%d", INT_MAX - 1);
    setenv ("GASNET_ EXITTIMEOUT", buf, 1);
}

/**
 * -----------------------------------------------------------------------
 */

/**
 * bail out of run-time with STATUS error code
 */
static inline void
shmemi_comms_exit (int status)
{
    /*
     * calling multiple times is undefined, I'm just going to do nothing
     */
    if (EXPR_UNLIKELY (GET_STATE (pe_status) == PE_SHUTDOWN)) {
        return;
    }

    /* ok, no more pending I/O ... */
    shmemi_comms_barrier_all ();

    release_cmdline ();

    shmemi_service_finalize ();

    /* clean up atomics and memory */
    shmemi_atomic_finalize ();
    shmemi_symmetric_memory_finalize ();
    shmemi_symmetric_globalvar_table_finalize ();

    /* clean up plugin modules */
    /* shmemi_modules_finalize (); */

    /* tidy up binary inspector */
    shmemi_executable_finalize ();

    /* stop run time clock */
    shmemi_elapsed_clock_finalize ();

    /* update our state */
    SET_STATE (pe_status, PE_SHUTDOWN);

    shmemi_trace (SHMEM_LOG_FINALIZE,
                  "finalizing shutdown, handing off to communications layer");

    /*
     * TODO, tc: need to be better at cleanup for 1.2, since finalize
     * doesn't imply follow-on exit, merely end of OpenSHMEM portion.
     *
     */

    /* shmemi_comms_barrier_all (); */
}

/**
 * finalize can now happen in 2 ways: (1) program finishes via
 * atexit(), or (2) user explicitly calls shmem_finalize().  Need to
 * detect explicit call and not terminate program until exit.
 *
 */
static void
shmemi_comms_finalize (void)
{
    shmemi_comms_exit (EXIT_SUCCESS);
}

/**
 * This is where the communications layer gets set up and torn down
 */
static inline void
shmemi_comms_init (void)
{
    /*
     * prepare environment for GASNet
     */
    parse_cmdline ();
    maximize_gasnet_timeout ();

    GASNET_SAFE (gasnet_init (&argc, &argv));

    /* now we can ask about the node count & heap */
    SET_STATE (mype, shmemi_comms_mynode ());
    SET_STATE (numpes, shmemi_comms_nodes ());
    SET_STATE (heapsize, shmemi_comms_get_segment_size ());

    /*
     * not guarding the attach for different gasnet models,
     * since last 2 params are ignored if not needed
     */
    GASNET_SAFE (gasnet_attach (handlers, nhandlers,
                                GET_STATE (heapsize), 0)
                 );

    /* set up any locality information */
    place_init ();

    /* fire up any needed progress management */
    shmemi_service_init ();

    /* enable messages */
    shmemi_elapsed_clock_init ();
    shmemi_tracers_init ();

    /* who am I? */
    shmemi_executable_init ();

    /* find global symbols */
    shmemi_symmetric_globalvar_table_init ();

    /* handle the heap */
    shmemi_symmetric_memory_init ();

    /* which message/trace levels are active */
    shmemi_maybe_tracers_show_info ();
    shmemi_tracers_show ();

    /* set up the atomic ops handling */
    shmemi_atomic_init ();

    /* initialize collective algs */
    shmemi_barrier_dispatch_init ();
    shmemi_barrier_all_dispatch_init ();
    shmemi_broadcast_dispatch_init ();
    shmemi_collect_dispatch_init ();
    shmemi_fcollect_dispatch_init ();

    /* register shutdown handler */
    if (EXPR_UNLIKELY (atexit (shmemi_comms_finalize) != 0)) {
        shmemi_trace (SHMEM_LOG_FATAL,
                      "internal error: cannot register OpenSHMEM finalize handler");
        /* NOT REACHED */
    }

    SET_STATE (pe_status, PE_RUNNING);

    /* Up and running! */
}

/* mcs-lock.c */

/*
 * ------------------------------------------------------------------
 *
 * Low-level lock routines
 *
 */

/*
 *    Copyright (c) 1996-2002 by Quadrics Supercomputers World Ltd.
 *    Copyright (c) 2003-2005 by Quadrics Ltd.
 *
 *    For licensing information please see the supplied COPYING file
 *
 */

/**
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

typedef struct
{
    union
    {
        struct
        {
            volatile uint16_t locked;   /* boolean to indicate current state
                                           of lock */
            volatile int16_t next;  /* vp of next requestor */
        } s;
        volatile uint32_t word;
    } u;
#define l_locked        u.s.locked
#define l_next          u.s.next
#define l_word          u.word
} SHMEM_LOCK;

enum
{
    SHMEM_LOCK_FREE = -1,
    SHMEM_LOCK_RESET,
    SHMEM_LOCK_SET
};

/* Macro to map lock virtual address to owning process vp */
#define LOCK_OWNER(LOCK) ( ((uintptr_t)(LOCK) >> 3) % (GET_STATE (numpes)) )

static inline void
shmemi_comms_lock_acquire (SHMEM_LOCK * node, SHMEM_LOCK * lock, int this_pe)
{
    SHMEM_LOCK tmp;
    long locked;
    int prev_pe;

    node->l_next = SHMEM_LOCK_FREE;

    /* Form our lock request (integer) */
    tmp.l_locked = 1;
    tmp.l_next = this_pe;

    LOAD_STORE_FENCE ();

    /*
     * Swap this_pe into the global lock owner, returning previous
     * value, atomically
     */
    tmp.l_word =
        shmem_int_swap ((int *) &lock->l_word, tmp.l_word, LOCK_OWNER (lock));

    /* Translate old (broken) default lock state */
    if (tmp.l_word == SHMEM_LOCK_FREE) {
        tmp.l_word = SHMEM_LOCK_RESET;
    }

    /* Extract the global lock (tail) state */
    prev_pe = tmp.l_next;
    locked = tmp.l_locked;

    /* Is the lock held by someone else ? */
    if (locked) {
        /*
         * This flag gets cleared (remotely) once the lock is dropped
         */
        node->l_locked = 1;

        LOAD_STORE_FENCE ();

        /*
         * I'm now next in global linked list, update l_next in the
         * prev_pe process with our vp
         */
        shmem_short_p ((short *) &node->l_next, this_pe, prev_pe);

        /* Wait for flag to be released */
        GASNET_BLOCKUNTIL (!(node->l_locked));
    }
}

static inline void
shmemi_comms_lock_release (SHMEM_LOCK * node, SHMEM_LOCK * lock, int this_pe)
{
    /* Is there someone on the linked list ? */
    if (node->l_next == SHMEM_LOCK_FREE) {
        SHMEM_LOCK tmp;

        /* Form the remote atomic compare value (int) */
        tmp.l_locked = 1;
        tmp.l_next = this_pe;

        /*
         * If global lock owner value still equals this_pe, load RESET
         * into it & return prev value
         */
        tmp.l_word = shmem_int_cswap ((int *) &lock->l_word,
                                      tmp.l_word,
                                      SHMEM_LOCK_RESET, LOCK_OWNER (lock));

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
        GASNET_BLOCKUNTIL (!
                           ((node->l_next == SHMEM_LOCK_FREE) ||
                            (node->l_next < 0))
            );

    }

    /* Be more strict about the test above, this memory consistency problem is
       a tricky one */
    GASNET_BLOCKUNTIL (!(node->l_next < 0));

    /*
     * Release any waiters on the linked list
     */

    shmem_short_p ((short *) &node->l_locked, 0, node->l_next);
}


/*
 * I am not sure this is strictly correct. The Cray man pages suggest
 * that this routine should not block. With this implementation we could
 * race with another PE doing the same and then block in the acquire
 * Perhaps a conditional swap at the beginning would fix it ??
 *
 * (addy 12.10.05)
 */
static inline int
shmemi_comms_lock_test (SHMEM_LOCK * node, SHMEM_LOCK * lock, int this_pe)
{
    SHMEM_LOCK tmp;
    int retval;

    /* Read the remote global lock value */
    tmp.l_word = shmem_int_g ((int *) &lock->l_word, LOCK_OWNER (lock));

    /* Translate old (broken) default lock state */
    if (tmp.l_word == SHMEM_LOCK_FREE)
        tmp.l_word = SHMEM_LOCK_RESET;

    /* If lock already set then return 1, otherwise grab the lock & return 0 */
    if (tmp.l_word == SHMEM_LOCK_RESET) {
        shmemi_comms_lock_acquire (node, lock, this_pe);
        retval = 0;
    }
    else {
        retval = 1;
    }

    return retval;
}

/* end: mcs-lock.c */
