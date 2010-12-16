/*
 * allow runtime choice of barrier (or other) algorithm.  Set up a
 * dispatch table for such functions.  Each index has an assoicated
 * symbol that is mapped to a function for that algorithm.
 * shmem_barrier_all() etc. then becomes a wrapper to dereference and
 * call that pointer.
 */

#include <stdlib.h>
#include <strings.h>

#include "barrier.h"
#include "warn.h"
#include "dispatch.h"
#include "comms.h"

#define DISPATCH_MAX 128

__shmem_dispatch_t __shmem_dispatch[DISPATCH_MAX];

void
__dispatch_init(void)
{
  __shmem_dispatch[SHMEM_BARRIER_DISPATCH] = DISPATCH_NULL;
}

static barrier_dispatches_t  barrier_table[] =
  {
    { "basic", __shmem_barrier_all_basic },
    { "basic", __shmem_barrier_basic     }
  };
static const int n_dispatches =
  sizeof(barrier_table) / sizeof(barrier_table[0]);

static const char *shmem_barrier_algorithm_envvar = "SHMEM_BARRIER_ALGORITHM";
static char *shmem_default_barrier_algorithm = "basic";

void
__barrier_dispatch_init()
{
  char *ba = __comms_getenv(shmem_barrier_algorithm_envvar);

  if (ba == (char *) NULL) {
    ba = shmem_default_barrier_algorithm;
  }

  {
    int i = 0;
    int found = 0;
    barrier_dispatches_t *dmp = barrier_table;

    for ( ; i < n_dispatches; i += 1){
      if (strcasecmp(ba, dmp->name) == 0) {
	__shmem_dispatch[SHMEM_BARRIER_DISPATCH] = dmp->func;
	found = 1;
	break;
      }
      else {
	dmp += 1;
      }
    }
    
    if (! found) {
      __shmem_warn(SHMEM_LOG_FATAL,
		   "unknown barrier algorithm \"%s\"",
		   ba
		   );
      /* NOT REACHED */
    }
  }
}
