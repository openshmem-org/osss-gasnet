#include <stdio.h>               /* NULL                           */
#include <sys/utsname.h>         /* uname()                        */
#include <string.h>              /* strdup()                       */
#include <sys/types.h>           /* size_t                         */

#include "comms.h"
#include "state.h"
#include "stats.h"
#include "warn.h"
#include "atomic.h"
#include "env.h"

/* ----------------------------------------------------------------- */

void
__shmem_exit(int status)
{

  __shmem_atomic_finalize();
  __symmetric_memory_finalize();

  SHMEM_STATS_REPORT();
  SHMEM_STATS_CLEANUP();

  __state.initialized = 0;

  /*
   * strictly speaking should free alloc'ed things,
   * but exit is immediately next, so everything gets reaped anyway...
   */
  __comms_shutdown(status);
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
  int s;

  s = uname(&u);
  if (s != 0) {
    __shmem_warn(SHMEM_LOG_FATAL,
                 "can't find any node information"
                );
  }

  __state.hostname = strdup(u.nodename);
  /* TODO: slightly wasteful of space, but domain names aren't too big */
  __state.nodename = strdup(u.nodename);
  {
    char *period = __state.nodename;
    while (*period != '\0') {
      if (*period == '.') {
        *period = '\0';
        break;
      }
      period += 1;
    }
  }
}

void
shmem_init(void)
{

  /* has to happen early to enable warn */
  __shmem_environment_init();

  __state.initialized += 1;

  /* I shouldn't really call this more than once */
  if (__state.initialized > 1) {
    __shmem_warn(SHMEM_LOG_FATAL,
                 "shmem has already been initialized"
                );
  }

  __comms_init();
  __state.mype = __comms_mynode();
  __state.numpes = __comms_nodes();

  __shmem_hostnode_init();

  __symmetric_memory_init();

  __shmem_atomic_init();

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
