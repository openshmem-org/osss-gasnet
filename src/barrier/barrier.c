/* (c) 2011 University of Houston System.  All rights reserved. */


#include <stdio.h>
#include <strings.h>

#include "comms.h"
#include "trace.h"
#include "utils.h"
#include "modules.h"

#include "mpp/pshmem.h"

/*
 * handlers for broadcast implementations
 *
 */

static module_info_t mi_all, mi_bar;

/*
 * called during initialization of shmem
 *
 */

void
__shmem_barriers_dispatch_init(void)
{
  char *bar_name;
  char *all_name;
  int sa, sb;

  /*
   * choose the barrier_all
   *
   */

  all_name = __shmem_comms_getenv("SHMEM_BARRIER_ALL_ALGORITHM");
  if (all_name == (char *) NULL) {
    all_name = __shmem_modules_get_implementation("barrier-all");
  }
  sa = __shmem_modules_load("barrier-all", all_name, &mi_all);
  if (sa != 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
                  "internal error: couldn't load barrier-all module \"%s\"",
                  all_name
                  );
    /* NOT REACHED */
  }

  /*
   * choose the barrier (could be different from _all)
   *
   */

  bar_name = __shmem_comms_getenv("SHMEM_BARRIER_ALGORITHM");
  if (bar_name == (char *) NULL) {
    bar_name = __shmem_modules_get_implementation("barrier");
  }
  sb = __shmem_modules_load("barrier", bar_name, &mi_bar);
  if (sb != 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
                  "internal error: couldn't load barrier module \"%s\"",
                  bar_name
                  );
    /* NOT REACHED */
  }

  /*
   * report which implementation we set up
   */
  __shmem_trace(SHMEM_LOG_BROADCAST,
                "using barrier_all \"%s\" & barrier \"%s\"",
                all_name,
		bar_name
                );

}

/*
 * the rest is what library users see
 *
 * in this case we don't have the 32/64 bit divide, so we just look at
 * the 32-bit version and use that pointer
 *
 */


/* @api@ */
void
pshmem_barrier_all(void)
{
  INIT_CHECK();

  shmem_fence();

  mi_all.func_32();
}

/* @api@ */
void
pshmem_barrier(int PE_start, int logPE_stride, int PE_size, long *pSync)
{
  INIT_CHECK();

  /* not implied by documetnation: shmem_fence(); */

  mi_bar.func_32(PE_start, logPE_stride, PE_size, pSync);
}

#pragma weak shmem_barrier_all = pshmem_barrier_all
#pragma weak shmem_barrier = pshmem_barrier
