/*
 * so apparently these are compatibility routines
 * for older SGI architectures.  So probably fine
 * to let them all be empty
 */

#include "warn.h"

#define CACHE_NO_OP(Name, Params)			\
  /* @api@ */						\
  void							\
  Name ( Params )					\
  {							\
    __shmem_warn(SHMEM_LOG_CACHE,			\
		 "operation \"%s\" is a no-op",		\
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


#ifdef HAVE_PSHMEM_SUPPORT
_Pragma("weak pshmem_clear_cache_inv=shmem_clear_cache_inv")
_Pragma("weak pshmem_set_cache_inv=shmem_set_cache_inv")
_Pragma("weak pshmem_clear_cache_line_inv=shmem_clear_cache_line_inv")
_Pragma("weak pshmem_set_cache_line_inv=shmem_set_cache_line_inv")
_Pragma("weak pshmem_udcflush=shmem_udcflush")
_Pragma("weak pshmem_udcflush_line=shmem_udcflush_line")
#endif /* HAVE_PSHMEM_SUPPORT */
