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



#include <stdio.h>
#include <strings.h>

#include "comms.h"
#include "trace.h"
#include "utils.h"

#include "mpp/shmem.h"

#include "modules.h"

/*
 * handlers for implementations
 *
 */

static module_info_t mi;


/*
 * called during initialization of shmem
 *
 */

void
__shmem_collect_dispatch_init (void)
{
  char *name;
  int s;

  /*
   * choose the collect
   *
   */

  name = __shmem_comms_getenv ("SHMEM_COLLECT_ALGORITHM");
  if (name == (char *) NULL)
    {
      name = __shmem_modules_get_implementation ("collect");
    }
  s = __shmem_modules_load ("collect", name, &mi);
  if (s != 0)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: couldn't load collect module \"%s\"",
		     name);
      /* NOT REACHED */
    }

  /*
   * report which implementation we set up
   */
  __shmem_trace (SHMEM_LOG_BROADCAST, "using collect \"%s\"", name);
}

/*
 * the rest is what library users see
 *
 */


/* @api@ */
void
pshmem_collect32 (void *target, const void *source, size_t nelems,
		  int PE_start, int logPE_stride, int PE_size, long *pSync)
{
  SYMMETRY_CHECK (target, 1, "shmem_collect32");
  SYMMETRY_CHECK (source, 2, "shmem_collect32");

  mi.func_32 (target, source, nelems, PE_start, logPE_stride, PE_size, pSync);
}

/* @api@ */
void
pshmem_collect64 (void *target, const void *source, size_t nelems,
		  int PE_start, int logPE_stride, int PE_size, long *pSync)
{
  SYMMETRY_CHECK (target, 1, "shmem_collect64");
  SYMMETRY_CHECK (source, 2, "shmem_collect64");

  mi.func_64 (target, source, nelems, PE_start, logPE_stride, PE_size, pSync);
}

#pragma weak shmem_collect32 = pshmem_collect32
#pragma weak shmem_collect64 = pshmem_collect64
