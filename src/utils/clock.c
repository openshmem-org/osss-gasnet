/*
 *
 * Copyright (c) 2011 - 2015
 *   University of Houston System and UT-Battelle, LLC.
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



#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

#include "trace.h"

#include "utils.h"

/**
 * record start of program run
 */
static double epoch;

/**
 * return number of (fractional) seconds
 * since program started
 */
static
inline
double
read_clock (void)
{
  struct timeval tv;
  double t;
  int s;

  s = gettimeofday (&tv, (struct timezone *) NULL);
  if (EXPR_UNLIKELY (s != 0))
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: can't read system clock (%s)",
		     strerror (errno)
		     );
      /* NOT REACHED */
    }

  t = (double) tv.tv_sec;
  t += (double) tv.tv_usec / 1000000.0;

  return t;
}

/**
 * start the clock running
 */
void
__shmem_elapsed_clock_init (void)
{
  epoch = read_clock ();
}

/**
 * stop the clock
 */
void
__shmem_elapsed_clock_finalize (void)
{
  return;
}

/**
 * read the current run time
 */
double
__shmem_elapsed_clock_get (void)
{
  double now = read_clock ();

  return now - epoch;
}
