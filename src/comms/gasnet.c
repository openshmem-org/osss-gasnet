/* (c) 2011 University of Houston.  All rights reserved. */


/*
 * This file provides the layer on top of GASNet, ARMCI or whatever.
 * API should be formalized at some point, but basically everything
 * non-static that starts with "__shmem_comms_"
 */

#define _POSIX_C_SOURCE 199309
#include <time.h>		/* for nanosleep */
#undef _POSIX_C_SOURCE
#include <math.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <values.h>

#include <gasnet.h>

#include "uthash.h"

#include "state.h"
#include "memalloc.h"
#include "trace.h"
#include "atomic.h"
#include "comms.h"
#include "ping.h"
#include "utils.h"
#include "exe.h"

/*
 * gasnet put model: this is just for testing different put
 * emulations; generally we want the nbi routines to get performance.
 *
 */

#define USING_IMPLICIT_HANDLES

#ifdef USING_IMPLICIT_HANDLES
# define GASNET_PUT(pe, dst, src, len)     gasnet_put_nbi(pe, dst, src, len)
# define GASNET_PUT_VAL(pe, dst, src, len) gasnet_put_nbi_val(pe, dst, src, len)
# define GASNET_WAIT_PUTS()                gasnet_wait_syncnbi_puts()
#else
# define GASNET_PUT(pe, dst, src, len)     gasnet_put(pe, dst, src, len)
# define GASNET_PUT_VAL(pe, dst, src, len) gasnet_put_val(pe, dst, src, len)
# define GASNET_WAIT_PUTS()
#endif /* USING_IMPLICIT_HANDLES */

/*
 * gasnet model choice
 *
 */

#if defined(GASNET_SEGMENT_FAST) || defined(GASNET_SEGMENT_LARGE)
#  define HAVE_MANAGED_SEGMENTS 1
#elif defined(GASNET_SEGMENT_EVERYTHING)
#  undef HAVE_MANAGED_SEGMENTS
#else
#  error "I don't know what kind of GASNet segment model you're trying to use"
#endif

/*
 * set up segment/symmetric handling
 *
 */

static gasnet_seginfo_t *seginfo_table;

#if ! defined(HAVE_MANAGED_SEGMENTS)

/*
 * this will be malloc'ed so we can respect setting from environment
 * variable
 */

#define DEFAULT_HEAP_SIZE 2000000000L /* 2G */

static void *great_big_heap;

#endif /* ! HAVE_MANAGED_SEGMENTS */

/*
 * trap gasnet errors gracefully
 *
 */
#define GASNET_SAFE(fncall) do {                                        \
    int _retval;                                                        \
    if ((_retval = fncall) != GASNET_OK) {                              \
      __shmem_trace(SHMEM_LOG_FATAL,					\
		   "error calling: %s at %s:%i, %s (%s)\n",		\
		   #fncall, __FILE__, __LINE__,				\
		   gasnet_ErrorName(_retval),				\
		   gasnet_ErrorDesc(_retval));				\
    }                                                                   \
  } while(0)

/*
 * --------------- real work starts here ---------------------
 *
 */

/*
 * define accepted size units in ascending order, which are
 * S.I. compliant
 *
 * http://en.wikipedia.org/wiki/SI_Unit_Prefixes
 *
 */

static char *units_string = "kmgtpezy";

static const size_t multiplier = 1000L;

/*
 * work out how big the symmetric segment areas should be.
 *
 * Either from environment setting, or default value from
 * implementation
 */
static size_t
__shmem_comms_get_segment_size(void)
{
  char unit = '\0';
  size_t bytes = 1L;
  char *p;
  char *mlss_str = __shmem_comms_getenv("SHMEM_SYMMETRIC_HEAP_SIZE");

  if (mlss_str == (char *) NULL) {
#ifdef HAVE_MANAGED_SEGMENTS
    return (size_t) gasnet_getMaxLocalSegmentSize();
#else
    return DEFAULT_HEAP_SIZE;
#endif
  }

  p = mlss_str;
  while (*p != '\0') {
    if (! isdigit(*p)) {
      unit = *p;
      *p = '\0';		/* get unit, chop */
      break;
    }
    p += 1;
  }

  /* if there's a unit, work out how much to scale */
  if (unit != '\0') {
    int i;
    int foundit = 0;
    char *usp = units_string;

    unit = tolower(unit);
    while (*usp != '\0') {
      bytes *= multiplier;
      if (*usp == unit) {
	foundit = 1;
	break;
      }
      usp += 1;
    }

    if (! foundit) {
      /* don't know that unit! */
      __shmem_trace(SHMEM_LOG_FATAL,
		    "unknown data size unit \"%c\" in symmetric heap specification",
		    unit);
      /* NOT REACHED */
    }
  }

  return bytes * (size_t) strtol(mlss_str, (char **) NULL, 10);
}

/*
 * allow the runtime to change the spin/block behavior dynamically,
 * would allow adaptivity
 */
void
__shmem_comms_set_waitmode(comms_spinmode_t mode)
{
  int gm;
  const char *mstr;

  switch (mode) {
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
    __shmem_trace(SHMEM_LOG_FATAL,
		  "tried to set unknown wait mode %d",
		  (int) mode
		  );
    /* NOT REACHED */
    break;
  }

  GASNET_SAFE( gasnet_set_waitmode(gm) );

  __shmem_trace(SHMEM_LOG_DEBUG,
		"set waitmode to %s",
		mstr
		);
}

/*
 * traffic progress
 *
 */
void
__shmem_comms_pause(void)
{
  GASNET_SAFE( gasnet_AMPoll() );
}

/*
 * can't just call getenv, it might not pass through environment
 * info to other nodes from launch.
 */
char *
__shmem_comms_getenv(const char *name)
{
  return gasnet_getenv(name);
}

/*
 * which node (PE) am I?
 */
int
__shmem_comms_mynode(void)
{
  return (int) gasnet_mynode();
}

/*
 * how many nodes (PEs) take part in this program?
 */
int
__shmem_comms_nodes(void)
{
  return (int) gasnet_nodes();
}

/*
 * we use the _nbi routine, so that gasnet tracks outstanding
 * I/O for us (fence/barrier waits for these implicit handles)
 */

void __shmem_comms_globalvar_put_request(); /* forward decl */

