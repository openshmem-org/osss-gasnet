#include "state.h"
#include "comms.h"
#include "trace.h"
#include "comms.h"

#include "shmem.h"

void
pshmem_barrier_all_naive(void)
{
  __shmem_comms_barrier_all();
}

void
pshmem_barrier_naive(int PE_start, int logPE_stride, int PE_size, long *pSync)
{
  shmem_fence();

  if (GET_STATE(mype) == PE_start) {
    const int step = 1 << logPE_stride;
    int i;
    int thatpe;
    /* root signals everyone else */
    for (thatpe = PE_start, i = 1; i < PE_size; i += 1) {
      thatpe += step;
      shmem_long_p(& pSync[thatpe], ~ _SHMEM_SYNC_VALUE, thatpe);
    }
    /* root waits for ack from everyone else */
    for (thatpe = PE_start, i = 1; i < PE_size; i += 1) {
      thatpe += step;
      shmem_wait(& pSync[thatpe], ~ _SHMEM_SYNC_VALUE);
    }
  }
  else {
    /* non-root waits for root to signal, then tell root we're ready */
    shmem_wait(& pSync[GET_STATE(mype)], _SHMEM_SYNC_VALUE);
    shmem_long_p(& pSync[GET_STATE(mype)], _SHMEM_SYNC_VALUE, PE_start);
  }
  /* restore pSync values */
  pSync[GET_STATE(mype)] = _SHMEM_SYNC_VALUE;

  shmem_fence();
}
