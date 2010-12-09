#include <stdio.h>               /* NULL                           */
#include <stdlib.h>              /* atexit()                       */
#include <sys/utsname.h>         /* uname()                        */
#include <string.h>              /* strdup()                       */
#include <sys/types.h>           /* size_t                         */

#include "comms.h"
#include "state.h"
#include "warn.h"
#include "atomic.h"
#include "env.h"

#include "shmem.h"

/* ----------------------------------------------------------------- */

void
__shmem_exit(int status)
{
  __shmem_atomic_finalize();
  __symmetric_memory_finalize();

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
__shmem_place_init(void)
{
  int s;

  s = uname(& __state.loc);
  if (s != 0) {
    __shmem_warn(SHMEM_LOG_FATAL,
                 "can't find any node information"
                );
  }
}

/* @api@ */
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

  __shmem_place_init();

  __symmetric_memory_init();

  __shmem_atomic_init();

  atexit(__shmem_exit_handler);

  /*
   * and we're up and running
   */

  __shmem_warn(SHMEM_LOG_INIT,
	       "version \"%s\" running on %d PE%s",
	       shmem_version(),
	       __state.numpes,
	       __state.numpes == 1 ? "" : "s"
	       );
}

/*
 * same as shmem_init()
 */
/* @api@ */
void
start_pes(int npes)
{
  shmem_init();
}

#ifdef HAVE_PSHMEM_SUPPORT
_Pragma("weak pshmem_init=shmem_init")
_Pragma("weak pstart_pes=start_pes")
#endif /* HAVE_PSHMEM_SUPPORT */