void
__shmem_comms_put(void *dst, void *src, size_t len, int pe)
{
  
#if defined(HAVE_MANAGED_SEGMENTS)
  if (__shmem_symmetric_is_globalvar(dst)) {
    __shmem_comms_globalvar_put_request(dst, src, len, pe);
  }
  else {
    GASNET_PUT(pe, dst, src, len);
  }
#else
  GASNET_PUT(pe, dst, src, len);
#endif /* HAVE_MANAGED_SEGMENTS */
}

void __shmem_comms_globalvar_get_request(); /* forward decl */

void
__shmem_comms_get(void *dst, void *src, size_t len, int pe)
{
#if defined(HAVE_MANAGED_SEGMENTS)
  if (__shmem_symmetric_is_globalvar(src)) {
    __shmem_comms_globalvar_get_request(dst, src, len, pe);
  }
  else {
    gasnet_get(dst, pe, src, len);
  }
#else
  gasnet_get(dst, pe, src, len);
#endif /* HAVE_MANAGED_SEGMENTS */
}

/*
 * not completely sure about using longs in these two:
 * it's big enough and hides the gasnet type: is that good enough?
 */

void
__shmem_comms_put_val(void *dst, long src, size_t len, int pe)
{
#if defined(HAVE_MANAGED_SEGMENTS)
  if (__shmem_symmetric_is_globalvar(dst)) {
    __shmem_comms_globalvar_put_request(dst, & src, len, pe);
  }
  else {
    GASNET_PUT_VAL(pe, dst, src, len);
  }
#else
  GASNET_PUT_VAL(pe, dst, src, len);
#endif /* HAVE_MANAGED_SEGMENTS */
}

long
__shmem_comms_get_val(void *src, size_t len, int pe)
{
#if defined(HAVE_MANAGED_SEGMENTS)
  if (__shmem_symmetric_is_globalvar(src)) {
    long retval;
    __shmem_comms_globalvar_get_request(&retval, src, len, pe);
    return retval;
  }
  else {
    return gasnet_get_val(pe, src, len);
  }
#else
  return gasnet_get_val(pe, src, len);
#endif /* HAVE_MANAGED_SEGMENTS */
}

/*
 * ---------------------------------------------------------------------------
 *
 * non-blocking puts: not part of current API
 */
#define COMMS_TYPE_PUT_NB(Name, Type)					\
  void *								\
  __shmem_comms_##Name##_put_nb(Type *target, const Type *source, size_t len, int pe) \
  {									\
    return gasnet_put_nb_bulk(pe, target, (Type *) source, sizeof(Type) * len);	\
  }

COMMS_TYPE_PUT_NB(short, short)
COMMS_TYPE_PUT_NB(int, int)
COMMS_TYPE_PUT_NB(long, long)
COMMS_TYPE_PUT_NB(longdouble, long double)
COMMS_TYPE_PUT_NB(longlong, long long)
COMMS_TYPE_PUT_NB(double, double)
COMMS_TYPE_PUT_NB(float, float)

#pragma weak __shmem_comms_putmem_nb = __shmem_comms_long_put_nb

#define COMMS_TYPE_GET_NB(Name, Type)					\
  void *								\
  __shmem_comms_##Name##_get_nb(Type *target, const Type *source, size_t len, int pe) \
  {									\
    return gasnet_get_nb_bulk(target, pe, (Type *) source, sizeof(Type) * len);	\
  }

COMMS_TYPE_GET_NB(short, short)
COMMS_TYPE_GET_NB(int, int)
COMMS_TYPE_GET_NB(long, long)
COMMS_TYPE_GET_NB(longdouble, long double)
COMMS_TYPE_GET_NB(longlong, long long)
COMMS_TYPE_GET_NB(double, double)
COMMS_TYPE_GET_NB(float, float)

#pragma weak __shmem_comms_getmem_nb = __shmem_comms_long_get_nb

void
__shmem_comms_wait_nb(void *h)
{
  gasnet_wait_syncnb((gasnet_handle_t) h);
  LOAD_STORE_FENCE();
}

int
__shmem_comms_test_nb(void *h)
{
  int s = gasnet_try_syncnb((gasnet_handle_t) h);
  return (s == GASNET_OK) ? 1 : 0;
}

/*
 * ---------------------------------------------------------------------------
 *
 * global barrier done through gasnet
 *
 */

static int barcount = 0;
static int barflag = 0;

void
__shmem_comms_barrier_all(void)
{
  /* GASNET_BEGIN_FUNCTION(); */

  /* use gasnet's global barrier */
  gasnet_barrier_notify(barcount, barflag);
  GASNET_SAFE( gasnet_barrier_wait(barcount, barflag) );

  /* barcount = 1 - barcount; */
  barcount += 1;
}


/*
 * ---------------------------------------------------------------------------
 *
 * start of handlers
 */

#if ! defined(HAVE_MANAGED_SEGMENTS)

#define GASNET_HANDLER_SETUP_OUT     128
#define GASNET_HANDLER_SETUP_BAK     129

static void handler_segsetup_out();
static void handler_segsetup_bak();

#endif /* ! HAVE_MANAGED_SEGMENTS */

#define GASNET_HANDLER_SWAP_OUT      130
#define GASNET_HANDLER_SWAP_BAK      131

static void handler_swap_out();
static void handler_swap_bak();

#define GASNET_HANDLER_CSWAP_OUT     132
#define GASNET_HANDLER_CSWAP_BAK     133

static void handler_cswap_out();
static void handler_cswap_bak();

#define GASNET_HANDLER_FADD_OUT      134
#define GASNET_HANDLER_FADD_BAK      135

static void handler_fadd_out();
static void handler_fadd_bak();

#define GASNET_HANDLER_FINC_OUT      136
#define GASNET_HANDLER_FINC_BAK      137

static void handler_finc_out();
static void handler_finc_bak();

#define GASNET_HANDLER_ADD_OUT       138
#define GASNET_HANDLER_ADD_BAK       139

static void handler_add_out();
static void handler_add_bak();

#define GASNET_HANDLER_INC_OUT       140
#define GASNET_HANDLER_INC_BAK       141

static void handler_inc_out();
static void handler_inc_bak();

#define GASNET_HANDLER_PING_OUT      142
#define GASNET_HANDLER_PING_BAK      143

static void handler_ping_out();
static void handler_ping_bak();

#if 0 /* not used */

#define GASNET_HANDLER_QUIET_OUT     144
#define GASNET_HANDLER_QUIET_BAK     145

static void handler_quiet_out();
static void handler_quiet_bak();

