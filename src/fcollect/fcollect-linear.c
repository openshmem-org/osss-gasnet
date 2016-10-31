/*
 *
 * Copyright (c) 2016
 *   Stony Brook University
 * Copyright (c) 2015 - 2016
 *   Los Alamos National Security, LLC.
 * Copyright (c) 2011 - 2016
 *   University of Houston System and UT-Battelle, LLC.
 * Copyright (c) 2009 - 2016
 *   Silicon Graphics International Corp.  SHMEM is copyrighted
 *   by Silicon Graphics International Corp. (SGI) The OpenSHMEM API
 *   (shmem) is released by Open Source Software Solutions, Inc., under an
 *   agreement with Silicon Graphics International Corp. (SGI).
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * o Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimers.
 *
 * o Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * o Neither the name of the University of Houston System,
 *   UT-Battelle, LLC. nor the names of its contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * o Neither the name of Los Alamos National Security, LLC, Los Alamos
 *   National Laboratory, LANL, the U.S. Government, nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
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


#include <sys/types.h>

#include "state.h"
#include "putget.h"
#include "trace.h"
#include "utils.h"
#include "symmtest.h"

#include "comms/comms.h"

#include "shmem.h"

/*
 * fcollect puts nelems (*same* value on all PEs) from source on each
 * PE in the set to target on all PEs in the set.  source -> target is
 * done in PE order.
 *
 */

#define SHMEM_FCOLLECT(Bits, Bytes)                                     \
    void                                                                \
    shmemi_fcollect##Bits##_linear(void *target, const void *source,    \
                                   size_t nelems,                       \
                                   int PE_start, int logPE_stride,      \
                                   int PE_size,                         \
                                   long *pSync)                         \
    {                                                                   \
        const int step = 1 << logPE_stride;                             \
        const int vpe = (GET_STATE(mype) - PE_start) >> logPE_stride;   \
        const size_t tidx = nelems * Bytes * vpe;                       \
        int i;                                                          \
        int pe = PE_start;                                              \
        for (i = 0; i < PE_size; i += 1) {                              \
            shmem_put##Bits(target + tidx, source, nelems, pe);         \
            pe += step;                                                 \
        }                                                               \
        shmem_barrier(PE_start, logPE_stride, PE_size, pSync);          \
    }

SHMEM_FCOLLECT (32, 4);
SHMEM_FCOLLECT (64, 8);
