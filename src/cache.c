/*
 * so apparently these are compatibility routines
 * for older SGI architectures.  So probably fine
 * to let them all be empty
 */

/* api@ */
void
shmem_clear_cache_inv(void)
{
  return;
}

/* api@ */
void
shmem_set_cache_inv(void)
{
  return;
}

/* api@ */
void
shmem_clear_cache_line_inv(void *target)
{
  return;
}

/* api@ */
void
shmem_set_cache_line_inv(void *target)
{
  return;
}

/* api@ */
void
shmem_udcflush(void)
{
  return;
}

/* api@ */
void
shmem_udcflush_line(void *target)
{
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
