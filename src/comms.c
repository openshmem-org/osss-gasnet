/*
 * This file provides the layer on top of GASNet, ARMCI or whatever.
 * API should be formalized at some point, but basically everything
 * non-static that starts with "__comms_"
 */

#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>

#include "gasnet_safe.h"

#include "state.h"
#include "memalloc.h"
#include "warn.h"
#include "dispatch.h"
#include "atomic.h"
#include "comms.h"

#include "shmem.h"

#if defined(GASNET_SEGMENT_FAST) || defined(GASNET_SEGMENT_LARGE)
#  define HAVE_MANAGED_SEGMENTS 1
#elif defined(GASNET_SEGMENT_EVERYTHING)
#  undef HAVE_MANAGED_SEGMENTS
#else
#  error "I don't know what kind of GASNet segment model you're trying to use"
#endif


static gasnet_seginfo_t *seginfo_table;

#if ! defined(HAVE_MANAGED_SEGMENTS)

/*
 * this will be malloc'ed so we can respect setting from environment
 * variable
 */

#define DEFAULT_HEAP_SIZE 10485760L

static void *great_big_heap;

#endif /* ! HAVE_MANAGED_SEGMENTS */

/*
 * define accepted size units.  Table set up to favor expected units
 */

struct unit_lookup {
  char unit;
  size_t size;
};

static struct unit_lookup units[] =
  {
    { 'g', 1024L * 1024L * 1024L         },
    { 'm', 1024L * 1024L                 },
    { 'k', 1024L                         },
    { 't', 1024L * 1024L * 1024L * 1024L }
  };
static const int nunits = sizeof(units) / sizeof(units[0]);

/*
 * work out how big the symmetric segment areas should be.
 *
 * Either from environment setting, or default value from
 * implementation
 */
static size_t
__comms_get_segment_size(void)
{
  char unit = '\0';
  size_t mul = 1;
  char *p;
  char *mlss_str = __comms_getenv("SHMEM_SYMMETRIC_HEAP_SIZE");

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

  if (unit != '\0') {
    int i;
    int foundit = 0;
    struct unit_lookup *up = (struct unit_lookup *) units;

    unit = tolower(unit);
    for (i = 0; i < nunits; up += 1, i += 1) {
      if (up->unit == unit) {	/* walk the table, assign if unit matches */
	mul = up->size;
	foundit = 1;
	break;
      }
    }
    if (! foundit) {
      __shmem_warn(SHMEM_LOG_FATAL,
		   "unknown data size unit \"%c\" in symmetric heap specification",
		   unit);
    }
  }

  return mul * (size_t) strtol(mlss_str, (char **) NULL, 10);
}

/*
 * allow the runtime to change the spin/block behavior dynamically,
 * would allow adaptivity
 */
void
__comms_set_waitmode(int mode)
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
    __shmem_warn(SHMEM_LOG_FATAL,
		 "tried to set unknown wait mode %d", mode);
    /* NOT REACHED */
    break;
  }

  GASNET_SAFE( gasnet_set_waitmode(gm) );

  __shmem_warn(SHMEM_LOG_DEBUG,
	       "set waitmode to %s",
	       mstr);
}

/*
 * used in wait loops to poll for put/get/AM traffic
 */
void
__comms_poll(void)
{
  gasnet_AMPoll();
}

/*
 * make sure everyone finishes stuff, then exit.
 */
void
__comms_shutdown(int status)
{
  __comms_barrier_all();
  gasnet_exit(status);
}

/*
 * can't just call getenv, it might not pass through environment
 * info to other nodes from launch.
 */
char *
__comms_getenv(const char *name)
{
  return gasnet_getenv(name);
}

/*
 * which node (PE) am I?
 */
int
__comms_mynode(void)
{
  return (int) gasnet_mynode();
}

/*
 * how many nodes (Pes) take part in this program?
 */
int
__comms_nodes(void)
{
  return (int) gasnet_nodes();
}

