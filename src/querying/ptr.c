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



/*
 * TODO: shmem_ptr only makes sense on platforms where puts and gets
 * occur exclusively in shared-memory.  On a multi-node cluster, it
 * can't do anything, so return NULL, which is correct behavior.
 * (which is really good, because I couldn't see how it could possibly
 * work :-)
 */

#include <stdio.h>

#include "trace.h"
#include "utils.h"

#ifdef HAVE_FEATURE_PSHMEM
# include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */


#ifdef HAVE_FEATURE_PSHMEM
# pragma weak shmem_ptr = pshmem_ptr
# define shmem_ptr pshmem_ptr
#endif /* HAVE_FEATURE_PSHMEM */

/**
 *
 *
 * \brief Return a pointer through which the calling PE can access
 * another PE's memory directly (as a local r/w).
 *
 * \b Synopsis:
 *
 * - C/C++:
 * \code
 *   void *shmem_ptr (void *target, int pe);
 * \endcode
 *
 * - Fortran:
 * \code
 *   POINTER (PTR, POINTEE)
 *   INTEGER pe
 *
 *   PTR = SHMEM_PTR(target, pe)
 * \endcode
 *
 * \param target    The symmetric data object to be referenced.
 * \param pe        An integer that indicates the PE number upon
 *                which target is to be accessed. If you are using Fortran, it must
 *                be a default integer value.
 *
 * \b Constraints:
 *      - target must be the address of a symmetric data object.
 *
 * \b Effect:
 *
 * The shmem_ptr routine returns an address that can be used to
 * directly reference target on the remote PE pe. With this address we
 * can perform ordinary loads and stores to the remote address.  When a
 * sequence of loads (gets) and stores (puts) to a data object on a
 * remote PE does not match the access pattern provided in a SHMEM data
 * transfer routine like shmem_put32(3) or shmem_real_iget(3), the
 * shmem_ptr function can provide an efficient means to accomplish the
 * communication.
 *
 * \return Address of remote memory, or NULL if the memory can not be
 * accessed.
 *
 */

void *
shmem_ptr (void *target, int pe)
{
  INIT_CHECK ();
  PE_RANGE_CHECK (pe, 2);

#ifdef SHMEM_PUTGET_SHARED_MEMORY

  __shmem_trace (SHMEM_LOG_NOTICE, "shmem_ptr() not implemented yet");
  return (void *) NULL;

#else /* ! SHMEM_PUTGET_SHARED_MEMORY */

  return (void *) NULL;

#endif /* SHMEM_PUTGET_SHARED_MEMORY */
}
