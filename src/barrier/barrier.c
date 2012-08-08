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



#include <stdio.h>
#include <strings.h>

#include "comms.h"
#include "trace.h"
#include "utils.h"
#include "modules.h"

#include "pshmem.h"

/*
 * handlers for broadcast implementations
 *
 */

static module_info_t mi_all, mi_bar;

/*
 * called during initialization of shmem
 *
 */

void
__shmem_barriers_dispatch_init (void)
{
  char *bar_name;
  char *all_name;
  int sa, sb;

  /*
   * choose the barrier_all
   *
   */

  all_name = __shmem_comms_getenv ("SHMEM_BARRIER_ALL_ALGORITHM");
  if (all_name == (char *) NULL)
    {
      all_name = __shmem_modules_get_implementation ("barrier-all");
    }
  sa = __shmem_modules_load ("barrier-all", all_name, &mi_all);
  if (sa != 0)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: couldn't load barrier-all module \"%s\"",
		     all_name);
      /* NOT REACHED */
    }

  /*
   * choose the barrier (could be different from _all)
   *
   */

  bar_name = __shmem_comms_getenv ("SHMEM_BARRIER_ALGORITHM");
  if (bar_name == (char *) NULL)
    {
      bar_name = __shmem_modules_get_implementation ("barrier");
    }
  sb = __shmem_modules_load ("barrier", bar_name, &mi_bar);
  if (sb != 0)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: couldn't load barrier module \"%s\"",
		     bar_name);
      /* NOT REACHED */
    }

  /*
   * report which implementation we set up
   */
  __shmem_trace (SHMEM_LOG_BROADCAST,
		 "using barrier_all \"%s\" & barrier \"%s\"",
		 all_name, bar_name);

}

/*
 * the rest is what library users see
 *
 * in this case we don't have the 32/64 bit divide, so we just look at
 * the 32-bit version and use that pointer
 *
 */


#pragma weak shmem_barrier_all = pshmem_barrier_all
#define shmem_barrier_all pshmem_barrier_all

/* @api@ */
void
shmem_barrier_all (void)
{
  INIT_CHECK ();

  shmem_quiet ();

  mi_all.func_32 ();
}



#pragma weak shmem_barrier = pshmem_barrier
#define shmem_barrier pshmem_barrier

/* @api@ */
void
shmem_barrier (int PE_start, int logPE_stride, int PE_size, long *pSync)
{
  INIT_CHECK ();

  /* barrier doesn't imply shmem_quiet(); */

  mi_bar.func_32 (PE_start, logPE_stride, PE_size, pSync);
}
