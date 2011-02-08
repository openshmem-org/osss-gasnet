#include "state.h"
#include "comms.h"
#include "warn.h"
/* #include "dispatch.h" */
#include "comms.h"
#include "hooks.h"

/* @api@ */
void
pshmem_barrier_all(void)
{
  __comms_barrier_all();
}

#if 0

/*
 * the SGI man pages say the bare name "barrier" is a synonym for
 * "shmem_barrier_all", but the barrier symbol does not appear in the
 * SMA library.
 *
 */

#pragma weak barrier = pshmem_barrier_all

#pragma weak barrier = pbarrier

#endif /* 0 */

/* @api@ */
void
pshmem_barrier(int PE_start, int logPE_stride, int PE_size, long *pSync)
{
  __comms_barrier(PE_start, logPE_stride, PE_size, pSync);
}

#pragma weak shmem_barrier_all = pshmem_barrier_all
#pragma weak shmem_barrier = pshmem_barrier
