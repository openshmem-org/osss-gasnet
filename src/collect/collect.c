/* (c) 2011 University of Houston.  All rights reserved. */


#include <sys/types.h>

#include "state.h"
#include "putget.h"
#include "trace.h"
#include "symmem.h"
/* #include "globalvar.h" */
#include "utils.h"
#include "atomic.h"

#include "mpp/shmem.h"

/*
 * collect puts nelems (can vary from PE to PE) from source on each
 * PE in the set to target on all PEs in the set.  source -> target is
 * done in PE order.
 *
 * We set up a wavefront that propagates the accumulated offsets and
 * then overlaps forwarding the extra offset contribution from each PE
 * with actually sending source -> target.  Control data goes left to
 * right, application data goes "top to bottom", as it were.
 *
 */

#define SHMEM_COLLECT(Bits, Bytes)					\
  /* @api@ */								\
  void									\
  pshmem_collect##Bits(void *target, const void *source, size_t nelems,	\
		       int PE_start, int logPE_stride, int PE_size,	\
		       long *pSync)					\
  {									\
    const int step = 1 << logPE_stride;					\
    const int last_pe = PE_start + step * (PE_size - 1);		\
    const int me = GET_STATE(mype);					\
    long *acc_off = & (pSync[0]);					\
									\
    INIT_CHECK();							\
    SYMMETRY_CHECK(target, 1, "shmem_collect");				\
    SYMMETRY_CHECK(source, 2, "shmem_collect");				\
									\
    __shmem_trace(SHMEM_LOG_COLLECT,					\
		  "PE_start = %d, PE_stride = %d, PE_size = %d, last_pe = %d", \
		  PE_start,						\
		  step,							\
		  PE_size,						\
		  last_pe						\
		  );							\
									\
    /* initialize left-most or wait for left-neighbor to notify */	\
    if (me == PE_start) {						\
      *acc_off = 0;							\
    }									\
									\
    shmem_long_wait(acc_off, _SHMEM_SYNC_VALUE);			\
    __shmem_trace(SHMEM_LOG_COLLECT,					\
		  "got acc_off = %ld",					\
		  *acc_off						\
		  );							\
									\
    /*									\
     * forward my contribution to (notify) right neighbor if not last PE \
     * in set								\
     */									\
    if (me < last_pe) {							\
      long next_off = *acc_off + nelems;				\
      int rnei = me + step;						\
									\
      shmem_long_p(acc_off, next_off, rnei);				\
									\
      __shmem_trace(SHMEM_LOG_COLLECT,					\
		    "put next_off = %ld to rnei = %d",			\
		    next_off,						\
		    rnei						\
		    );							\
    }									\
									\
    /* send my array slice to target everywhere */			\
    {									\
      long tidx = *acc_off * Bytes;					\
      int i;								\
      int pe = PE_start;						\
									\
      for (i = 0; i < PE_size; i += 1) {				\
	shmem_put##Bits(target + tidx, source, nelems, pe);		\
	__shmem_trace(SHMEM_LOG_COLLECT,				\
		      "put%d: tidx = %ld -> %d",			\
		      Bits,						\
		      tidx,						\
		      pe						\
		      );						\
	pe += step;							\
      }									\
    }									\
									\
    /* clean up, and wait for everyone to finish */			\
    *acc_off = _SHMEM_SYNC_VALUE;					\
    __shmem_trace(SHMEM_LOG_COLLECT,					\
		  "acc_off before barrier = %ld",			\
		  *acc_off						\
		  );							\
    shmem_barrier(PE_start, logPE_stride, PE_size, pSync);		\
  }

SHMEM_COLLECT(32, 4)
SHMEM_COLLECT(64, 8)

#pragma weak shmem_collect32 = pshmem_collect32
#pragma weak shmem_collect64 = pshmem_collect64
