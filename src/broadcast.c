#include <stdio.h>
#include <strings.h>

#include "shmem.h"
#include "comms.h"
#include "trace.h"
#include "utils.h"

#include "broadcast-naive.h"
#include "broadcast-tree.h"

/*
 * these are what we use if nothing else available
 *
 */

static char *DEFAULT_BROADCAST_ALGORITHM     = "naive";

/*
 * handlers for broadcast implementations
 *
 */

typedef void (*dispatch_function)();

static dispatch_function broadcast32_func = NULL;
static dispatch_function broadcast64_func = NULL;

/*
 * build tables of names and corresponding functions to call.
 */

typedef struct {
  char *name;
  dispatch_function func;
} broadcast_table_t;

static broadcast_table_t broadcast32_table[] =
  {
    { "naive", pshmem_broadcast32_naive },
    { "tree", pshmem_broadcast32_tree },
  };
static const int n_broadcast32 = TABLE_SIZE(broadcast32_table);

static broadcast_table_t broadcast64_table[] =
  {
    { "naive", pshmem_broadcast64_naive },
    { "tree", pshmem_broadcast64_tree },
  };
static const int n_broadcast64 =
  sizeof(broadcast64_table) / sizeof(broadcast64_table[0]);


/*
 * called during library initialization to find the right broadcast
 * algorithm.  Return function pointer, or NULL if we can't find
 * the name.
 *
 */

static dispatch_function
lookup(broadcast_table_t *tp, int n, char *name)
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
__broadcast_dispatch_init(void)
{
  char *broadcast32_name, *broadcast64_name;
  broadcast_table_t *brtp32, *brtp64;

  /*
   * choose the broadcast
   *
   */

  broadcast32_name =  __shmem_comms_getenv("SHMEM_BROADCAST_ALGORITHM");
  if (broadcast32_name == (char *) NULL) {
    broadcast32_name = DEFAULT_BROADCAST_ALGORITHM;
  }
  brtp32 = broadcast32_table;
  broadcast32_func = lookup(brtp32, n_broadcast32, broadcast32_name);
  if (broadcast32_func == (dispatch_function) NULL) {
    __shmem_trace(SHMEM_LOG_BROADCAST,
		  "unknown broadcast alogrithm \"%s\", using default \"%s\"",
		  broadcast32_name, DEFAULT_BROADCAST_ALGORITHM
		  );
    broadcast32_func = lookup(brtp32, n_broadcast32, DEFAULT_BROADCAST_ALGORITHM);
  }


 broadcast64_name =  __shmem_comms_getenv("SHMEM_BROADCAST_ALGORITHM");
  if (broadcast64_name == (char *) NULL) {
    broadcast64_name = DEFAULT_BROADCAST_ALGORITHM;
  }
  brtp64 = broadcast64_table;
  broadcast64_func = lookup(brtp64, n_broadcast64, broadcast64_name);
  if (broadcast64_func == (dispatch_function) NULL) {
    __shmem_trace(SHMEM_LOG_BROADCAST,
                  "unknown broadcast alogrithm \"%s\", using default \"%s\"",
                  broadcast64_name, DEFAULT_BROADCAST_ALGORITHM
                  );
    broadcast64_func = lookup(brtp64, n_broadcast64, DEFAULT_BROADCAST_ALGORITHM);
  }


  /*
   * report which broadcast implementations we set up
   */
  __shmem_trace(SHMEM_LOG_BROADCAST,
		"using broadcast \"%s\"",
		broadcast32_name
		);
}

/*
 * the rest is what library users see
 *
 */


/* @api@ */
void
pshmem_broadcast32(void *target, const void *source, size_t nlong,
		   int PE_root, int PE_start, int logPE_stride, int PE_size,
		   long *pSync)
{
  if (broadcast32_func == (dispatch_function) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: no broadcast handler defined"
		  );
    /* NOT REACHED */
  }

  (*broadcast32_func)(target, source, nlong,
		      PE_root, PE_start, logPE_stride, PE_size,
		      pSync);
}


/* @api@ */
void
pshmem_broadcast64(void *target, const void *source, size_t nlong,
		   int PE_root, int PE_start, int logPE_stride, int PE_size,
		   long *pSync)
{
  if (broadcast64_func == (dispatch_function) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
                  "internal error: no broadcast handler defined"
                  );
    /* NOT REACHED */
  }

  (*broadcast64_func)(target, source, nlong,
		      PE_root, PE_start, logPE_stride, PE_size,
		      pSync);
}

#pragma weak shmem_broadcast32 = pshmem_broadcast32
#pragma weak shmem_broadcast64 = pshmem_broadcast64

/* @api@ */
void
pshmem_sync_init(long *pSync){
        const int nb = _SHMEM_BCAST_SYNC_SIZE;
        int i;

        for (i = 0; i < nb; i += 1) {
                pSync[i] = _SHMEM_SYNC_VALUE;
        }
        shmem_barrier_all();
}

#pragma weak shmem_sync_init = pshmem_sync_init
