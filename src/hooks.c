/*
 * these hooks are used to do things before and after certain events.
 *
 * The idea is to allow registration of functions into a chain that can
 * be called for e.g. profiling.
 */


#include "warn.h"

void
__hooks_pre_barrier(void)
{
  __shmem_warn(SHMEM_LOG_DEBUG,
	       "pre-barrier hook call-chain"
	       );
}

void
__hooks_post_barrier(void)
{
  __shmem_warn(SHMEM_LOG_DEBUG,
	       "post-barrier hook call-chain"
	       );
}
