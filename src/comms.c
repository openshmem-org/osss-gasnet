#include <assert.h>

#include "gasnet_safe.h"

#include "state.h"
#include "symmem.h"

/*
 * use the global anonymous barrier for _all.
 *
 * will presumably have to set up our own barrier flags
 * for the subset-barrier
 */

void
__comms_barrier_all(void)
{
  GASNET_BEGIN_FUNCTION();
  gasnet_barrier_notify(0,GASNET_BARRIERFLAG_ANONYMOUS);
  gasnet_barrier_wait(0,GASNET_BARRIERFLAG_ANONYMOUS);
}

void
__comms_init(void)
{
  /*
   * fake the command-line args
   */
  int argc = 1;
  char **argv;
  uintptr_t mss;
  int nhandlers;
  gasnet_handlerentry_t *handlers = (gasnet_handlerentry_t *)NULL;

  argv = (char **) malloc(argc * sizeof(*argv));
  assert(argv != (char **)NULL);
  argv[0] = "hello";

  GASNET_SAFE( gasnet_init(&argc, &argv) );

  mss = gasnet_getMaxLocalSegmentSize();

  /*
   * no active message handlers for now, but may be needed later
   * (e.g. for atomic swap)
   */
  nhandlers = 0;
  GASNET_SAFE( gasnet_attach(handlers, nhandlers, mss, 0) );

  __comms_barrier_all();

  /*
   * GASNET_WAIT_SPIN | GASNET_WAIT_BLOCK | GASNET_WAIT_SPINBLOCK
   */
   GASNET_SAFE( gasnet_set_waitmode(GASNET_WAIT_SPINBLOCK) );

  __comms_barrier_all();
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

static gasnet_seginfo_t* seginfo_table;

/* UNUSED */
#define ROUNDUP(v, n) (((v) + (n)) & ~((n) - 1))

void
__symmetric_memory_init(void)
{
  seginfo_table = (gasnet_seginfo_t *)calloc(__state.numpes,
                                             sizeof(gasnet_seginfo_t));
  assert(seginfo_table != (gasnet_seginfo_t *)NULL);

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
__inline__
void *
__symmetric_var_base(int pe)
{
  return seginfo_table[pe].addr;
}

/*
 * is the address in the managed symmetric area?
 */
__inline__
int
__symmetric_var_in_range(void *addr, int pe)
{
  void *top = seginfo_table[pe].addr + seginfo_table[pe].size;
  return (seginfo_table[pe].addr <= addr) && (addr <= top) ? 1 : 0;
}

__inline__ void
__comms_poll(void)
{
  GASNET_BEGIN_FUNCTION();
  gasnet_AMPoll();
}
