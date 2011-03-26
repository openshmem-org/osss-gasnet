#include "state.h"
#include "comms.h"
#include "trace.h"
#include "atomic.h"

#include "shmem.h"

void
pshmem_barrier_all_naive(void)
{
  __shmem_comms_barrier_all();
}

void
pshmem_barrier_naive(int PE_start, int logPE_stride, int PE_size, long *pSync)
{
  const int me = GET_STATE(mype);
  const long save = pSync[0];

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
      shmem_long_p(& pSync[thatpe], _SHMEM_SYNC_VALUE - 1, thatpe);
    }
  }
  else {
    /* non-root tells root, then waits for ack */
    shmem_long_p(& pSync[me], _SHMEM_SYNC_VALUE - 1, PE_start);
    shmem_long_wait(& pSync[me], _SHMEM_SYNC_VALUE);
  }
  /* restore pSync values */
  pSync[me] = save;
}
