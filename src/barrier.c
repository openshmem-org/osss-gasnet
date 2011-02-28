#include <stdio.h>
#include <strings.h>

#include "comms.h"
#include "trace.h"
#include "utils.h"

/*
 * pull-in all the implementations here
 *
 */

#include "barrier-naive.h"

/*
 * these are what we use if nothing else available
 *
 */

static char *DEFAULT_BARRIER_ALL_ALGORITHM = "naive";
static char *DEFAULT_BARRIER_ALGORITHM     = "naive";

/*
 * handlers for barrier implementations
 *
 */

typedef void (*dispatch_function)();

static dispatch_function bar_all_func = NULL;
static dispatch_function bar_func = NULL;

/*
 * build tables of names and corresponding functions to call.  We
 * split up barrier and barrier_all to allow different implementations
 * of each.
 *
 */

typedef struct {
  char *name;
  dispatch_function func;
} bar_table_t;

/*
 * the barrier_all versions
 *
 */

static bar_table_t barrier_all_table[] =
  {
    { "naive", pshmem_barrier_all_naive },
  };
static const int n_barrier_all =
  sizeof(barrier_all_table) / sizeof(barrier_all_table[0]);

/*
 * the barrier versions
 *
 */

static bar_table_t barrier_table[] =
  {
    { "naive", pshmem_barrier_naive },
  };
static const int n_barrier =
  sizeof(barrier_table) / sizeof(barrier_table[0]);

/*
 * called during library initialization to find the right barrier
 * algorithm(s).  Return function pointer, or NULL if we can't find
 * the name.
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
  return (dispatch_function) NULL;
}

/*
 * called during initialization of shmem
 *
 */

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

  bar_all_name = __shmem_comms_getenv("SHMEM_BARRIER_ALL_ALGORITHM");
  if (bar_all_name == (char *) NULL) {
    bar_all_name = DEFAULT_BARRIER_ALL_ALGORITHM;
  }
  batp = barrier_all_table;
  bar_all_func = lookup(batp, n_barrier_all, bar_all_name);
  if (bar_all_func == (dispatch_function) NULL) {
    __shmem_trace(SHMEM_LOG_BARRIER,
		  "unknown barrier_all alogrithm \"%s\", using default \"%s\"",
		  bar_all_name, DEFAULT_BARRIER_ALL_ALGORITHM
		  );
    bar_all_func = lookup(batp, n_barrier_all, DEFAULT_BARRIER_ALL_ALGORITHM);
  }

  /*
   * choose the barrier (could be different from _all)
   *
   */

  bar_name = __shmem_comms_getenv("SHMEM_BARRIER_ALGORITHM");
  if (bar_name == (char *) NULL) {
    bar_name = DEFAULT_BARRIER_ALGORITHM;
  }
  btp = barrier_table;
  bar_func = lookup(btp, n_barrier, bar_name);
  if (bar_func == (dispatch_function) NULL) {
    __shmem_trace(SHMEM_LOG_BARRIER,
		  "unknown barrier alogrithm \"%s\", using default \"%s\"",
		  bar_name, DEFAULT_BARRIER_ALGORITHM
		  );
    bar_func = lookup(btp, n_barrier, DEFAULT_BARRIER_ALGORITHM);
  }

  /*
   * report which barrier implementations we set up
   */
  __shmem_trace(SHMEM_LOG_BARRIER,
		"using barrier \"%s\", barrier_all \"%s\"",
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
  INIT_CHECK();

  if (bar_all_func == (dispatch_function) NULL) {
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
  INIT_CHECK();

  if (bar_func == (dispatch_function) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: no barrier handler defined"
		  );
    /* NOT REACHED */
  }

  (*bar_func)(PE_start, logPE_stride, PE_size, pSync);
}

#pragma weak shmem_barrier_all = pshmem_barrier_all
#pragma weak shmem_barrier = pshmem_barrier
