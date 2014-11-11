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



#include "comms.h"
#include "trace.h"
#include "atomic.h"

#include "shmem.h"

void
__shmem_barrier_linear (int PE_start, int logPE_stride, int PE_size,
                        long *pSync)
{
  const int me = _my_pe ();
  const int step = 1 << logPE_stride;
  const long nreplies = _SHMEM_SYNC_VALUE + PE_size - 1;
  int i, round;
  int thatpe;

  for (round = 0; round < 2; round += 1)
    {

      for (thatpe = PE_start, i = 0; i < PE_size; thatpe += step, i += 1)
        {

          if (thatpe != me)
            {
              shmem_long_inc (&pSync[round], thatpe);

              __shmem_trace (SHMEM_LOG_BARRIER,
                             "round = %d, sent increment to PE %d",
                             round, thatpe);
            }

        }
      shmem_long_wait_until (&pSync[round], _SHMEM_CMP_EQ, nreplies);

      pSync[round] = _SHMEM_SYNC_VALUE;

    }
}
