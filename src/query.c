#include "state.h"
#include "trace.h"
#include "utils.h"

/*
 * these routines handle the questions "how many PEs?" and "which PE
 * am I?".  Also added an initial thought about locality with the
 * nodename query
 *
 */

#define SHMEM_MY_PE(Variant)			\
  int						\
  p##Variant (void)				\
  {						\
    INIT_CHECK();				\
    return GET_STATE(mype);			\
  }

SHMEM_MY_PE(my_pe)
SHMEM_MY_PE(_my_pe)

#define SHMEM_NUM_PES(Variant)			\
  int						\
  p##Variant (void)				\
  {						\
    INIT_CHECK();				\
    return GET_STATE(numpes);			\
  }

SHMEM_NUM_PES(num_pes)
SHMEM_NUM_PES(_num_pes)

char *
pshmem_nodename(void)
{
  INIT_CHECK();
  return GET_STATE(loc.nodename);
}


#pragma weak my_pe = pmy_pe
#pragma weak _my_pe = p_my_pe

#pragma weak num_pes = pnum_pes
#pragma weak _num_pes = p_num_pes

#pragma weak shmem_nodename = pshmem_nodename

#ifdef CRAY_COMPAT
SHMEM_NUM_PES(shmem_num_pes)
SHMEM_NUM_PES(shmem_n_pes)
SHMEM_MY_PE(shmem_my_pe)

#pragma weak shmem_my_pe = pshmem_my_pe
#pragma weak shmem_num_pes = pshmem_num_pes
#pragma weak shmem_n_pes = pshmem_n_pes
#endif /* CRAY_COMPAT */

/*
 * check the target PE "pe" is within the program range
 *
 */

void
__shmem_pe_range_check(int pe)
{
  const int top_pe = GET_STATE(numpes) - 1;

  if (pe < 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "Target PE out of range (%d < %d)",
		  pe,
		  0
		  );
    /* NOT REACHED */
  }
  else if (pe > top_pe) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "Target PE out of range (%d > %d)",
		  pe,
		  top_pe
		  );
    /* NOT REACHED */
  }
}
