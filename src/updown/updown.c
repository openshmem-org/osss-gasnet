/*
 *
 * Copyright (c) 2011 - 2015
 *   University of Houston System and Oak Ridge National Laboratory.
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
#include <sys/utsname.h>	/* uname()                        */
#include <sys/types.h>		/* size_t                         */

#include "globalvar.h"
#include "state.h"
#include "trace.h"
#include "atomic.h"

#include "barrier.h"
#include "barrier-all.h"
#include "broadcast.h"
#include "collect.h"
#include "fcollect.h"

#include "ping.h"
#include "utils.h"
#include "clock.h"
#include "exe.h"

#include "shmem.h"

#include "comms/comms.h"

#ifdef HAVE_FEATURE_PSHMEM
# include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */

#ifdef HAVE_FEATURE_EXPERIMENTAL
# include "pshmemx.h"
#endif /* HAVE_FEATURE_EXPERIMENTAL */

#include "version.h"

/* ----------------------------------------------------------------- */

/**
 * I shouldn't really initialize more than once
 */
static
inline
int
check_pe_status (void)
{
  int yn = 1;
  const pe_status_t s = GET_STATE (pe_status);

  switch (s)
    {
    case PE_UNINITIALIZED:	/* this is what it should be */
      yn = 1;
      break;
    case PE_UNKNOWN:
    case PE_RUNNING:
    case PE_SHUTDOWN:
    case PE_FAILED:
      shmemi_trace (SHMEM_LOG_INFO,
		     "OpenSHMEM has already been initialized (%s)",
		     shmemi_state_as_string (s)
		     );
      yn = 0;
      break;
    default:			/* shouldn't be here */
      yn = 0;
      break;
    }

  return yn;
}

static
inline
void
report_up (void)
{
  int maj, min;
  const int n = GET_STATE (numpes);
  const size_t h = GET_STATE (heapsize);

  if (shmemi_version (&maj, &min) == 0)
    {
      shmemi_trace (SHMEM_LOG_INIT,
		     "version %d.%d running on %d PE%s, using %zd bytes of symmetric heap",
		     maj, min,
		     n, (n == 1) ? "" : "s",
		     h
		     );
    }
}

#ifdef HAVE_FEATURE_PSHMEM
# pragma weak start_pes = pstart_pes
# define start_pes pstart_pes
#endif /* HAVE_FEATURE_PSHMEM */

static
inline
void
shmem_init_private (int npes)
{
  if ( ! check_pe_status ())
    {
      return;
    }

  shmemi_comms_init ();

  /* just note start_pes() not passed 0, it's not a big deal */
  if (npes != 0)
    {
      shmemi_trace (SHMEM_LOG_INFO,
		     "start_pes() was passed %d, should be 0",
                     npes
		     );
    }

  report_up ();

  /*
   * and we're up and running
   */
}

/**
 * \brief This routine initializes the OpenSHMEM environment on the calling PE.
 *
 * \b Synopsis:
 *
 * - C/C++:
 * \code
 *   void shmem_init (void);
 *   void start_pes (int npes);
 * \endcode
 *
 * - Fortran:
 * \code
 *   INTEGER npes
 *
 *   CALL START_PES (npes)
 * \endcode
 *
 * \param npes the number of PEs participating in the program.  This
 * is ignored and should be set to 0.
 *
 * \b Effect:
 *
 * Initializes the OpenSHMEM environment on the calling PE.
 *
 * \return None.
 *
 */

#ifdef HAVE_FEATURE_PSHMEM
# pragma weak shmem_init = pshmem_init
# define shmem_init pshmem_init
# pragma weak shmem_finalize = pshmem_finalize
# define shmem_finalize pshmem_finalize
#endif /* HAVE_FEATURE_PSHMEM */

void
shmem_init (void)
{
  shmem_init_private (0);
}

void
start_pes (int npes)
{
  shmem_init_private (npes);
}

/*
 * TODO: make sure atexit() knows if finalize already called explicitly
 */
void
shmem_finalize (void)
{
  shmemi_comms_finalize ();
}

void
shmem_global_exit (int status)
{
  shmemi_comms_globalexit_request (status);
}