#endif /* not used */

#if defined(HAVE_MANAGED_SEGMENTS)

#define GASNET_HANDLER_GLOBALVAR_PUT_OUT 160
#define GASNET_HANDLER_GLOBALVAR_PUT_BAK 161

static void handler_globalvar_put_out();
static void handler_globalvar_put_bak();

#define GASNET_HANDLER_GLOBALVAR_GET_OUT 162
#define GASNET_HANDLER_GLOBALVAR_GET_BAK 163

static void handler_globalvar_get_out();
static void handler_globalvar_get_bak();

#endif /* HAVE_MANAGED_SEGMENTS */

static gasnet_handlerentry_t handlers[] =
  {
#if ! defined(HAVE_MANAGED_SEGMENTS)
    { GASNET_HANDLER_SETUP_OUT,         handler_segsetup_out      },
    { GASNET_HANDLER_SETUP_BAK,         handler_segsetup_bak      },
#endif /* ! HAVE_MANAGED_SEGMENTS */
    { GASNET_HANDLER_SWAP_OUT,          handler_swap_out          },
    { GASNET_HANDLER_SWAP_BAK,          handler_swap_bak          },
    { GASNET_HANDLER_CSWAP_OUT,         handler_cswap_out         },
    { GASNET_HANDLER_CSWAP_BAK,         handler_cswap_bak         },
    { GASNET_HANDLER_FADD_OUT,          handler_fadd_out          },
    { GASNET_HANDLER_FADD_BAK,          handler_fadd_bak          },
    { GASNET_HANDLER_FINC_OUT,          handler_finc_out          },
    { GASNET_HANDLER_FINC_BAK,          handler_finc_bak          },
    { GASNET_HANDLER_ADD_OUT,           handler_add_out           },
    { GASNET_HANDLER_ADD_BAK,           handler_add_bak           },
    { GASNET_HANDLER_INC_OUT,           handler_inc_out           },
    { GASNET_HANDLER_INC_BAK,           handler_inc_bak           },
    { GASNET_HANDLER_PING_OUT,          handler_ping_out          },
    { GASNET_HANDLER_PING_BAK,          handler_ping_bak          },
#if 0 /* not used */
    { GASNET_HANDLER_QUIET_OUT,         handler_quiet_out         },
    { GASNET_HANDLER_QUIET_BAK,         handler_quiet_bak         },
#endif /* not used */
#if defined(HAVE_MANAGED_SEGMENTS)
    { GASNET_HANDLER_GLOBALVAR_PUT_OUT, handler_globalvar_put_out },
    { GASNET_HANDLER_GLOBALVAR_PUT_BAK, handler_globalvar_put_bak },
    { GASNET_HANDLER_GLOBALVAR_GET_OUT, handler_globalvar_get_out },
    { GASNET_HANDLER_GLOBALVAR_GET_BAK, handler_globalvar_get_bak },
#endif /* HAVE_MANAGED_SEGMENTS */
  };
static const int nhandlers = TABLE_SIZE(handlers);

/*
 * end of handlers
 */

/*
 * -----------------------------------------------------------------------
 *
 * This is where the communications layer gets set up and torn down
 */

static int argc;
static char **argv;

void
__shmem_comms_init(void)
{
  /*
   * fake the command-line args
   */
  argc = 1;
  argv = (char **) malloc(argc * sizeof(*argv));
  if (argv == (char **) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: could not allocate memory for GASNet initialization"
		  );
    /* NOT REACHED */
  }
  argv[0] = GET_STATE(exe_name);

  /*
   * let's get gasnet up and running
   */  
  GASNET_SAFE( gasnet_init(&argc, &argv) );

  /*
   * now we can ask about the node count & heap
   */
  SET_STATE(mype,     __shmem_comms_mynode()           );
  SET_STATE(numpes,   __shmem_comms_nodes()            );
  SET_STATE(heapsize, __shmem_comms_get_segment_size() );

  /*
   * not guarding the attach for different gasnet models,
   * since last 2 params are ignored if not needed
   */
  GASNET_SAFE(
	      gasnet_attach(handlers, nhandlers,
			    GET_STATE(heapsize), 0
			    )
	      );

  __shmem_trace(SHMEM_LOG_INIT,
		"gasnet attached, %d handlers registered, heap = %ld bytes",
		nhandlers,
		GET_STATE(heapsize)
		);

  __shmem_comms_set_waitmode(SHMEM_COMMS_SPINBLOCK);

  /*
   * make sure all nodes are up to speed before "declaring"
   * initialization done
   */
  __shmem_comms_barrier_all();

  __shmem_trace(SHMEM_LOG_INIT,
		"communication layer initialization complete"
		);

  /* Up and running! */
}

/*
 * bail out of run-time with STATUS error code
 */
void
__shmem_comms_exit(int status)
{
  gasnet_exit(status);
}

/*
 * make sure everyone finishes stuff, then exit with STATUS.
 */
void
__shmem_comms_finalize(int status)
{
  if (argv != NULL) {
    free(argv);
  }

  __shmem_comms_exit(status);
}

/*
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

/*
 * remotely modified, stop it being put in a register
 */
static volatile int seg_setup_replies_received = 0;

static gasnet_hsl_t setup_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t setup_bak_lock = GASNET_HSL_INITIALIZER;

/*
 * unpack buf from sender PE and store seg info locally.  Ack. receipt.
 */
static void
handler_segsetup_out(gasnet_token_t token,
		     void *buf, size_t bufsiz,
		     gasnet_handlerarg_t unused)
{
  gasnet_node_t src_pe;
  gasnet_seginfo_t *gsp = (gasnet_seginfo_t *) buf;

  /*
   * no lock here: each PE writes exactly once to its own array index,
   * and only to that...
   */

  /* gasnet_hsl_lock(& setup_out_lock); */

  GASNET_SAFE( gasnet_AMGetMsgSource(token, &src_pe) );

  seginfo_table[(int) src_pe].addr = gsp->addr;
  seginfo_table[(int) src_pe].size = gsp->size;

  /* gasnet_hsl_unlock(& setup_out_lock); */

  gasnet_AMReplyMedium1(token, GASNET_HANDLER_SETUP_BAK,
			(void *) NULL, 0, unused
			);
}

/*
 * record receipt ack.  We only need to count the number of replies
 */
