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

#include "trace.h"
#include "globalvar.h"
#include "comms.h"
#include "state.h"

/*
 * Deprecated
 *
 */

void
__shmem_symmetric_test_with_abort (void *remote_addr,
				   void *local_addr,
				   const char *name, const char *routine)
{
  if (remote_addr == NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "shmem_%s_%s: address %p is not symmetric",
		     name, routine, local_addr);
      /* NOT REACHED */
    }
}

/*
 * check that the address is accessible to shmem on that PE
 *
 */
int
__shmem_symmetric_addr_accessible (void *addr, int pe)
{
  return (__shmem_symmetric_addr_lookup (addr, pe) != NULL);
}

int
__shmem_is_symmetric (void *addr)
{
  return
    __shmem_symmetric_is_globalvar (addr)
    || __shmem_symmetric_var_in_range (addr, GET_STATE (mype));
}
