/* (c) 2011 University of Houston System.  All rights reserved. */


#include "comms.h"
#include "trace.h"
#include "atomic.h"

#include "mpp/shmem.h"

void
__shmem_barrier_linear(int PE_start, int logPE_stride, int PE_size, long *pSync)
{
  const int me = _my_pe();
  const int step = 1 << logPE_stride;
  const long nreplies = _SHMEM_SYNC_VALUE + PE_size - 1;
  int i, round;
  int thatpe;

  for (round = 0; round < 2; round += 1) {

    for (thatpe = PE_start, i = 0;
	 i < PE_size;
	 thatpe += step, i += 1) {

      if (thatpe != me) {
	shmem_long_inc(& pSync[round], thatpe);

	__shmem_trace(SHMEM_LOG_BARRIER,
		      "round = %d, sent increment to PE %d",
		      round, thatpe
		      );
      }

    }
    shmem_long_wait_until(& pSync[round], _SHMEM_CMP_EQ, nreplies);

    pSync[round] = _SHMEM_SYNC_VALUE;

  }
}

#include "module_info.h"
module_info_t module_info =
  {
    __shmem_barrier_linear,
    __shmem_barrier_linear,
  };