static void
handler_segsetup_bak(gasnet_token_t token,
		     void *buf, size_t bufsiz,
		     gasnet_handlerarg_t unused)
{
  gasnet_hsl_lock(& setup_bak_lock);

  seg_setup_replies_received += 1;

  gasnet_hsl_unlock(& setup_bak_lock);
}

#endif /* ! HAVE_MANAGED_SEGMENTS */

/*
 * initialize the symmetric memory, taking into account the different
 * gasnet configurations
 */

void
__shmem_symmetric_memory_init(void)
{
  const int me          = GET_STATE(mype);
  const int npes        = GET_STATE(numpes);
  const size_t heapsize = GET_STATE(heapsize);
  int pm_r;

  /*
   * calloc zeroes for us
   */
  seginfo_table = (gasnet_seginfo_t *) calloc(npes, sizeof(gasnet_seginfo_t));
  if (seginfo_table == (gasnet_seginfo_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "could not allocate GASNet segments (%s)",
		  strerror(errno)
		  );
    /* NOT REACHED */
  }

  /*
   * prep the segments for use across all PEs
   *
   */

#ifdef HAVE_MANAGED_SEGMENTS

  /* gasnet handles the segment allocation for us */
  GASNET_SAFE( gasnet_getSegmentInfo(seginfo_table, npes) );

#else

  /* allocate the heap - has to be pagesize aligned */
  pm_r = posix_memalign(& great_big_heap, GASNET_PAGESIZE, heapsize);
  if (pm_r != 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "unable to allocate symmetric heap (%s)",
		  strerror(pm_r)
		  );
    /* NOT REACHED */
  }

  /* everyone has their local info before exchanging messages */
  __shmem_comms_barrier_all();

  __shmem_trace(SHMEM_LOG_MEMORY,
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

    for (pe = 0; pe < npes; pe += 1) {
      /* send to everyone else */
      if (me != pe) {
	gasnet_AMRequestMedium1(pe, GASNET_HANDLER_SETUP_OUT,
				&gs, sizeof(gs),
				0
				);
      }
    }

    /* now wait on the AM replies (0-based AND don't count myself) */
    GASNET_BLOCKUNTIL(seg_setup_replies_received == npes - 1);

    __shmem_trace(SHMEM_LOG_MEMORY,
		  "received all replies (%d)",
		  seg_setup_replies_received
		  );

  }

#endif /* HAVE_MANAGED_SEGMENTS */

  /* initialize my heap */
  __shmem_mem_init(seginfo_table[me].addr,
		   seginfo_table[me].size
		   );

  /* and make sure everyone is up-to-speed */
  __shmem_comms_barrier_all();

  /*
   * spit out the seginfo table (but check first that the loop is
   * warranted)
   */
  if (__shmem_trace_is_enabled(SHMEM_LOG_INIT)) {
    int pe;
    for (pe = 0; pe < npes; pe += 1) {
      __shmem_trace(SHMEM_LOG_INIT,
		    "cross-check: segment[%d] = { .addr = %p, .size = %ld }",
		    pe,
		    seginfo_table[pe].addr,
		    seginfo_table[pe].size
		    );
    }
  }
}

/*
 * shut down the memory allocation handler
 */
void
__shmem_symmetric_memory_finalize(void)
{
  __shmem_mem_finalize();
#if ! defined(HAVE_MANAGED_SEGMENTS)
  free(great_big_heap);
#endif /* HAVE_MANAGED_SEGMENTS */
}

/*
 * where the symmetric memory starts on the given PE
 */
void *
__shmem_symmetric_var_base(int p)
{
  return seginfo_table[p].addr;
}

/*
 * is the address in the managed symmetric area?
 */
int
__shmem_symmetric_var_in_range(void *addr, int pe)
{
  int retval;

  if (addr < seginfo_table[pe].addr) {
    __shmem_trace(SHMEM_LOG_MEMORY,
		  "addr %p < seginfo %p",
		  addr,
		  seginfo_table[pe].addr
		  );
    retval = 0;
  }
  else if (addr > (seginfo_table[pe].addr + seginfo_table[pe].size)) {
    __shmem_trace(SHMEM_LOG_MEMORY,
		  "addr %p > seginfo + size %p",
		  addr,
		  seginfo_table[pe].addr + seginfo_table[pe].size
		  );
    retval = 0;
  }
  else {
    retval = 1;
  }

  return retval;
}

/*
 * translate my "dest" to corresponding address on PE "pe"
 */
void *
__shmem_symmetric_addr_lookup(void *dest, int pe)
{
  const int me = GET_STATE(mype);
  size_t offset;
  char *rdest;

  /* globals are in same place everywhere */
  if (__shmem_symmetric_is_globalvar(dest)) {
    return dest;
  }

  /* not symmetric if outside of heap */
  if (! __shmem_symmetric_var_in_range(dest, me)) {
    return NULL;
  }

  /* where this is in *my* heap */
  offset = (char *) dest - (char *) __shmem_symmetric_var_base(me);
  /* and where it is in the remote heap */
  rdest = (char *) __shmem_symmetric_var_base(pe) + offset;

  /* assume this is good */
  return rdest;
}

/*
 * -- lock finding/creating utility --
 */

typedef struct {
  void *addr;
  gasnet_hsl_t *lock;
  UT_hash_handle hh;		/* makes this structure hashable */
} lock_table_t;

static lock_table_t *lock_table = NULL;

/*
 * Look up the lock for a given address ADDR.  If ADDR has never been
 * seen before, create the lock for it.
 *
 */
static gasnet_hsl_t *
get_lock_for(void *addr)
{
  lock_table_t *try;

  HASH_FIND_PTR(lock_table, &addr, try);

  if (try == (lock_table_t *) NULL) {
    gasnet_hsl_t *L = (gasnet_hsl_t *) malloc(sizeof(*L));

    if (L == (gasnet_hsl_t *) NULL) {
      __shmem_trace(SHMEM_LOG_FATAL,
		    "internal error: unable to allocate lock for address %p",
		    addr
		    );
      /* NOT REACHED */
    }

    try = (lock_table_t *) malloc(sizeof(*try));
    if (try == (lock_table_t *) NULL) {
      __shmem_trace(SHMEM_LOG_FATAL,
		    "internal error: unable to allocate lock table entry for address %p",
		    addr
		    );
      /* NOT REACHED */
    }

    gasnet_hsl_init(L);

    try->addr = addr;
    try->lock = L;

    HASH_ADD_PTR(lock_table, addr, try);

    __shmem_trace(SHMEM_LOG_LOCK,
		  "created new lock for address %p",
		  addr
		  );
  }
  else {
    __shmem_trace(SHMEM_LOG_LOCK,
		  "already have a lock for address %p",
		  addr
		  );
  }    

  return try->lock;
}

