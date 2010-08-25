/*
 * allow runtime choice of barrier (or other) algorithm.  Set up a
 * dispatch table for such functions.  Each index has an assoicated
 * symbol that is mapped to a function for that algorithm.
 * shmem_barrier_all() etc. then becomes a wrapper to dereference and
 * call that pointer.
 */

#include <stdlib.h>

#include "barrier.h"
#include "warn.h"
#include "dispatch.h"

#define DISPATCH_MAX 128

__shmem_dispatch_t __shmem_dispatch[DISPATCH_MAX];

void
__dispatch_init(void)
{
  __shmem_dispatch[SHMEM_BARRIER_DISPATCH] = DISPATCH_NULL;
}

#define BARRIER_TABLE_ENTRY(n) { #n , __shmem_barrier_all_##n }

typedef struct {
  const char *name;
  __shmem_dispatch_t func;
} __barrier_dispatches_t;

static __barrier_dispatches_t  __barrier_table[] =
  {
    BARRIER_TABLE_ENTRY(basic)
  };
static const int n_dispatches =
  sizeof(__barrier_table) / sizeof(__barrier_table[0]);

static const char *shmem_barrier_algorithm_envvar = "SHMEM_BARRIER_ALGORITHM";
static char *shmem_default_barrier_algorithm = "basic";

void
__barrier_dispatch_init()
{
  char *ba = getenv(shmem_barrier_algorithm_envvar);

  if (ba == (char *) NULL) {
    ba = shmem_default_barrier_algorithm;
  }

  {
    int i = 0;
    int found = 0;
    __barrier_dispatches_t *dmp = __barrier_table;

    for ( ; !found && i < n_dispatches; i += 1){
      if (strcasecmp(ba, dmp->name) == 0) {
	__shmem_dispatch[SHMEM_BARRIER_DISPATCH] = dmp->func;
	found = 1;
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
    }
  }
}
