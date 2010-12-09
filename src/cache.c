/*
 * so apparently these are compatibility routines
 * for older SGI architectures.  So probably fine
 * to let them all be empty
 */

#include "warn.h"

static void inline
cache_no_op(void)
{
  __shmem_warn(SHMEM_LOG_DEBUG,
	       "cache operations are a no-op"
	       );
}

/* @api@ */
void
shmem_clear_cache_inv(void)
{
  cache_no_op();
  return;
}

/* @api@ */
void
shmem_set_cache_inv(void)
{
  cache_no_op();
  return;
}

/* @api@ */
void
shmem_clear_cache_line_inv(void *target)
{
  cache_no_op();
  return;
}

/* @api@ */
void
shmem_set_cache_line_inv(void *target)
{
  cache_no_op();
  return;
}

/* @api@ */
void
shmem_udcflush(void)
{
  cache_no_op();
  return;
}

/* @api@ */
void
shmem_udcflush_line(void *target)
{
  cache_no_op();
  return;
}

#ifdef HAVE_PSHMEM_SUPPORT
_Pragma("weak pshmem_clear_cache_inv=shmem_clear_cache_inv")
_Pragma("weak pshmem_set_cache_inv=shmem_set_cache_inv")
_Pragma("weak pshmem_clear_cache_line_inv=shmem_clear_cache_line_inv")
_Pragma("weak pshmem_set_cache_line_inv=shmem_set_cache_line_inv")
_Pragma("weak pshmem_udcflush=shmem_udcflush")
_Pragma("weak pshmem_udcflush_line=shmem_udcflush_line")
#endif /* HAVE_PSHMEM_SUPPORT */
