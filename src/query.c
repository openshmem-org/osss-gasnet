/*
 *
 * Copyright (c) 2011, University of Houston System and Oak Ridge National
 * Loboratory.
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
 *   National Loboratory nor the names of its contributors may be used to
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



#include "state.h"
#include "trace.h"
#include "utils.h"

#include "mpp/shmem.h"

/*
 * these routines handle the questions "how many PEs?" and "which PE
 * am I?".  Also added an initial thought about locality with the
 * nodename query
 *
 */

#define SHMEM_MY_PE(Variant)			\
  int						\
  p##Variant (void)				\
  {						\
    INIT_CHECK();				\
    return GET_STATE(mype);			\
  }

/* SHMEM_MY_PE(my_pe) */
SHMEM_MY_PE(_my_pe)

#define SHMEM_NUM_PES(Variant)			\
  int						\
  p##Variant (void)				\
  {						\
    INIT_CHECK();				\
    return GET_STATE(numpes);			\
  }

/* SHMEM_NUM_PES(num_pes) */
SHMEM_NUM_PES(_num_pes)

char *
pshmem_nodename(void)
{
  INIT_CHECK();
  return GET_STATE(loc.nodename);
}


/* #pragma weak my_pe = pmy_pe */
#pragma weak _my_pe = p_my_pe

/* #pragma weak num_pes = pnum_pes */
#pragma weak _num_pes = p_num_pes

#pragma weak shmem_nodename = pshmem_nodename

#ifdef CRAY_COMPAT
SHMEM_NUM_PES(shmem_num_pes)
SHMEM_NUM_PES(shmem_n_pes)
SHMEM_MY_PE(shmem_my_pe)

#pragma weak shmem_my_pe = pshmem_my_pe
#pragma weak shmem_num_pes = pshmem_num_pes
#pragma weak shmem_n_pes = pshmem_n_pes
#endif /* CRAY_COMPAT */
