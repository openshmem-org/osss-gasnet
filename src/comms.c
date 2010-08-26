#include "gasnet_safe.h"

#include "state.h"
#include "symmem.h"
#include "warn.h"
#include "dispatch.h"
#include "atomic.h"
#include "comms.h"

static int barcount = 0;
static int barflag = 0; // GASNET_BARRIERFLAG_ANONYMOUS;

/*
 *start of handlers
 */

#define GASNET_HANDLER_SWAP_OUT 128
#define GASNET_HANDLER_SWAP_BAK 129

static gasnet_handlerentry_t handlers[] =
  {
    { GASNET_HANDLER_SWAP_OUT, handler_swap_out },
    { GASNET_HANDLER_SWAP_BAK, handler_swap_bak }
  };
static const int nhandlers = sizeof(handlers) / sizeof(handlers[0]);

/*
 * end of handlers
 */

void
__comms_barrier_all(void)
{
  GASNET_BEGIN_FUNCTION();
  gasnet_barrier_notify(barcount, barflag);
  GASNET_SAFE( gasnet_barrier_wait(barcount, barflag) );

  // barcount += 1;
}

void
__comms_init(void)
{
  /*
   * fake the command-line args
   */
  int argc = 1;
  char **argv;
  uintptr_t mlss;

  argv = (char **) malloc(argc * sizeof(*argv));
  if (argv == (char **) NULL) {
    __shmem_warn(SHMEM_LOG_FATAL,
                 "could not allocate memory for GASNet initialization");
  }
  argv[0] = "shmem";
  
  GASNET_SAFE(
	      gasnet_init(&argc, &argv)
	      );
  
  mlss = gasnet_getMaxLocalSegmentSize();

  GASNET_SAFE(
	      gasnet_attach(handlers, nhandlers,
			    mlss,
			    0)
	      );
  __shmem_warn(SHMEM_LOG_DEBUG,
	       "attached %d handler%s, segment size is %ld\n",
	       nhandlers,
	       (nhandlers == 1) ? "" : "s",
	       mlss);

  __comms_set_waitmode(SHMEM_COMMS_SPINBLOCK);

  __comms_barrier_all();
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

__inline__ void
__comms_poll(void)
{
  GASNET_BEGIN_FUNCTION();
  gasnet_AMPoll();
}

void
__comms_shutdown(int status)
{
  __comms_barrier_all();
  
  gasnet_exit(status);
}

int
__comms_mynode(void)
{
  return (int) gasnet_mynode();
}

int
__comms_nodes(void)
{
  return (int) gasnet_nodes();
}

void
__comms_put(void *dst, void *src, size_t len, int pe)
{
  gasnet_put(pe, dst, src, len);
}

void
__comms_get(void *dst, void *src, size_t len, int pe)
{
  gasnet_get(dst, pe, src, len);
}

void
__comms_put_val(void *dst, long src, size_t len, int pe)
{
  gasnet_put_val(pe, dst, src, len);
}

long
__comms_get_val(void *src, size_t len, int pe)
{
  return gasnet_get_val(pe, src, len);
}

/*
 * initialize the symmetric segments
 */

static gasnet_seginfo_t * seginfo_table;

/* UNUSED */
#define ROUNDUP(v, n) (((v) + (n)) & ~((n) - 1))

void
__symmetric_memory_init(void)
{
  seginfo_table = (gasnet_seginfo_t *)calloc(__state.numpes,
                                             sizeof(gasnet_seginfo_t));
  if (seginfo_table == (gasnet_seginfo_t *) NULL) {
    __shmem_warn(SHMEM_LOG_FATAL,
                 "could not allocate GASNet segments");
  }

  GASNET_SAFE( gasnet_getSegmentInfo(seginfo_table, __state.numpes) );

  /*
   * each PE initializes its own table, but can see addresses of all PEs
   */

  myspace = create_mspace_with_base(seginfo_table[__state.mype].addr,
				    seginfo_table[__state.mype].size,
				    1);

  __comms_barrier_all();
}

void
__symmetric_memory_finalize(void)
{
  destroy_mspace(myspace);
}

/*
 * where the symmetric memory starts on the given PE
 */
__inline__ void *
__symmetric_var_base(int pe)
{
  return seginfo_table[pe].addr;
}

/*
 * is the address in the managed symmetric area?
 */
__inline__ int
__symmetric_var_in_range(void *addr, int pe)
{
  void *top = seginfo_table[pe].addr + seginfo_table[pe].size;
  return (seginfo_table[pe].addr <= addr) && (addr <= top) ? 1 : 0;
}

/*
 * translate my "dest" to corresponding address on PE "pe"
 */
__inline__ void *
__symmetric_var_offset(void *dest, int pe)
{
  size_t offset = (char *)dest - (char *)__symmetric_var_base(__state.mype);
  char *rdest = (char *)__symmetric_var_base(pe) + offset;
  return (void *)rdest;
}

/*
 * -- swap handlers --
 */
gasnet_hsl_t swap_out_lock = GASNET_HSL_INITIALIZER;
gasnet_hsl_t swap_bak_lock = GASNET_HSL_INITIALIZER;

typedef struct {
  void *s_symm_addr;		/* sender symmetric var */
  void *r_symm_addr;		/* recipient symmetric car */
  long value;			/* value to be swapped */
  int sentinel;			/* end of transaction marker */
  int *sentinel_addr;		/* addr of marker for copied payload */
} swap_payload_t;

/*
 * called by remote PE to do the swap.  Store new value, send back old value
 */
void
handler_swap_out(gasnet_token_t token,
                 void *buf, size_t bufsiz,
                 gasnet_handlerarg_t unused)
{
  long old;
  swap_payload_t *pp = (swap_payload_t *) buf;

  gasnet_hsl_lock(& swap_out_lock);

  old = *(long *) pp->r_symm_addr;
  *(long *) pp->r_symm_addr = pp->value;
  pp->value = old;

  gasnet_hsl_unlock(& swap_out_lock);

  /* return the updated payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_SWAP_BAK, buf, bufsiz, unused);
}

/*
 * called by swap invoker when old value returned by remote PE
 */
void
handler_swap_bak(gasnet_token_t token,
                 void *buf, size_t bufsiz,
                 gasnet_handlerarg_t unused)
{
  swap_payload_t *pp = (swap_payload_t *) buf;

  gasnet_hsl_lock(& swap_bak_lock);

  *(long *) pp->s_symm_addr = pp->value;

  *(pp->sentinel_addr) = 1;

  gasnet_hsl_unlock(& swap_bak_lock);
}


long
__comms_request(void *target, long value, int pe)
{
  // allocate p
  swap_payload_t *p = (swap_payload_t *) malloc(sizeof(*p));
  p->s_symm_addr = target;
  p->r_symm_addr = __symmetric_var_offset(target, pe);
  p->value = value;
  p->sentinel = 0;
  p->sentinel_addr = &(p->sentinel); /* this one, not the copy */

  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_SWAP_OUT,
			  p, sizeof(*p),
			  0);

  GASNET_BLOCKUNTIL(p->sentinel);

  free(p);

  return p->value;
}
