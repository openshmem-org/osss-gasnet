#include <stdio.h>               /* NULL                           */

#include "state.h"

/* ----------------------------------------------------------------- */

#define SHMEM_MY_PE(Variant) \
  int			     \
  Variant (void)	     \
  {			     \
    return __state.mype;     \
  }

SHMEM_MY_PE(shmem_my_pe)
SHMEM_MY_PE(my_pe)
SHMEM_MY_PE(_my_pe)

#define SHMEM_NUM_PES(Variant)			\
  int						\
  Variant (void)				\
  {						\
    return __state.numpes;			\
  }

SHMEM_NUM_PES(shmem_num_pes)
SHMEM_NUM_PES(shmem_n_pes)
SHMEM_NUM_PES(num_pes)
SHMEM_NUM_PES(_num_pes)

char *
shmem_nodename(void)
{
  return __state.loc.nodename;
}


#ifdef HAVE_PSHMEM_SUPPORT
_Pragma("weak pshmem_my_pe=shmem_my_pe")
_Pragma("weak pmy_pe=my_pe")
_Pragma("weak p_my_pe=_my_pe")

_Pragma("weak pshmem_num_pes=shmem_num_pes")
_Pragma("weak pshmem_n_pes=shmem_n_pes")
_Pragma("weak pnum_pes=num_pes")
_Pragma("weak p_num_pes=_num_pes")
#endif /* HAVE_PSHMEM_SUPPORT */
