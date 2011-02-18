#include <stdio.h>               /* NULL                           */
#include <stdlib.h>              /* atexit()                       */
#include <sys/utsname.h>         /* uname()                        */
#include <string.h>              /* strdup()                       */
#include <sys/types.h>           /* size_t                         */

#include "comms.h"
#include "state.h"
#include "trace.h"
#include "atomic.h"
#include "barrier.h"
#include "ping.h"
#include "utils.h"

#include "shmem.h"

/* ----------------------------------------------------------------- */

/*
 * shut down shmem, and then hand off to the comms layer to shut
 * itself down
 *
 */

void
__shmem_exit(int status)
{
  __shmem_atomic_finalize();
  __symmetric_memory_finalize();

  __state.pe_status = PE_SHUTDOWN;

  /*
   * strictly speaking should free alloc'ed things,
   * but exit is immediately next, so everything gets reaped anyway...
   */
  __comms_shutdown(status);
}

/*
 * registered by start_pes() to trigger shut down at exit
 *
 */

static void
__shmem_exit_handler(void)
{
  __shmem_exit(0);
}

/*
 * find the short & (potentially) long host/node name
 *
 */
static void
__shmem_place_init(void)
{
  int s;

  s = uname(& __state.loc);
  if (s != 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "can't find any node information"
		  );
  }
}

/*
 * this is where we get everything up and running
 *
 */

/* @api@ */
void
pstart_pes(int npes)
{
  /* has to happen early to enable messages */
  __shmem_tracers_init();

  /* I shouldn't really call this more than once */
  if (__state.pe_status != PE_UNINITIALIZED) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "shmem has already been initialized (%s)",
		  __shmem_state_as_string(__state.pe_status)
		  );
    /* NOT REACHED */
  }

  /* set up communications layer */
  __comms_init();

  /* see if we want to say which message/trace levels are active */
  __shmem_tracers_show();

  /* set up any locality information */
  __shmem_place_init();

  /* set up PE memory management */
  __symmetric_memory_init();

  /* set up the atomic ops handling */
  __shmem_atomic_init();

  /* set up timeouts */
  __shmem_ping_init();

  /* set up barrier selection */
  __barrier_dispatch_init();

  if (atexit(__shmem_exit_handler) != 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: cannot register shutdown handler"
		  );
    /* NOT REACHED */
  }

  /*
   * SGI man page says npes *should* be zero, so just make
   * this check informational
   */
  if (npes != 0) {
    __shmem_trace(SHMEM_LOG_INFO,
		  "start_pes() was passed %d, should be 0",
		  npes
		  );
  }

  /*
   * and we're up and running
   */

  __state.pe_status = PE_RUNNING;

  __shmem_trace(SHMEM_LOG_INIT,
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
pshmem_init(void)
{
  pstart_pes(0);
}

/*
 * does nothing here (just for compatibility with other shmems)
 */
/* @api@ */
void
pshmem_finalize(void)
{
  INIT_CHECK();
}

#pragma weak start_pes = pstart_pes
#pragma weak shmem_init = pshmem_init
#pragma weak shmem_finalize = pshmem_finalize
