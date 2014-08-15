/*
 *
 * Copyright (c) 2011 - 2014
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



/**
 * Do network service.  When code is not engaged in shmem calls,
 * something needs to provide communication access so that operations
 * where "this" PE is a passive target can continue
 */


#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

/**
 * for hi-res timer
 */

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 199309
#endif /* _POSIX_C_SOURCE */
#include <time.h>

#include "gasnet_safe.h"

#include "../comms.h"

#include "bail.h"

#include "utils.h"

/**
 * for refractory back-off
 */

static long delay = 1000L; /* ns */
static struct timespec delayspec;

/**
 * new thread for progress-o-matic
 */

static pthread_t thr;

/**
 * polling sentinel
 */

static volatile int done = 0;


/**
 * traffic progress
 *
 */
void
__shmem_comms_service (void)
{
  GASNET_SAFE (gasnet_AMPoll ());
}

/**
 * does comms. service until told not to
 */

static
void *
start_service (void *unused)
{
  do
    {
      __shmem_comms_service ();
      pthread_yield ();
      nanosleep (&delayspec, NULL); /* back off */
    }
  while (! done);

  return NULL;
}

/**
 * assume initially we need to manage progress ourselves
 */
static int use_conduit_thread = 0;

/**
 * tell a PE how to contend for updates
 *
 */
static
inline
void
waitmode_init (void)
{
  /*
   * this gives best performance in all cases observed by the author
   * (@ UH).  Could make this programmable.
   */
  GASNET_SAFE (gasnet_set_waitmode (GASNET_WAIT_SPINBLOCK));
}

/**
 * start the servicer
 */

void
__shmem_service_init (void)
{
  /*
   * Zap this code for now.  Problems with IBV conduit thread if all
   * PEs on one physical node.
   *
   */
#if 0
#if defined(GASNETC_IBV_RCV_THREAD) && \
    (defined(GASNET_CONDUIT_IBV) || defined(GASNET_CONDUIT_VAPI))
  /*
   * if we have an IBV progress thread configured, then check env for
   * GASNET_RCV_THREAD.
   *
   * With no env var, let ibv conduit handle things...
   *
   * If set to [0nN] (false), we start our own progress thread
   * If set to [1yY] (true), the conduit handles progress
   *
   * Any other value, assume true but make a note.  NB with 1.20.2,
   * GASNet itself traps other values and aborts.
   *
   */

  const char *grt_str = "GASNET_RCV_THREAD";
  char *rtv = __shmem_comms_getenv (grt_str);
  if (EXPR_LIKELY (rtv == NULL))
    {
      use_conduit_thread = 1;
    }
  else
    {
      switch (*rtv)
	{
	case '0':
	case 'n':
	case 'N':
	  use_conduit_thread = 0;
	  break;
	case '1':
	case 'y':
	case 'Y':
	  use_conduit_thread = 1;
	  break;
	default:
	  use_conduit_thread = 1;
	  break;
	}
    }
#endif /* defined(GASNETC_IBV_RCV_THREAD) &&
          (defined(GASNET_CONDUIT_IBV) || defined(GASNET_CONDUIT_VAPI)) */
#endif /* commented out */

  if (! use_conduit_thread)
    {
      int s;

      delayspec.tv_sec = (time_t) 0;
      delayspec.tv_nsec = delay;

      s = pthread_create (&thr, NULL, start_service, (void *) 0);
      if (EXPR_UNLIKELY (s != 0))
        {
	  comms_bailout ("internal error: progress thread creation failed (%s)",
			 strerror (s)
			 );
	  /* NOT REACHED */
        }
    }

  waitmode_init ();
}

/**
 * stop the servicer
 */

void
__shmem_service_finalize (void)
{
  if (! use_conduit_thread)
    {
      int s;

      done = 1;

      s = pthread_join (thr, NULL);
      if (EXPR_UNLIKELY (s != 0))
	{
	  comms_bailout ("internal error: progress thread termination failed (%s)",
			 strerror (s)
			 );
	  /* NOT REACHED */
	}
    }
}
