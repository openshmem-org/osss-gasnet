#include "state.h"
#include "comms.h"
#include "trace.h"
#include "atomic.h"

#include "shmem.h"

void
__shmem_barrier_all_naive(void)
{
  __shmem_comms_barrier_all();
}

void
__shmem_barrier_naive(int PE_start, int logPE_stride, int PE_size, long *pSync)
{
  const int me = GET_STATE(mype);

  if (me == PE_start) {
    const int step = 1 << logPE_stride;
    int i;
    int thatpe;

    /* phase 1: root waits for hello from everyone else */
    for (thatpe = PE_start, i = 1; i < PE_size; i += 1) {
      thatpe += step;
      shmem_long_wait(& pSync[thatpe], _SHMEM_SYNC_VALUE);
    }

    /* phase 2: root signals everyone else */
    for (thatpe = PE_start, i = 1; i < PE_size; i += 1) {
      thatpe += step;
      shmem_long_p(& pSync[thatpe], ~ _SHMEM_SYNC_VALUE, thatpe);
    }

    pSync[thatpe] = _SHMEM_SYNC_VALUE;
  }
  else {
    /* non-root tells root, then waits for ack */
    shmem_long_p(& pSync[me], ~ _SHMEM_SYNC_VALUE, PE_start);
    shmem_long_wait(& pSync[me], _SHMEM_SYNC_VALUE);

    pSync[me] = _SHMEM_SYNC_VALUE;
  }
}
