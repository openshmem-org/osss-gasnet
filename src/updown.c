/*
 *
 * Copyright (c) 2011, University of Houston System and Oak Ridge National
 * Laboratory.
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
 * o Neither the name of the University of Houston System, Oak Ridge
 *   National Laboratory nor the names of its contributors may be used to
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



#include <stdio.h>		/* NULL                           */
#include <stdlib.h>		/* atexit()                       */
#include <sys/utsname.h>	/* uname()                        */
#include <sys/types.h>		/* size_t                         */

#include "comms.h"
#include "globalvar.h"
#include "state.h"
#include "trace.h"
#include "atomic.h"

#include "barrier.h"
#include "broadcast.h"
#include "collect.h"
#include "fcollect.h"

#include "ping.h"
#include "utils.h"
#include "clock.h"
#include "exe.h"

#include "modules.h"

#include "mpp/shmem.h"

/* ----------------------------------------------------------------- */

/*
 * shut down shmem, and then hand off to the comms layer to shut
 * itself down
 *
 */
void
__shmem_exit (int status)
{
  __shmem_comms_barrier_all ();
  /* ok, no more pending I/O ... */

  /* clean up atomics and memory */
  __shmem_atomic_finalize ();
  __shmem_symmetric_memory_finalize ();

  /* clean up plugin modules */
  __shmem_modules_finalize ();

  /* tidy up binary inspector */
  __shmem_executable_finalize ();

  /* stop run time clock */
  __shmem_elapsed_clock_finalize ();

  /* update our state */
  SET_STATE (pe_status, PE_SHUTDOWN);

  __shmem_trace (SHMEM_LOG_INIT,
		 "finalizing shutdown, handing off to communications layer");

  /*
   * strictly speaking should free remaining alloc'ed things,
   * but exit is immediately next, so everything gets reaped anyway...
   */
  __shmem_comms_finalize (status);
}

/*
 * registered by start_pes() to trigger shutdown at exit
 *
 */
static void
__shmem_exit_handler (void)
{
  __shmem_exit (0);
}

/*
 * find the short & (potentially) long host/node name
 *
 */
static void
__shmem_place_init (void)
{
  int s;

  s = uname (&GET_STATE (loc));
  if (s != 0)
    {
      __shmem_trace (SHMEM_LOG_FATAL, "can't find any node information");
    }
}

/*
 * this is where we get everything up and running
 *
 */

/* @api@ */
void
pstart_pes (int npes)
{
  /* these have to happen first to enable messages */
  __shmem_elapsed_clock_init ();	/* start the tracking clock */
  __shmem_tracers_init ();	/* messages set up */

  /* I shouldn't really call this more than once */
  if (GET_STATE (pe_status) != PE_UNINITIALIZED)
    {
      __shmem_trace (SHMEM_LOG_INFO,
		     "shmem has already been initialized (%s)",
		     __shmem_state_as_string (GET_STATE (pe_status)));
      return;
      /* NOT REACHED */
    }

  /* find out what this executable image is */
  __shmem_executable_init ();

  /* set up communications layer */
  __shmem_comms_init ();
  /* __shmem_comms_barrier_all(); */

  /*
   * find the global symbols (i.e. those addressable outside the
   * symmetric heap)
   */
  __shmem_symmetric_globalvar_table_init ();

  /* handle the heap */
  __shmem_symmetric_memory_init ();

  /* see if we want to say which message/trace levels are active */
  __shmem_maybe_tracers_show_info ();
  __shmem_tracers_show ();

  /* set up the atomic ops handling */
  __shmem_atomic_init ();

  /* set up timeouts */
  __shmem_ping_init ();

  /* set up module selection */
  __shmem_modules_init ();

  __shmem_barriers_dispatch_init ();
  __shmem_broadcast_dispatch_init ();
  __shmem_collect_dispatch_init ();
  __shmem_fcollect_dispatch_init ();

  /* set up any locality information */
  __shmem_place_init ();

  /* register shutdown handler */
  if (atexit (__shmem_exit_handler) != 0)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: cannot register shutdown handler");
      /* NOT REACHED */
    }

  /* just note start_pes() not passed 0, it's not a big deal */
  if (npes != 0)
    {
      __shmem_trace (SHMEM_LOG_INFO,
		     "start_pes() was passed %d, should be 0", npes);
    }

  /*
   * and we're up and running
   */

  SET_STATE (pe_status, PE_RUNNING);

  {
    int maj, min;
    if (shmem_version (&maj, &min) == 0)
      {
	__shmem_trace (SHMEM_LOG_VERSION,
		       "version %d.%d running on %d PE%s",
		       maj, min,
		       GET_STATE (numpes),
		       GET_STATE (numpes) == 1 ? "" : "s");
      }
  }

  __shmem_comms_barrier_all ();
}

#pragma weak start_pes = pstart_pes



#ifdef CRAY_COMPAT

/*
 * same as start_pes()
 */

/* @api@ */
void
pshmem_init (void)
{
  pstart_pes (0);
}

/*
 * does nothing here: just for compatibility
 */

/* @api@ */
void
pshmem_finalize (void)
{
  INIT_CHECK ();
}

#pragma weak shmem_init = pshmem_init
#pragma weak shmem_finalize = pshmem_finalize

#endif /* CRAY_COMPAT */
