#include "comms.h"
#include "trace.h"
#include "atomic.h"

#include "mpp/shmem.h"

void
__shmem_barrier_all_naive(void)
{
  __shmem_comms_barrier_all();
}

void
__shmem_barrier_naive(int PE_start, int logPE_stride, int PE_size, long *pSync)
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
      }

    }

    shmem_long_wait_until(& pSync[round], _SHMEM_CMP_EQ, nreplies);
    pSync[round] = _SHMEM_SYNC_VALUE;

  }
}
