#include <assert.h>              /* assert()                       */
#include <stdlib.h>              /* strdup()                       */
#include <sys/types.h>           /* size_t                         */

#include "gasnet_safe.h"         /* call wrapper w/ err handler    */

#include "state.h"
#include "stats.h"

/* ----------------------------------------------------------------- */

void
__shmem_gasnet_init(void)
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

  __gasnet_barrier_all();

  /*
   * GASNET_WAIT_SPIN | GASNET_WAIT_BLOCK | GASNET_WAIT_SPINBLOCK
   */
   GASNET_SAFE( gasnet_set_waitmode(GASNET_WAIT_SPINBLOCK) );

  __gasnet_barrier_all();
}