/*
 * we use the _nbi routine, so that gasnet tracks outstanding
 * I/O for us (fence/barrier waits for these implicit handles)
 */
void
__comms_put(void *dst, void *src, size_t len, int pe)
{
  gasnet_put_nbi(pe, dst, src, len);
}

void
__comms_get(void *dst, void *src, size_t len, int pe)
{
  gasnet_get(dst, pe, src, len);
}

/*
 * not completely sure about using longs in these two:
 * it's big enough and hides the gasnet type: is that good enough?
 */

void
__comms_put_val(void *dst, long src, size_t len, int pe)
{
  gasnet_put_nbi_val(pe, dst, src, len);
}

long
__comms_get_val(void *src, size_t len, int pe)
{
  return gasnet_get_val(pe, src, len);
}

#define COMMS_TYPE_PUT_NB(Name, Type)					\
  void *								\
  __comms_##Name##_put_nb(Type *target, Type *source, size_t len, int pe) \
  {									\
    return gasnet_put_nb(pe, target, source, sizeof(Type) * len);	\
  }

COMMS_TYPE_PUT_NB(short, short)
COMMS_TYPE_PUT_NB(int, int)
COMMS_TYPE_PUT_NB(long, long)
COMMS_TYPE_PUT_NB(longdouble, long double)
COMMS_TYPE_PUT_NB(longlong, long long)
COMMS_TYPE_PUT_NB(double, double)
COMMS_TYPE_PUT_NB(float, float)

_Pragma("weak __comms_putmem_nb=__comms_long_put_nb")

void
__comms_wait_nb(void *h)
{
  gasnet_wait_syncnb((gasnet_handle_t) h);

  LOAD_STORE_FENCE();
}

void
__comms_fence(void)
{
  gasnet_wait_syncnbi_all();
  LOAD_STORE_FENCE();
}

#if 1

static int barcount = 0;
static int barflag = 0; // GASNET_BARRIERFLAG_ANONYMOUS;

void
__comms_barrier_all(void)
{
  // GASNET_BEGIN_FUNCTION();

  /* wait for gasnet to finish pending puts/gets */
  gasnet_wait_syncnbi_all();

  /* use gasnet's global barrier */
  gasnet_barrier_notify(barcount, barflag);
  GASNET_SAFE( gasnet_barrier_wait(barcount, barflag) );

  // barcount = 1 - barcount;
  barcount += 1;
}

#else

void
__comms_barrier_all(void)
{
  __comms_barrier(0, 0, __state.numpes, syncit);
}

#endif



