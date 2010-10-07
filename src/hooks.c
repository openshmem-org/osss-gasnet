/*
 * these hooks are used to do things before and after certain events.
 *
 * The idea is to allow registration of functions into a chain that can
 * be called for e.g. profiling.
 *
 * Need to build out a framework for enumerating hooks: this is just
 * an outline to work things out
 */

#include <stdio.h>

#include "warn.h"

typedef void (*hook_func)(void);

struct hook_info {
  hook_func chain;
  int count;
};

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
