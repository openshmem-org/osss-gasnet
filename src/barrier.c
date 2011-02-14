#include <stdio.h>
#include <strings.h>

#include "comms.h"
#include "trace.h"
#include "barrier-naive.h"

static char *DEFAULT_BARRIER_ALL_ALGORITHM = "naive";
static char *DEFAULT_BARRIER_ALGORITHM     = "naive";

typedef void (*dispatch_function)();

static dispatch_function bar_func = NULL;
static dispatch_function bar_all_func = NULL;

typedef struct {
  char *name;
  dispatch_function func;
} bar_table_t;

static bar_table_t barrier_all_table[] =
  {
    { "naive", pshmem_barrier_all_naive },
  };
static const int n_barrier_all =
  sizeof(barrier_all_table) / sizeof(barrier_all_table[0]);

static bar_table_t barrier_table[] =
  {
    { "naive", pshmem_barrier_naive },
  };
static const int n_barrier =
  sizeof(barrier_table) / sizeof(barrier_table[0]);

/*
 * called during library initialization to find the right barrier
 * algorithm(s)
 *
 */

static dispatch_function
lookup(bar_table_t *tp, int n, char *name)
{
  int i;

  for (i = 0; i < n; i += 1) {
    if (strcasecmp(name, tp->name) == 0) {
      return tp->func;
      break;
      /* NOT REACHED */
    }
    tp += 1;
  }
  return NULL;
}

void
__barrier_dispatch_init(void)
{
  char *bar_all_name;
  bar_table_t *batp;
  char *bar_name;
  bar_table_t *btp;

  /*
   * choose the barrier_all
   *
   */

  bar_all_name = __comms_getenv("SHMEM_BARRIER_ALL_ALGORITHM");
  if (bar_all_name == NULL) {
    bar_all_name = DEFAULT_BARRIER_ALL_ALGORITHM;
  }
  batp = barrier_all_table;
  bar_all_func = lookup(batp, n_barrier_all, bar_all_name);
  if (bar_all_func == NULL) {
    __shmem_trace(SHMEM_LOG_INIT,
		  "unknown barrier_all alogrithm \"%s\", using default",
		  bar_all_name
		  );
    bar_all_func = lookup(batp, n_barrier_all, DEFAULT_BARRIER_ALL_ALGORITHM);
  }

  /*
   * choose the barrier (could be different from _all)
   *
   */

  bar_name = __comms_getenv("SHMEM_BARRIER_ALGORITHM");
  if (bar_name == NULL) {
    bar_name = DEFAULT_BARRIER_ALGORITHM;
  }
  btp = barrier_table;
  bar_func = lookup(btp, n_barrier, bar_name);
  if (bar_func == NULL) {
    __shmem_trace(SHMEM_LOG_INIT,
		  "unknown barrier alogrithm \"%s\", using default",
		  bar_name
		  );
    bar_func = lookup(btp, n_barrier, DEFAULT_BARRIER_ALGORITHM);
  }

  __shmem_trace(SHMEM_LOG_INIT,
		"using \"%s\" barrier, \"%s\"",
		bar_name, bar_all_name
		);
}

/*
 * the rest is what library users see
 *
 */


/* @api@ */
void
pshmem_barrier_all(void)
{
  if (bar_all_func == NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: no barrier_all handler defined"
		  );
    /* NOT REACHED */
  }

  (*bar_all_func)();
}

/* @api@ */
void
pshmem_barrier(int PE_start, int logPE_stride, int PE_size, long *pSync)
{
  if (bar_func == NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: no barrier handler defined"
		  );
    /* NOT REACHED */
  }

  (*bar_func)(PE_start, logPE_stride, PE_size, pSync);
}

#pragma weak shmem_barrier_all = pshmem_barrier_all
#pragma weak shmem_barrier = pshmem_barrier

#if 0

/*
 * the SGI man pages say the bare name "barrier" is a synonym for
 * "shmem_barrier_all", but the barrier symbol does not appear in the
 * SMA library.
 *
 */

#pragma weak pbarrier = pshmem_barrier_all

#pragma weak barrier = pbarrier

#endif /* 0 */
