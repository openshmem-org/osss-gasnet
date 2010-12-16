#include "state.h"
#include "comms.h"
#include "warn.h"
#include "dispatch.h"
#include "comms.h"
#include "hooks.h"

/* @api */
void
shmem_barrier_all(void)
{
  __comms_barrier_all();
}

/* @api */
void
shmem_barrier(int PE_start, int logPE_stride, int PE_size, long *pSync)
{
  __comms_barrier(PE_start, logPE_stride, PE_size, pSync);
}
_Pragma("weak barrier=shmem_barrier")

#ifdef HAVE_PSHMEM_SUPPORT
_Pragma("weak pshmem_barrier_all=shmem_barrier_all")
_Pragma("weak pshmem_barrier_all=shmem_barrier_all")
_Pragma("weak pshmem_barrier=shmem_barrier")
_Pragma("weak pbarrier=barrier")
#endif /* HAVE_PSHMEM_SUPPORT */
