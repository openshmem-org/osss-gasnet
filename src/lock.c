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
