/*
 *
 * Copyright (c) 2011 - 2013
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

#include "shmem.h"

#include "modules.h"

/*
 * handlers for broadcast implementations
 *
 */

static module_info_t mi;


/*
 * called during initialization of shmem
 *
 */

void
__shmem_broadcast_dispatch_init (void)
{
  char *name;
  int s;

  /*
   * choose the broadcast: env >> config file
   *
   */
  name = __shmem_comms_getenv ("SHMEM_BROADCAST_ALGORITHM");
  if (name == (char *) NULL)
    {
      name = __shmem_modules_get_implementation ("broadcast");
    }
  s = __shmem_modules_load ("broadcast", name, &mi);
  if (s != 0)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: couldn't load broadcast module \"%s\"",
		     name);
      /* NOT REACHED */
    }

  /*
   * report which broadcast implementation we set up
   */
  __shmem_trace (SHMEM_LOG_BROADCAST, "using broadcast \"%s\"", name);
}

/*
 * the rest is what library users see
 *
 */

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_broadcast32 = pshmem_broadcast32
#define shmem_broadcast32 pshmem_broadcast32
#endif /* HAVE_FEATURE_PSHMEM */

/* @api@ */
void
shmem_broadcast32 (void *target, const void *source, size_t nelems,
		   int PE_root, int PE_start, int logPE_stride, int PE_size,
		   long *pSync)
{
  SYMMETRY_CHECK (target, 1, "shmem_broadcast32");
  SYMMETRY_CHECK (source, 2, "shmem_broadcast32");

  mi.func_32 (target, source, nelems,
	      PE_root, PE_start, logPE_stride, PE_size, pSync);
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_broadcast64 = pshmem_broadcast64
#define shmem_broadcast64 pshmem_broadcast64
#endif /* HAVE_FEATURE_PSHMEM */

/* @api@ */
void
shmem_broadcast64 (void *target, const void *source, size_t nelems,
		   int PE_root, int PE_start, int logPE_stride, int PE_size,
		   long *pSync)
{
  SYMMETRY_CHECK (target, 1, "shmem_broadcast64");
  SYMMETRY_CHECK (source, 2, "shmem_broadcast64");

  mi.func_64 (target, source, nelems,
	      PE_root, PE_start, logPE_stride, PE_size, pSync);
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_sync_init = pshmem_sync_init
#define shmem_sync_init pshmem_sync_init
#endif /* HAVE_FEATURE_PSHMEM */

/* @api@ */
void
shmem_sync_init (long *pSync)
{
  const int nb = _SHMEM_BCAST_SYNC_SIZE;
  int i;

  SYMMETRY_CHECK (pSync, 1, "shmem_sync_init");

  for (i = 0; i < nb; i += 1)
    {
      pSync[i] = _SHMEM_SYNC_VALUE;
    }
  shmem_barrier_all ();
}
