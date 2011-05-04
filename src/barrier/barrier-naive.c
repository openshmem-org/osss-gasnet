#include "state.h"
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
  const int me = GET_STATE(mype);
  const int root = PE_start;

  if (me == root) {
    const int step = 1 << logPE_stride;
    int i;
    int thatpe;

    /* phase 1: root signals everyone else */
    for (thatpe = root, i = 1; i < PE_size; i += 1) {
      thatpe += step;
      shmem_long_p(& pSync[thatpe], ~ _SHMEM_SYNC_VALUE, thatpe);
    }

    /* phase 2: root waits for everyone else and resets pSync */
    for (thatpe = root, i = 1; i < PE_size; i += 1) {
      thatpe += step;
      shmem_long_wait(& pSync[thatpe], ~ _SHMEM_SYNC_VALUE);

      pSync[thatpe] = _SHMEM_SYNC_VALUE;
    }

    /* phase 3: root says it is done too */
    for (thatpe = root, i = 1; i < PE_size; i += 1) {
      thatpe += step;
      shmem_long_p(& pSync[root], ~ _SHMEM_SYNC_VALUE, thatpe);
    }
  }
  else {
    /* 1: non-root waits for root to poke it */
    shmem_long_wait(& pSync[me], _SHMEM_SYNC_VALUE);
    /* 2: then reset and ack */
    pSync[me] = _SHMEM_SYNC_VALUE;
    shmem_long_p(& pSync[me], pSync[me], root);
    /* 3: final ack from root */
    shmem_long_wait(& pSync[root], _SHMEM_SYNC_VALUE);
    pSync[root] = _SHMEM_SYNC_VALUE;
  }
}