/*
 * -- swap handlers ---------------------------------------------------------
 */

/*
 * NB we make the cond/value "long long" throughout
 * to be used by smaller types as self-contained payload
 */

typedef struct {
  void *local_store;		/* sender saves here */
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be swapped */
} swap_payload_t;

/*
 * called by remote PE to do the swap.  Store new value, send back old value
 */
static void
handler_swap_out(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  long long old;
  swap_payload_t *pp = (swap_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for(pp->r_symm_addr);

  gasnet_hsl_lock(lk);

  /* save and update */
  (void) memmove(&old, pp->r_symm_addr, pp->nbytes);
  (void) memmove(pp->r_symm_addr, &(pp->value), pp->nbytes);
  pp->value = old;

  LOAD_STORE_FENCE();

  gasnet_hsl_unlock(lk);

  /* return updated payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_SWAP_BAK, buf, bufsiz, unused);
}

/*
 * called by swap invoker when old value returned by remote PE
 */
static void
handler_swap_bak(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  swap_payload_t *pp = (swap_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for(pp->r_symm_addr);

  gasnet_hsl_lock(lk);

  /* save returned value */
  (void) memmove(pp->local_store, &(pp->value), pp->nbytes);

  LOAD_STORE_FENCE();

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(lk);
}

/*
 * perform the swap
 */
void
__shmem_comms_swap_request(void *target, void *value, size_t nbytes,
			   int pe,
			   void *retval)
{
  swap_payload_t *p = (swap_payload_t *) malloc(sizeof(*p));
  if (p == (swap_payload_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate swap payload memory"
		  );
  }
  /* build payload to send */
  p->local_store = retval;
  p->r_symm_addr = __shmem_symmetric_addr_lookup(target, pe);
  p->nbytes = nbytes;
  p->value = *(long long *) value;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* send and wait for ack */
  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_SWAP_OUT,
			  p, sizeof(*p),
			  0);

  WAIT_ON_COMPLETION(p->completed);

  free(p);
}

typedef struct {
  void *local_store;		/* sender saves here */
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr; /* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be swapped */
  long long cond;		/* conditional value */
} cswap_payload_t;

/*
 * called by remote PE to do the swap.  Store new value if cond
 * matches, send back old value in either case
 */
static void
handler_cswap_out(gasnet_token_t token,
		  void *buf, size_t bufsiz,
		  gasnet_handlerarg_t unused)
{
  void *old;
  cswap_payload_t *pp = (cswap_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for(pp->r_symm_addr);

  gasnet_hsl_lock(lk);

  old = malloc(pp->nbytes);

  /* save current target */
  memmove(old, pp->r_symm_addr, pp->nbytes);

  /* update value if cond matches */
  if (memcmp(&(pp->cond), pp->r_symm_addr, pp->nbytes) == 0) {
    memmove(pp->r_symm_addr, &(pp->value), pp->nbytes);
  }
  /* return value */
  memmove(&(pp->value), old, pp->nbytes);

  LOAD_STORE_FENCE();

  free(old);

  gasnet_hsl_unlock(lk);

  /* return updated payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_CSWAP_BAK, buf, bufsiz, unused);
}

/*
 * called by swap invoker when old value returned by remote PE
 * (same as swap_bak for now)
 */
static void
handler_cswap_bak(gasnet_token_t token,
		  void *buf, size_t bufsiz,
		  gasnet_handlerarg_t unused)
{
  cswap_payload_t *pp = (cswap_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for(pp->r_symm_addr);

  gasnet_hsl_lock(lk);

  /* save returned value */
  (void) memmove(pp->local_store, &(pp->value), pp->nbytes);

  LOAD_STORE_FENCE();

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(lk);
}

/*
 * perform the conditional swap
 */
void
__shmem_comms_cswap_request(void *target, void *cond, void *value, size_t nbytes,
		      int pe,
		      void *retval)
{
  cswap_payload_t *cp = (cswap_payload_t *) malloc(sizeof(*cp));
  if (cp == (cswap_payload_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate conditional swap payload memory"
		  );
  }
  /* build payload to send */
  cp->local_store = retval;
  cp->r_symm_addr = __shmem_symmetric_addr_lookup(target, pe);
  cp->nbytes = nbytes;
  cp->value = cp->cond = 0LL;
  memmove(&(cp->value), value, nbytes);
  memmove(&(cp->cond), cond, nbytes);
  cp->completed = 0;
  cp->completed_addr = &(cp->completed);

  LOAD_STORE_FENCE();

  /* send and wait for ack */
  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_CSWAP_OUT,
			  cp, sizeof(*cp),
			  0);

  WAIT_ON_COMPLETION(cp->completed);

  free(cp);
}

/*
 * fetch/add
 */

typedef struct {
  void *local_store;		/* sender saves here */
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be added & then return old */
} fadd_payload_t;

/*
 * called by remote PE to do the fetch and add.  Store new value, send
 * back old value
 */
static void
handler_fadd_out(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  long long old = 0;
  long long plus = 0;
  fadd_payload_t *pp = (fadd_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for(pp->r_symm_addr);

  gasnet_hsl_lock(lk);

  /* save and update */
  (void) memmove(&old, pp->r_symm_addr, pp->nbytes);
  plus = old + pp->value;
  (void) memmove(pp->r_symm_addr, &plus, pp->nbytes);
  pp->value = old;

  LOAD_STORE_FENCE();

  gasnet_hsl_unlock(lk);

  /* return updated payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_FADD_BAK, buf, bufsiz, unused);
}

/*
 * called by fadd invoker when old value returned by remote PE
 */
static void
handler_fadd_bak(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  fadd_payload_t *pp = (fadd_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for(pp->r_symm_addr);

  gasnet_hsl_lock(lk);

  /* save returned value */
  (void) memmove(pp->local_store, &(pp->value), pp->nbytes);

  LOAD_STORE_FENCE();

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(lk);
}

/*
 * perform the fetch-and-add
 */
void
__shmem_comms_fadd_request(void *target, void *value, size_t nbytes, int pe, void *retval)
{
  fadd_payload_t *p = (fadd_payload_t *) malloc(sizeof(*p));
  if (p == (fadd_payload_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate fetch-and-add payload memory"
		  );
  }
  /* build payload to send */
  p->local_store = retval;
  p->r_symm_addr = __shmem_symmetric_addr_lookup(target, pe);
  p->nbytes = nbytes;
  p->value = *(long long *) value;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* send and wait for ack */
  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_FADD_OUT,
			  p, sizeof(*p),
			  0);

  WAIT_ON_COMPLETION(p->completed);

  free(p);
}

/*
 * fetch/increment
 */

typedef struct {
  void *local_store;		/* sender saves here */
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be returned */
} finc_payload_t;

/*
 * called by remote PE to do the fetch and increment.  Store new
 * value, send back old value
 */
static void
handler_finc_out(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  long long old = 0;
  long long plus = 1;
  finc_payload_t *pp = (finc_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for(pp->r_symm_addr);

  gasnet_hsl_lock(lk);

  /* save and update */
  (void) memmove(&old, pp->r_symm_addr, pp->nbytes);
  plus = old + 1;
  (void) memmove(pp->r_symm_addr, &plus, pp->nbytes);
  pp->value = old;

  LOAD_STORE_FENCE();

  gasnet_hsl_unlock(lk);

  /* return updated payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_FINC_BAK, buf, bufsiz, unused);
}

/*
 * called by finc invoker when old value returned by remote PE
 */
static void
handler_finc_bak(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  finc_payload_t *pp = (finc_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for(pp->r_symm_addr);

  gasnet_hsl_lock(lk);

  /* save returned value */
  (void) memmove(pp->local_store, &(pp->value), pp->nbytes);

  LOAD_STORE_FENCE();

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(lk);
}

/*
 * perform the fetch-and-increment
 */
void
__shmem_comms_finc_request(void *target, size_t nbytes, int pe, void *retval)
{
  finc_payload_t *p = (finc_payload_t *) malloc(sizeof(*p));
  if (p == (finc_payload_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate fetch-and-increment payload memory"
		  );
  }
  /* build payload to send */
  p->local_store = retval;
  p->r_symm_addr = __shmem_symmetric_addr_lookup(target, pe);
  p->nbytes = nbytes;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* send and wait for ack */
  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_FINC_OUT,
			  p, sizeof(*p),
			  0);

  WAIT_ON_COMPLETION(p->completed);

  free(p);
}

/*
 * remote add
 */

typedef struct {
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be returned */
} add_payload_t;

/*
 * called by remote PE to do the remote add.
 */
static void
handler_add_out(gasnet_token_t token,
		void *buf, size_t bufsiz,
		gasnet_handlerarg_t unused)
{
  long long old = 0;
  long long plus = 0;
  add_payload_t *pp = (add_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for(pp->r_symm_addr);

  gasnet_hsl_lock(lk);

  /* save and update */
  (void) memmove(&old, pp->r_symm_addr, pp->nbytes);
  plus = old + pp->value;
  (void) memmove(pp->r_symm_addr, &plus, pp->nbytes);

  LOAD_STORE_FENCE();

  gasnet_hsl_unlock(lk);

  /* return updated payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_ADD_BAK, buf, bufsiz, unused);
}

/*
 * called by remote add invoker when store done
 */
static void
handler_add_bak(gasnet_token_t token,
		void *buf, size_t bufsiz,
		gasnet_handlerarg_t unused)
{
  add_payload_t *pp = (add_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for(pp->r_symm_addr);

  gasnet_hsl_lock(lk);

  LOAD_STORE_FENCE();

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(lk);
}

/*
 * perform the add
 */
void
__shmem_comms_add_request(void *target, void *value, size_t nbytes, int pe)
{
  add_payload_t *p = (add_payload_t *) malloc(sizeof(*p));
  if (p == (add_payload_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate remote add payload memory"
		  );
  }
  /* build payload to send */
  p->r_symm_addr = __shmem_symmetric_addr_lookup(target, pe);
  p->nbytes = nbytes;
  p->value = *(long long *) value;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* send and wait for ack */
  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_ADD_OUT,
			  p, sizeof(*p),
			  0);

  WAIT_ON_COMPLETION(p->completed);

  free(p);
}

/*
 * remote increment
 */

typedef struct {
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
  size_t nbytes;		/* how big the value is */
} inc_payload_t;

static gasnet_hsl_t inc_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t inc_bak_lock = GASNET_HSL_INITIALIZER;

/*
 * called by remote PE to do the remote increment
 */
static void
handler_inc_out(gasnet_token_t token,
		void *buf, size_t bufsiz,
		gasnet_handlerarg_t unused)
{
  long long old = 0;
  long long plus = 0;
  inc_payload_t *pp = (inc_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for(pp->r_symm_addr);

  gasnet_hsl_lock(lk);

  /* save and update */
  (void) memmove(&old, pp->r_symm_addr, pp->nbytes);
  plus = old + 1;
  (void) memmove(pp->r_symm_addr, &plus, pp->nbytes);
  LOAD_STORE_FENCE();

  __shmem_trace(SHMEM_LOG_ATOMIC,
		"%lld -> %lld",
		old, plus
		);

  gasnet_hsl_unlock(lk);

  /* return updated payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_INC_BAK, buf, bufsiz, unused);
}

/*
 * called by remote increment invoker when store done
 */
static void
handler_inc_bak(gasnet_token_t token,
		void *buf, size_t bufsiz,
		gasnet_handlerarg_t unused)
{
  inc_payload_t *pp = (inc_payload_t *) buf;
  gasnet_hsl_t *lk = get_lock_for(pp->r_symm_addr);

  gasnet_hsl_lock(lk);

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(lk);
}

/*
 * perform the increment
 */
void
__shmem_comms_inc_request(void *target, size_t nbytes, int pe)
{
  inc_payload_t *p = (inc_payload_t *) malloc(sizeof(*p));
  if (p == (inc_payload_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate remote increment payload memory"
		  );
  }
  /* build payload to send */
  p->r_symm_addr = __shmem_symmetric_addr_lookup(target, pe);
  p->nbytes = nbytes;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* send and wait for ack */
  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_INC_OUT,
			  p, sizeof(*p),
			  0);

  WAIT_ON_COMPLETION(p->completed);

  free(p);
}



/*
 * ---------------------------------------------------------------------------
 *
 * Handlers for pinging for shmem_pe_accessible
 *
 */

typedef struct {
  pe_status_t remote_pe_status;	/* health of remote PE */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
} ping_payload_t;

static int pe_acked = 1;

static jmp_buf jb;

/*
 * do this when the timeout occurs
 */
static void
ping_timeout_handler(int signum)
{
  pe_acked = 0;

  longjmp(jb, 1);
}

/*
 * can use single static lock here, no per-addr needed
 */
static gasnet_hsl_t ping_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t ping_bak_lock = GASNET_HSL_INITIALIZER;

/*
 * called by remote PE when (if) it gets the ping
 */
static void
handler_ping_out(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  ping_payload_t *pp = (ping_payload_t *) buf;

  gasnet_hsl_lock(& ping_out_lock);

  pp->remote_pe_status = PE_RUNNING;

  gasnet_hsl_unlock(& ping_out_lock);

  /* return ack'ed payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_PING_BAK, buf, bufsiz, unused);
}

/*
 * called by sender PE when (if) remote PE ack's the ping
 */
static void
handler_ping_bak(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  ping_payload_t *pp = (ping_payload_t *) buf;
  
  gasnet_hsl_lock(& ping_bak_lock);

  pe_acked = pe_acked && (pp->remote_pe_status == PE_RUNNING);

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(& ping_bak_lock);
}

/*
 * perform the ping
 */
int
__shmem_comms_ping_request(int pe)
{
  sighandler_t sig;
  int sj_status;
  ping_payload_t *p = (ping_payload_t *) malloc(sizeof(*p));
  if (p == (ping_payload_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate remote accessibility payload memory"
		  );
  }
  /* build payload to send */
  p->completed = 0;
  p->completed_addr = &(p->completed);
  p->remote_pe_status = PE_UNKNOWN;

  /* now the ping is ponged, or we timeout waiting... */
  sig = signal(SIGALRM, ping_timeout_handler);
  if (sig == SIG_ERR) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: registration of ping timeout handler failed"
		  );
    /* NOT REACHED */
  }

  /* hope for the best */
  pe_acked = 1;

  __shmem_ping_set_alarm();

  sj_status = setjmp(jb);

  /* only ping if we're coming through the first time */
  if (sj_status == 0) {
    /* send and wait for ack */
    gasnet_AMRequestMedium1(pe, GASNET_HANDLER_PING_OUT,
			    p, sizeof(*p),
			    0);

    WAIT_ON_COMPLETION(p->completed);
  }

  __shmem_ping_clear_alarm();

  sig = signal(SIGALRM, sig);
  if (sig == SIG_ERR) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: release of ping timeout handler failed"
		  );
    /* NOT REACHED */
  }

  free(p);

  return pe_acked;
}


#if 0 /* not used */

/*
 * ---------------------------------------------------------------------------
 */

typedef struct {
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
} quiet_payload_t;

/*
 * can use single static lock here, no per-addr needed
 */
static gasnet_hsl_t quiet_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t quiet_bak_lock = GASNET_HSL_INITIALIZER;

/*
 * called by remote PE to quiet and acknowledge
 */
static void
handler_quiet_out(gasnet_token_t token,
		  void *buf, size_t bufsiz,
		  gasnet_handlerarg_t unused)
{
  gasnet_hsl_lock(& quiet_out_lock);

  __shmem_comms_fence_request();

  gasnet_hsl_unlock(& quiet_out_lock);

  __shmem_trace(SHMEM_LOG_QUIET,
		"sending ack"
		);

  /* return ack, payload not modified in this case */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_QUIET_BAK, buf, bufsiz, unused);
}

/*
 * called to ack remote fence
 */
static void
handler_quiet_bak(gasnet_token_t token,
		  void *buf, size_t bufsiz,
		  gasnet_handlerarg_t unused)
{
  quiet_payload_t *pp = (quiet_payload_t *) buf;

  gasnet_hsl_lock(& quiet_bak_lock);

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(& quiet_bak_lock);

  __shmem_trace(SHMEM_LOG_QUIET,
		"ack'ed remote fence"
		);
}

/*
 * perform the quiet
 */
void
__shmem_comms_quiet_request(void)
{
  int other_pe;
  const int npes = GET_STATE(numpes);
  const int me   = GET_STATE(mype);
  quiet_payload_t **pa = (quiet_payload_t **) malloc(npes * sizeof(*pa));
  if (pa == (quiet_payload_t **) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate quiet payload array"
		  );
  }
  /* build payload to send */
  for (other_pe = 0; other_pe < npes; other_pe += 1) {
    quiet_payload_t *p = (quiet_payload_t *) malloc(sizeof(*p));
    if (me != other_pe) {
      p->completed = 0;
      p->completed_addr = &(p->completed);
      pa[other_pe] = p;

      /* send and wait for ack */
      gasnet_AMRequestMedium1(other_pe, GASNET_HANDLER_QUIET_OUT,
			      p, sizeof(*p),
			      0);
      __shmem_trace(SHMEM_LOG_QUIET,
		    "sent request to PE %d",
		    other_pe
		    );

    }
  }
  /* wait for all acks */
  for (other_pe = npes -1; other_pe >= 0; other_pe -= 1) {
    if (me != other_pe) {

      __shmem_trace(SHMEM_LOG_QUIET,
		    "waiting for ack from PE %d",
		    other_pe
		    );

      WAIT_ON_COMPLETION(pa[other_pe]->completed);

      free(pa[other_pe]);
    }
    else {
      __shmem_comms_fence_request();
    }
  }

  free(pa);
}

#endif /* not used */

/*
 * called by mainline to fence off outstanding requests
 */

void
__shmem_comms_fence_request(void)
{
  GASNET_WAIT_PUTS();

  LOAD_STORE_FENCE();

  return;
}


/*
 * ---------------------------------------------------------------------------
 *
 * global variable put/get handlers (for non-everything cases):
 *
 * TODO: locking feels too coarse-grained with static (single) buffer,
 * TODO: would love to find a better way of doing this bit
 *
 */

/*
 * suitably allocate buffer for transfers.
 *
 */

static void
allocate_buffer_and_check(void **buf, size_t siz)
{
  int r = posix_memalign(buf, GASNET_PAGESIZE, siz);
  switch (r) {
  case 0:
    /* all ok, return */
    break;
  case EINVAL:
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: global variable payload not aligned correctly"
		  );
    /* NOT REACHED */
    break;
  case ENOMEM:
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: no memory to allocate global variable payload"
		  );
    /* NOT REACHED */
    break;
  default:
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unknown error with global variable payload (posix_memalign returned %d)",
		  r
		  );
    /* NOT REACHED */
    break;
  }
}

#if defined(HAVE_MANAGED_SEGMENTS)

typedef struct {
  size_t nbytes;	        /* size of write */
  void *target;			/* where to write */
  void *source;			/* data we want to get */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	/* addr of marker */
} globalvar_payload_t;

/*
 * Puts
 */

/*
 * called by remote PE to grab and write to its variable
 */
static void
handler_globalvar_put_out(gasnet_token_t token,
			  void *buf, size_t bufsiz,
			  gasnet_handlerarg_t unused)
{
  globalvar_payload_t *pp = (globalvar_payload_t *) buf;
  void *data = buf + sizeof(*pp);

  memmove(pp->target, data, pp->nbytes);
  LOAD_STORE_FENCE();

  /* return ack, just need the control structure */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_GLOBALVAR_PUT_BAK,
			buf, sizeof(*pp), unused);
}

/*
 * invoking PE just has to ack remote write
 */
static void
handler_globalvar_put_bak(gasnet_token_t token,
			  void *buf, size_t bufsiz,
			  gasnet_handlerarg_t unused)
{
  globalvar_payload_t *pp = (globalvar_payload_t *) buf;

  *(pp->completed_addr) = 1;
}

/*
 * Generate the active message to do a put to a global variable.
 *
 * Put a lock around this bit so we know when it is safe to reuse the
 * buffer.
 *
 */
static void inline
put_a_chunk(void *buf, size_t bufsize,
	    void *target, void *source,
	    size_t offset, size_t bytes_to_send,
	    int pe)
{
  globalvar_payload_t *p = buf;
  void *data = buf + sizeof(*p);

  /*
   * build payload to send
   * (global var is trivially symmetric here, no translation needed)
   */
  p->nbytes         = bytes_to_send;
  p->source         = NULL; 		/* not used in put */
  p->target         = target + offset;	/* on the other PE */
  p->completed      = 0;
  p->completed_addr = &(p->completed);

  /* data added after control structure */
  memmove(data, source + offset, bytes_to_send);
  LOAD_STORE_FENCE();

  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_GLOBALVAR_PUT_OUT,
			  p, bufsize,
			  0);

  WAIT_ON_COMPLETION(p->completed);
}

/*
 * perform the put to a global variable
 */
void
__shmem_comms_globalvar_put_request(void *target, void *source, size_t nbytes, int pe)
{
  /* get the buffer size and chop off control structure */
  const size_t max_req = gasnet_AMMaxMedium();
  const size_t max_data = max_req - sizeof(globalvar_payload_t);
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

  allocate_buffer_and_check(& put_buf, alloc_size);

  if (nchunks > 0) {
    size_t i;

    for (i = 0; i < nchunks; i += 1) {
      put_a_chunk(put_buf, alloc_size,
		  target, source,
		  offset, payload_size,
		  pe
		  );
      offset += payload_size;
    }
  }

  if (rem_send > 0) {

    payload_size = rem_send;

    put_a_chunk(put_buf, alloc_size,
		target, source,
		offset, payload_size,
		pe
		);

  }

  free(put_buf);
}

/*
 * Gets
 *
 */

/*
 * called by remote PE to grab remote data and return
 */
static void
handler_globalvar_get_out(gasnet_token_t token,
			  void *buf, size_t bufsiz,
			  gasnet_handlerarg_t unused)
{
  globalvar_payload_t *pp = (globalvar_payload_t *) buf;
  globalvar_payload_t *datap = buf + sizeof(*pp);

  /* fetch from remote global var into payload */
  memmove(datap, pp->source, pp->nbytes);
  LOAD_STORE_FENCE();

  /* return ack, copied data is returned */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_GLOBALVAR_GET_BAK,
			buf, bufsiz, unused);
}