void
__comms_barrier(int PE_start, int logPE_stride, int PE_size, long *pSync)
{
  __comms_fence();

  if (__state.mype == PE_start) {
    const int step = 1 << logPE_stride;
    int i;
    int thatpe;
    /* root signals everyone else */
    for (thatpe = PE_start, i = 1; i < PE_size; i += 1) {
      thatpe += step;
      shmem_long_p(& pSync[thatpe], ~ _SHMEM_SYNC_VALUE, thatpe);
    }
    /* root waits for ack from everyone else */
    for (thatpe = PE_start, i = 1; i < PE_size; i += 1) {
      thatpe += step;
      shmem_wait(& pSync[thatpe], ~ _SHMEM_SYNC_VALUE);
    }
  }
  else {
    /* non-root waits for root to signal, then tell root we're ready */
    shmem_wait(& pSync[__state.mype], _SHMEM_SYNC_VALUE);
    shmem_long_p(& pSync[__state.mype], _SHMEM_SYNC_VALUE, PE_start);
  }
  /* restore pSync values */
  pSync[__state.mype] = _SHMEM_SYNC_VALUE;
  __comms_fence();
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

#define GASNET_HANDLER_GLOBALVAR_OUT 130
#define GASNET_HANDLER_GLOBALVAR_BAK 131

static void handler_globalvar_out();
static void handler_globalvar_bak();

#endif /* ! HAVE_MANAGED_SEGMENTS */

#define GASNET_HANDLER_SWAP_OUT      132
#define GASNET_HANDLER_SWAP_BAK      133

static void handler_swap_out();
static void handler_swap_bak();

#define GASNET_HANDLER_CSWAP_OUT     134
#define GASNET_HANDLER_CSWAP_BAK     135

static void handler_cswap_out();
static void handler_cswap_bak();

static gasnet_handlerentry_t handlers[] =
  {
#if ! defined(HAVE_MANAGED_SEGMENTS)
    { GASNET_HANDLER_SETUP_OUT,     handler_segsetup_out  },
    { GASNET_HANDLER_SETUP_BAK,     handler_segsetup_bak  },
    { GASNET_HANDLER_GLOBALVAR_OUT, handler_globalvar_out },
    { GASNET_HANDLER_GLOBALVAR_BAK, handler_globalvar_bak },
#endif /* ! HAVE_MANAGED_SEGMENTS */
    { GASNET_HANDLER_SWAP_OUT,      handler_swap_out      },
    { GASNET_HANDLER_SWAP_BAK,      handler_swap_bak      },
    { GASNET_HANDLER_CSWAP_OUT,     handler_cswap_out     },
    { GASNET_HANDLER_CSWAP_BAK,     handler_cswap_bak     }
  };
static const int nhandlers = sizeof(handlers) / sizeof(handlers[0]);

/*
 * end of handlers
 */

/*
 * This is where the communications layer gets set up
 */
void
__comms_init(void)
{
  /*
   * fake the command-line args
   */
  int argc = 1;
  char **argv;

  argv = (char **) malloc(argc * sizeof(*argv));
  if (argv == (char **) NULL) {
    __shmem_warn(SHMEM_LOG_FATAL,
                 "could not allocate memory for GASNet initialization"
		 );
    /* NOT REACHED */
  }
  argv[0] = "shmem";
  
  GASNET_SAFE( gasnet_init(&argc, &argv) );

  /*
   * now we can ask about the node count & heap
   */
  __state.mype     = __comms_mynode();
  __state.numpes   = __comms_nodes();
  __state.heapsize = __comms_get_segment_size();

  /*
   * not guarding the attach for different gasnet models,
   * since last 2 params are ignored if not needed
   */
  GASNET_SAFE(
	      gasnet_attach(handlers, nhandlers,
			    __state.heapsize, 0
			    )
	      );

  __comms_set_waitmode(SHMEM_COMMS_SPINBLOCK);

  /*
   * make sure all nodes are up to speed before "declaring"
   * initialization done
   */
  __comms_barrier_all();

  __shmem_warn(SHMEM_LOG_INIT,
	       "initialization complete"
	       );

  /* Up and running! */
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

  // gasnet_hsl_lock(& setup_out_lock);

  GASNET_SAFE( gasnet_AMGetMsgSource(token, &src_pe) );

  seginfo_table[(int) src_pe].addr = gsp->addr;
  seginfo_table[(int) src_pe].size = gsp->size;

  // gasnet_hsl_unlock(& setup_out_lock);

  gasnet_AMReplyMedium1(token, GASNET_HANDLER_SETUP_BAK,
			(void *) NULL, 0, unused);
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

void
__symmetric_memory_init(void)
{
  /*
   * calloc zeroes for us
   */
  seginfo_table = (gasnet_seginfo_t *) calloc(__state.numpes,
					      sizeof(gasnet_seginfo_t));
  if (seginfo_table == (gasnet_seginfo_t *) NULL) {
    __shmem_warn(SHMEM_LOG_FATAL,
                 "could not allocate GASNet segments (%s)",
		 strerror(errno)
		 );
    /* NOT REACHED */
  }

  /*
   * prep the segments for use across all PEs
   *
   * each PE manages its own segment, but can see addresses from all PEs
   */

#ifdef HAVE_MANAGED_SEGMENTS

  GASNET_SAFE( gasnet_getSegmentInfo(seginfo_table, __state.numpes) );

#else

  /* allocate the heap - has to be pagesize aligned */
  if (posix_memalign(& great_big_heap,
		     GASNET_PAGESIZE, __state.heapsize) != 0) {
    __shmem_warn(SHMEM_LOG_FATAL,
		 "unable to allocate symmetric heap"
		 );
    /* NOT REACHED */
  }

  __shmem_warn(SHMEM_LOG_MEMORY,
	       "symmetric heap @ %p, size is %ld bytes",
	       great_big_heap, __state.heapsize
	       );

  {
    gasnet_seginfo_t gsp;
    int pe;
    for (pe = 0; pe < __state.numpes; pe += 1) {
      /* send to everyone else */
      if (__state.mype != pe) {

	gsp.addr = great_big_heap;
        gsp.size = __state.heapsize;

	gasnet_AMRequestMedium1(pe, GASNET_HANDLER_SETUP_OUT,
				&gsp, sizeof(gsp),
				0);
      }
    }
    /* messages swirl around...do local init then wait for responses */

    /*
     * store my own heap entry
     */
    seginfo_table[__state.mype].addr = great_big_heap;
    seginfo_table[__state.mype].size = __state.heapsize;

    /*
     * initialize my heap
     */
    __mem_init(seginfo_table[__state.mype].addr,
	       seginfo_table[__state.mype].size);

    {
      /* now wait on the AM replies */
      int got_all = __state.numpes - 2; /* 0-based AND don't count myself */
      do {
	__comms_poll();
      } while (seg_setup_replies_received <= got_all);
    }
  }

#endif /* HAVE_MANAGED_SEGMENTS */

  /* and make sure everyone is up-to-speed */
  __comms_barrier_all();

  /*
   * spit out the seginfo table (but check first that the loop is
   * warranted)
   */
  if (__warn_is_enabled(SHMEM_LOG_INIT)) {
    int pe;
    for (pe = 0; pe < __state.numpes; pe += 1) {
      __shmem_warn(SHMEM_LOG_INIT,
		   "seginfo_table[%d] = ( addr = %p, size = %ld )",
		   pe,
		   seginfo_table[pe].addr,
		   seginfo_table[pe].size
		   );
    }
  }
}

void
__symmetric_memory_finalize(void)
{
  __mem_finalize();
  free(great_big_heap);
}

/*
 * where the symmetric memory starts on the given PE
 */
void *
__symmetric_var_base(int pe)
{
  return seginfo_table[pe].addr;
}

/*
 * is the address in the managed symmetric area?
 */
int
__symmetric_var_in_range(void *addr, int pe)
{
  int retval;

  if (addr < seginfo_table[pe].addr) {
    retval = 0;
  }
  else if (addr > (seginfo_table[pe].addr + seginfo_table[pe].size)) {
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
__symmetric_var_offset(void *dest, int pe)
{
  size_t offset = (size_t) dest - (size_t) __symmetric_var_base(__state.mype);
  char *rdest = (char *) __symmetric_var_base(pe) + offset;

  if (__symmetric_var_in_range(rdest, pe)) {
    return (void *) rdest;
  }
  else {
    /* TODO: assume remotely accessible global for now, but should ELF check */
    return dest;
  }
}

/*
 * -- swap handlers --
 */
static gasnet_hsl_t swap_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t swap_bak_lock = GASNET_HSL_INITIALIZER;

/*
 * NB we make the cond/value "long long" throughout
 * to be used by smaller types as self-contained payload
 */

typedef struct {
  void *s_symm_addr;		/* sender symmetric var */
  void *r_symm_addr;		/* recipient symmetric var */
  int sentinel;			/* end of transaction marker */
  int *sentinel_addr;	        /* addr of marker for copied payload */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be swapped */
} swap_payload_t;

static gasnet_hsl_t cswap_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t cswap_bak_lock = GASNET_HSL_INITIALIZER;

typedef struct {
  void *s_symm_addr;		/* sender symmetric var */
  void *r_symm_addr;		/* recipient symmetric var */
  int sentinel;			/* end of transaction marker */
  int *sentinel_addr;	        /* addr of marker for copied payload */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be swapped */
  long long cond;		/* conditional value */
} cswap_payload_t;

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

  gasnet_hsl_lock(& swap_out_lock);

#if 0
  /* debugging */
  fprintf(stderr, "PP: pp->s_symm_addr = %p\n", pp->s_symm_addr);
  fprintf(stderr, "PP: pp->r_symm_addr = %p\n", pp->r_symm_addr);
  fprintf(stderr, "PP: pp->value       = %p\n", pp->value);
  fprintf(stderr, "PP: pp->sentinel    = %d\n", pp->sentinel);
  fprintf(stderr, "PP: pp->nbytes      = %ld\n", pp->nbytes);
#endif

  /* save and update */
  (void) memcpy(&old, pp->r_symm_addr, pp->nbytes);
  (void) memcpy(pp->r_symm_addr, &(pp->value), pp->nbytes);
  pp->value = old;

  gasnet_hsl_unlock(& swap_out_lock);

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

  gasnet_hsl_lock(& swap_bak_lock);

  /* save returned value */
  (void) memcpy(pp->s_symm_addr, &(pp->value), pp->nbytes);

  /* done it */
  *(pp->sentinel_addr) = 1;

  gasnet_hsl_unlock(& swap_bak_lock);
}

void
__comms_swap_request(void *target, void *value, size_t nbytes, int pe, void *retval)
{
  swap_payload_t *p = (swap_payload_t *) malloc(sizeof(*p));
  if (p == (swap_payload_t *) NULL) {
    __shmem_warn(SHMEM_LOG_FATAL,
		 "internal error: unable to allocate swap payload memory"
		 );
  }
  /* build payload to send */
  p->s_symm_addr = target;
  p->r_symm_addr = __symmetric_var_offset(target, pe);
  p->nbytes = nbytes;
  p->value = *(long long *) value;
  p->sentinel = 0;
  p->sentinel_addr = &(p->sentinel);

  /* send and wait for ack */
  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_SWAP_OUT,
			  p, sizeof(*p),
			  0);
  GASNET_BLOCKUNTIL(p->sentinel);

  /* local store */
  (void) memcpy(retval, &(p->value), nbytes);

  free(p);
}

/*
 * called by remote PE to do the swap.  Store new value if cond
 * matches, send back old value in either case
 */
static void
handler_cswap_out(gasnet_token_t token,
		  void *buf, size_t bufsiz,
		  gasnet_handlerarg_t unused)
{
  long long old;
  cswap_payload_t *pp = (cswap_payload_t *) buf;

  gasnet_hsl_lock(& cswap_out_lock);

  /* save current target */
  old = *(long long *) pp->r_symm_addr;
  /* update value if cond matches */
  if ( *(long long *) pp->cond == *(long long *) pp->r_symm_addr) {
    *(long long *) pp->r_symm_addr = pp->value;
  }
  /* return value */
  pp->value = old;

  gasnet_hsl_unlock(& cswap_out_lock);

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

  gasnet_hsl_lock(& cswap_bak_lock);

  /* save returned value */
  (void) memcpy(pp->s_symm_addr, &(pp->value), pp->nbytes);

  /* done it */
  *(pp->sentinel_addr) = 1;

  gasnet_hsl_unlock(& cswap_bak_lock);
}

void
__comms_cswap_request(void *target, void *cond, void *value, size_t nbytes,
		      int pe,
		      void *retval)
{
  cswap_payload_t *cp = (cswap_payload_t *) malloc(sizeof(*cp));
  if (cp == (cswap_payload_t *) NULL) {
    __shmem_warn(SHMEM_LOG_FATAL,
		 "internal error: unable to allocate conditional swap payload memory"
		 );
  }
  /* build payload to send */
  cp->s_symm_addr = target;
  cp->r_symm_addr = __symmetric_var_offset(target, pe);
  cp->nbytes = nbytes;
  cp->value = *(long long *) value;
  cp->cond = *(long long *) cond;
  cp->sentinel = 0;
  cp->sentinel_addr = &(cp->sentinel);

  /* send and wait for ack */
  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_CSWAP_OUT,
			  cp, sizeof(*cp),
			  0);
  GASNET_BLOCKUNTIL(cp->sentinel);

  /* local store */
  (void) memcpy(retval, &(cp->value), nbytes);

  free(cp);
}

/*
 * ---------------------------------------------------------------------------
 */

#if ! defined(HAVE_MANAGED_SEGMENTS)

/*
 * global variable put/get handlers (for non-everything cases):
 *
 * 1. sender AMs remote with address of variable to write into, total size to write, and
 * current offset (like seek), then the data itself.
 *
 * 2. remote acks
 *
 * (repeat as needed if data size > max request length, updating offset)
 *
 * 3. done?
 *
 * HOW DO WE HANDLE WAITING FOR OUTSTANDING OPS IN THIS FRAMEWORK FOR BARRIERS etc.?
 * INSERT MEMBAR?
 *
 */

static gasnet_hsl_t globalvar_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t globalvar_bak_lock = GASNET_HSL_INITIALIZER;

typedef struct {
  void *var_addr;		/* address of global var to be written to on remote PE */
  long var_size;		/* size of data to be written (so we can allocate remotely) */
  long offset;		        /* where we are in the write process */
  void *data;			/* the actual data to be sent */
  int sentinel;			/* completion marker */
  int *sentinel_addr;		/* addr of symmetric completion marker */
} globalvar_payload_t;


/* TODO: these are all stubs, still in thinking phase */

/*
 * called by remote PE to grab and write to its variable
 */
static void
handler_globalvar_out(gasnet_token_t token,
		      void *buf, size_t bufsiz,
		      gasnet_handlerarg_t unused)
{
#if 0
  globalvar_payload_t *pp = (globalvar_payload_t *) buf;

  gasnet_hsl_lock(& globalvar_out_lock);

  gasnet_hsl_unlock(& globalvar_out_lock);

  /* return the updated payload */
  gasnet_AMReplyLong(token, GASNET_HANDLER_GLOBALVAR_BAK, buf, bufsiz, unused);
#endif
}

/*
 * called by sender PE after remote has set up temp buffer.
 * Sends data across to remote.
 */
static void
handler_globalvar_bak(gasnet_token_t token,
		      void *buf, size_t bufsiz,
		      gasnet_handlerarg_t unused)
{
#if 0
  globalvar_payload_t *pp = (globalvar_payload_t *) buf;

  gasnet_hsl_lock(& globalvar_bak_lock);

  *(pp->sentinel_addr) = 1;

  gasnet_hsl_unlock(& globalvar_bak_lock);
#endif
}

void
__comms_globalvar_translation(void *target, long value, int pe)
{
#if 0
  globalvar_payload_t *p = (globalvar_payload_t *) malloc(sizeof(*p));

  if (p == (globalvar_payload_t *) NULL) {
    __shmem_warn(SHMEM_LOG_FATAL,
		 "internal error: unable to allocate heap variable payload memory"
		 );
  }

  p->sentinel = 0;
  p->sentinel_addr = &(p->sentinel); /* this one, not the copy */

  gasnet_AMRequestLong1(pe, GASNET_HANDLER_GLOBALVAR_OUT,
			p, sizeof(*p),
			0);

  GASNET_BLOCKUNTIL(p->sentinel);

  free(p);
#endif
}

#endif /* ! HAVE_MANAGED_SEGMENTS */
