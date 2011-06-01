#include <stdio.h>
#include <strings.h>

#include "comms.h"
#include "trace.h"
#include "utils.h"

#include "mpp/shmem.h"

#include "modules.h"

/*
 * these are what we use if nothing else available
 *
 */

static char *DEFAULT_BROADCAST_ALGORITHM  = "naive";

/*
 * handlers for broadcast implementations
 *
 */

static module_info_t mi;


/*
 * called during initialization of shmem
 *
 */

void
__shmem_broadcast_dispatch_init(void)
{
  char *name;
  int s;

  /*
   * choose the broadcast
   *
   */

  name = __shmem_comms_getenv("SHMEM_BROADCAST_ALGORITHM");
  if (name == (char *) NULL) {
    name = DEFAULT_BROADCAST_ALGORITHM;
  }
  s = __shmem_modules_load("broadcast", name, &mi);
  if (s != 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: couldn't load broadcast module \"%s\"",
		  name
		  );
    /* NOT REACHED */
  }

  /*
   * report which broadcast implementation we set up
   */
  __shmem_trace(SHMEM_LOG_BROADCAST,
		"using broadcast \"%s\"",
		name
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
  SYMMETRY_CHECK(target, 1, "shmem_broadcast32");
  SYMMETRY_CHECK(source, 2, "shmem_broadcast32");

  mi.func_32(target, source, nlong,
	     PE_root, PE_start, logPE_stride, PE_size,
	     pSync);
}


/* @api@ */
void
pshmem_broadcast64(void *target, const void *source, size_t nlong,
		   int PE_root, int PE_start, int logPE_stride, int PE_size,
		   long *pSync)
{
  SYMMETRY_CHECK(target, 1, "shmem_broadcast64");
  SYMMETRY_CHECK(source, 2, "shmem_broadcast64");

  mi.func_64(target, source, nlong,
	     PE_root, PE_start, logPE_stride, PE_size,
	     pSync);
}

#pragma weak shmem_broadcast32 = pshmem_broadcast32
#pragma weak shmem_broadcast64 = pshmem_broadcast64

/* @api@ */
void
pshmem_sync_init(long *pSync)
{
  const int nb = _SHMEM_BCAST_SYNC_SIZE;
  int i;

  SYMMETRY_CHECK(pSync, 1, "shmem_sync_init");

  for (i = 0; i < nb; i += 1) {
    pSync[i] = _SHMEM_SYNC_VALUE;
  }
  shmem_barrier_all();
}

#pragma weak shmem_sync_init = pshmem_sync_init