/*
 * called by invoking PE to write fetched data
 */
static void
handler_globalvar_get_bak(gasnet_token_t token,
			  void *buf, size_t bufsiz,
			  gasnet_handlerarg_t unused)
{
  globalvar_payload_t *pp = (globalvar_payload_t *) buf;

  /* write back payload data here */
  memmove(pp->target, buf + sizeof(*pp), pp->nbytes);
  LOAD_STORE_FENCE();

  *(pp->completed_addr) = 1;
}

/*
 * Generate the active message to do a get from a global variable.
 *
 */
static void inline
get_a_chunk(globalvar_payload_t *p, size_t bufsize,
	    void *target, void *source,
	    size_t offset, size_t bytes_to_send,
	    int pe)
{
  /*
   * build payload to send
   * (global var is trivially symmetric here, no translation needed)
   */
  p->nbytes         = bytes_to_send;
  p->source         = source + offset;	/* on the other PE */
  p->target         = target + offset;	/* track my local writes upon return */
  p->completed      = 0;
  p->completed_addr = &(p->completed);

  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_GLOBALVAR_GET_OUT,
			  p, bufsize,
			  0);

  WAIT_ON_COMPLETION(p->completed);
}

/*
 * perform the get from a global variable
 */
void
__shmem_comms_globalvar_get_request(void *target, void *source, size_t nbytes, int pe)
{
  /* get the buffer size and chop off control structure */
  const size_t max_req = gasnet_AMMaxMedium();
  const size_t max_data = max_req - sizeof(globalvar_payload_t);
  /* how to split up transfers */
  const size_t nchunks = nbytes / max_data;
  const size_t rem_send = nbytes % max_data;
  /* track size and progress of transfers */
  size_t payload_size;
  size_t alloc_size;
  size_t offset = 0;
  void *get_buf;

  alloc_size = max_req;

  allocate_buffer_and_check(& get_buf, alloc_size);

  if (nchunks > 0) {
    size_t i;

    payload_size = max_data;

    for (i = 0; i < nchunks; i += 1) {
      get_a_chunk(get_buf, alloc_size,
                  target, source,
                  offset, payload_size,
                  pe
                  );
      offset += payload_size;
    }
  }

  if (rem_send > 0) {

    payload_size = rem_send;

    get_a_chunk(get_buf, alloc_size,
                target, source,
                offset, payload_size,
                pe
                );

  }

  free(get_buf);
}

#endif /* HAVE_MANAGED_SEGMENTS */
