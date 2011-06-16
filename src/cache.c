/* (c) 2011 University of Houston.  All rights reserved. */


/*
 * so apparently these are compatibility routines
 * for older SGI architectures.  So probably fine
 * to let them all be empty
 */

#include "utils.h"
#include "trace.h"

#define CACHE_NO_OP(Name, Params)			\
  /* @api@ */						\
  void							\
  p##Name ( Params )					\
  {							\
    INIT_CHECK();					\
    __shmem_trace(SHMEM_LOG_CACHE,			\
		  "operation \"%s\" is a no-op",	\
		  #Name					\
		  );					\
    return;						\
  }


CACHE_NO_OP(shmem_clear_cache_inv, void)
CACHE_NO_OP(shmem_set_cache_inv, void)
CACHE_NO_OP(shmem_clear_cache_line_inv, void *target)
CACHE_NO_OP(shmem_set_cache_line_inv, void *target)
CACHE_NO_OP(shmem_udcflush, void)
CACHE_NO_OP(shmem_udcflush_line, void *target)


#pragma weak shmem_clear_cache_inv = pshmem_clear_cache_inv
#pragma weak shmem_set_cache_inv = pshmem_set_cache_inv
#pragma weak shmem_clear_cache_line_inv = pshmem_clear_cache_line_inv
#pragma weak shmem_set_cache_line_inv = pshmem_set_cache_line_inv
#pragma weak shmem_udcflush = pshmem_udcflush
#pragma weak shmem_udcflush_line = pshmem_udcflush_line
