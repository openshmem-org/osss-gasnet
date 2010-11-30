/*
 * set lock: wait for lock to become 0.  then set to 1, and send 1 to
 * everyone else.
 *
 * clear lock: if non-0, flush all I/O in critical region & set to zero.
 *
 * test lock: check, then set only if 0.  return 0 if able to set,
 * return 1 if not.
 */

#include "warn.h"

static void
unimpl_helper(char *name)
{
  __shmem_warn(SHMEM_LOG_FATAL,
	       "shmem_%s_lock not yet implemented",
	       name
	       );
}

void
shmem_clear_lock(long *lock)
{
  unimpl_helper("clear");
}

void
shmem_set_lock(long *lock)
{
  unimpl_helper("set");
}

int
shmem_test_lock(long *lock)
{
  unimpl_helper("test");
}

#ifdef HAVE_PSHMEM_SUPPORT
_Pragma("weak pshmem_clear_lock=shmem_clear_lock")
_Pragma("weak pshmem_set_lock=shmem_set_lock")
_Pragma("weak pshmem_test_lock=shmem_test_lock")
#endif /* HAVE_PSHMEM_SUPPORT */
