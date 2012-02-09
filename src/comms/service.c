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



/*
 * Do network service.  When code is not engaged in shmem calls,
 * something needs to provide communication access so that operations
 * where "this" PE is a passive target can continue
 */


#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

#include "comms.h"

static struct itimerval t;

static void
init_timer (void)
{
  t.it_value.tv_sec = 0L;
  t.it_value.tv_usec = 1000L; /* 1 ms */

  t.it_interval.tv_sec = 0L;
  t.it_interval.tv_usec = 1000L; /* 1 ms */
}

static void
alarm_handler (int signum)
{
  __shmem_comms_pause ();
}

/*
 * start the servicer
 */

void
__shmem_service_init (void)
{
  init_timer ();

  signal (SIGVTALRM, alarm_handler);

  setitimer (ITIMER_VIRTUAL,
	     &t,
	     NULL
	     );
}

/*
 * called when an I/O op. has occurred.  The servicer can go back to
 * sleep for a while.
 */

void
__shmem_service_reset (void)
{
  __shmem_service_init ();
}
