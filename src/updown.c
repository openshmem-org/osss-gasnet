#include <stdio.h>               /* NULL                           */
#include <sys/utsname.h>         /* uname()                        */
#include <assert.h>              /* assert()                       */
#include <string.h>              /* strdup()                       */
#include <sys/types.h>           /* size_t                         */

#include "gasnet_safe.h"         /* call wrapper w/ err handler    */

#include "state.h"
#include "barrier.h"
#include "symmem.h"
#include "stats.h"
#include "warn.h"
#include "atomic.h"
#include "env.h"

/* ----------------------------------------------------------------- */

void
__shmem_exit(int status)
{
  /*
   * apparently we're supposed to barrier gasnet
   * before calling exit
   */
  __gasnet_barrier_all();

  __shmem_atomic_finalize();

  __symmetric_memory_finalize();

  SHMEM_STATS_REPORT();
  SHMEM_STATS_CLEANUP();

  __state.initialized = 0;

  /*
   * strictly speaking should free alloc'ed things,
   * but exit is immediately next, so everything gets reaped anyway...
   */
  gasnet_exit(status);
}

static void
__shmem_exit_handler(void)
{
  __shmem_exit(0);
}

/*
 * find the short & (potentially) long host/node name
 */
static void
__shmem_hostnode_init(void)
{
  struct utsname u;

  assert( uname(&u) == 0 );

  __state.hostname = strdup(u.nodename);
  __state.nodename = strdup(u.nodename);
  {
    char *period = __state.nodename;
    while (*period != '\0') {
      if (*period == '.') {
        *period = '\0';
        break;
      }
      ++period;
    }
  }
}

void
shmem_init(void)
{
  __state.initialized += 1;

  /* I shouldn't really call this more than once */
  if (__state.initialized > 1) {
    __shmem_warn("NOTICE",
                 "shmem has already been initialized (occasion %d)",
                 __state.initialized - 1
                );
    return;
  }

  /*
   * TODO: abstract away from gasnet with intermediate layer
   */
  __shmem_gasnet_init();
  __state.mype = gasnet_mynode();
  __state.numpes = gasnet_nodes();
  __shmem_hostnode_init();
  __symmetric_memory_init();
  __shmem_atomic_init();
  __shmem_environment_init();
  SHMEM_STATS_INIT();
  atexit(__shmem_exit_handler);

  /*
   * and we're up and running
   */
}

/*
 * same as shmem_init()
 */
void
start_pes(int npes)
{
    shmem_init();
}

void
start_pes_(int *npes)                  /* fortran */
{
  shmem_init();
}

_Pragma("weak shmem_init_=shmem_init") /* fortran */
