/*
 *
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
 *   this list of conditions and the following disclaimer.
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

#include "comms/comms.h"

#include "shmem.h"

/**
 * collect puts nelems (can vary from PE to PE) from source on each
 * PE in the set to target on all PEs in the set.  source -> target is
 * done in PE order.
 *
 * We set up a wavefront that propagates the accumulated offsets and
 * then overlaps forwarding the extra offset contribution from each PE
 * with actually sending source -> target.  Control data goes left to
 * right, application data goes "top to bottom", as it were.
 *
 */

#define SHMEM_COLLECT(Bits, Bytes)                                      \
    void                                                                \
    shmemi_collect##Bits##_linear(void *target, const void *source, size_t nelems, \
                                  int PE_start, int logPE_stride, int PE_size, \
                                  long *pSync)                          \
    {                                                                   \
        const int step = 1 << logPE_stride;                             \
        const int last_pe = PE_start + step * (PE_size - 1);            \
        const int me = GET_STATE(mype);                                 \
        /* TODO: temp fix: I know barrier doesn't use this many indices */ \
        long *acc_off = & (pSync[SHMEM_COLLECT_SYNC_SIZE - 1]);         \
                                                                        \
        INIT_CHECK();                                                   \
        SYMMETRY_CHECK(target, 1, "shmem_collect");                     \
        SYMMETRY_CHECK(source, 2, "shmem_collect");                     \
                                                                        \
        shmemi_trace(SHMEM_LOG_COLLECT,                                 \
                     "nelems = %ld, PE_start = %d, PE_stride = %d, PE_size = %d, last_pe = %d", \
                     nelems,                                            \
                     PE_start,                                          \
                     step,                                              \
                     PE_size,                                           \
                     last_pe                                            \
                     );                                                 \
                                                                        \
        /* initialize left-most or wait for left-neighbor to notify */  \
        if (me == PE_start) {                                           \
            *acc_off = 0;                                               \
        }                                                               \
        else {                                                          \
            shmem_long_wait(acc_off, SHMEM_SYNC_VALUE);                 \
            shmemi_trace(SHMEM_LOG_COLLECT,                             \
                         "got acc_off = %ld",                           \
                         *acc_off                                       \
                         );                                             \
        }                                                               \
                                                                        \
        /*                                                              \
         * forward my contribution to (notify) right neighbor if not last PE \
         * in set                                                       \
         */                                                             \
        if (me < last_pe) {                                             \
            const long next_off = *acc_off + nelems;                    \
            const int rnei = me + step;                                 \
                                                                        \
            shmem_long_p(acc_off, next_off, rnei);                      \
                                                                        \
            shmemi_trace(SHMEM_LOG_COLLECT,                             \
                         "put next_off = %ld to rnei = %d",             \
                         next_off,                                      \
                         rnei                                           \
                         );                                             \
        }                                                               \
                                                                        \
        /* send my array slice to target everywhere */                  \
        {                                                               \
            const long tidx = *acc_off * Bytes;                         \
            int i;                                                      \
            int pe = PE_start;                                          \
                                                                        \
            for (i = 0; i < PE_size; i += 1) {                          \
                shmem_put##Bits(target + tidx, source, nelems, pe);     \
                shmemi_trace(SHMEM_LOG_COLLECT,                         \
                             "put%d: tidx = %ld -> %d",                 \
                             Bits,                                      \
                             tidx,                                      \
                             pe                                         \
                             );                                         \
                pe += step;                                             \
            }                                                           \
        }                                                               \
                                                                        \
        /* clean up, and wait for everyone to finish */                 \
        *acc_off = SHMEM_SYNC_VALUE;                                    \
        shmemi_trace(SHMEM_LOG_COLLECT,                                 \
                     "acc_off before barrier = %ld",                    \
                     *acc_off                                           \
                     );                                                 \
        shmem_barrier(PE_start, logPE_stride, PE_size, pSync);          \
    }

SHMEM_COLLECT (32, 4);
SHMEM_COLLECT (64, 8);
