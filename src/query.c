#include <stdio.h>               /* NULL                           */

#include "state.h"
#include "stats.h"

/* ----------------------------------------------------------------- */

#define SHMEM_MY_PE(Variant) \
  int			     \
  Variant (void)	     \
  {			     \
    SHMEM_STATS_MY_PE();     \
    return __state.mype;     \
  }

SHMEM_MY_PE(shmem_my_pe)
SHMEM_MY_PE(my_pe)
SHMEM_MY_PE(_my_pe)
SHMEM_MY_PE(shmem_my_pe_)	/* fortran */
SHMEM_MY_PE(my_pe_)		/* fortran */

#define SHMEM_NUM_PES(Variant)			\
  int						\
  Variant (void)				\
  {						\
    SHMEM_STATS_NUM_PES();			\
    return __state.numpes;			\
  }

SHMEM_NUM_PES(shmem_num_pes)
SHMEM_NUM_PES(num_pes)
SHMEM_NUM_PES(_num_pes)
SHMEM_NUM_PES(shmem_num_pes_)	/* fortran */
SHMEM_NUM_PES(num_pes_)		/* fortran */

char *
shmem_hostname(void)
{
  SHMEM_STATS_HOSTNAME();
  return __state.hostname;
}

char *
shmem_nodename(void)
{
  SHMEM_STATS_NODENAME();
  return __state.nodename;
}

char *
shmem_version(void)
{
  return "Super Happy Fun SHMEM";
}
