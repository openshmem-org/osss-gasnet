/*
 *
 * Copyright (c) 2011, 2012
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



/*
 * Do network service.  When code is not engaged in shmem calls,
 * something needs to provide communication access so that operations
 * where "this" PE is a passive target can continue
 */


#include <stdio.h>
#include <pthread.h>
#include <errno.h>

#include "comms.h"
#include "trace.h"

static pthread_t thr;

static volatile int done = 0;

/*
 * does comms. service until told not to
 */

static void *
start_service (void *unused)
{
  while (! done)
    {
      __shmem_comms_service ();
      pthread_yield ();
    }
}

/*
 * start the servicer
 */

void
__shmem_service_init (void)
{
  int s;

  s = pthread_create (&thr, NULL, start_service, (void *) 0);
  if (s != 0)
    {
       __shmem_trace (SHMEM_LOG_FATAL,
                      "internal error: service thread creation failed"
                     );
       /* NOT REACHED */
    }
}

/*
 * stop the servicer
 */

void
__shmem_service_finalize (void)
{
  int s;

  done = 1;

  s = pthread_join (thr, NULL);
  if (s != 0)
    {
       __shmem_trace (SHMEM_LOG_FATAL,
                      "internal error: service thread termination failed"
                     );
       /* NOT REACHED */
    }
}
