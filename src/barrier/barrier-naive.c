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
    const long wf = _SHMEM_SYNC_VALUE + PE_size - 1;

    /* phase 1: root signals everyone else */
    for (thatpe = root, i = 1; i < PE_size; i += 1) {
      thatpe += step;
      shmem_long_inc(& pSync[0], thatpe);
    }

    /* phase 2: waits for acks */
    shmem_long_wait_until(& pSync[1], _SHMEM_CMP_LT, wf);

    __shmem_trace(SHMEM_LOG_BARRIER,
		  "root finishes"
		  );
  }
  else {

    /* 1: non-root waits for root to say "go" */
    shmem_long_wait(& pSync[0], _SHMEM_SYNC_VALUE);

    /* 2: then tell root */
    shmem_long_inc(& pSync[1], root);

  }

  pSync[0] = pSync[1] = _SHMEM_SYNC_VALUE;
}



#if 0
void
__shmem_barrier_naive(int PE_start, int logPE_stride, int PE_size, long *pSync)
{
  const int me = GET_STATE(mype);
  const int root = PE_start;

  if (me == root) {
    const int step = 1 << logPE_stride;
    int i;
    int thatpe;

    /* phase 1: root waits for everyone else */
    for (thatpe = root, i = 1; i < PE_size; i += 1) {
      thatpe += step;
      shmem_long_wait(& pSync[thatpe], _SHMEM_SYNC_VALUE);
    }

    /* phase 2: root signals everyone else, reset pSync */
    for (thatpe = root, i = 1; i < PE_size; i += 1) {
      thatpe += step;
      shmem_long_p(& pSync[thatpe], ~ _SHMEM_SYNC_VALUE, thatpe);
      pSync[thatpe] = _SHMEM_SYNC_VALUE;
    }
  }
  else {
    /* 1: non-root signals root it has arrived */
    shmem_long_p(& pSync[me], ~ _SHMEM_SYNC_VALUE, root);
    /* 2: then wait for root to ack, and reset pSync */
    shmem_long_wait(& pSync[me], _SHMEM_SYNC_VALUE);
    pSync[me] = _SHMEM_SYNC_VALUE;
  }
}
#endif
