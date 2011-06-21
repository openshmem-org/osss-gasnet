/* (c) 2011 University of Houston.  All rights reserved. */


#include <stdio.h>
#include <strings.h>

/* #include "comms.h" */
#include "trace.h"
#include "utils.h"

#include "mpp/shmem.h"

#include "modules.h"

/*
 * these are what we use if nothing else available
 *
 */

static char *DEFAULT_FCOLLECT_ALGORITHM  = "naive";

/*
 * handlers for implementations
 *
 */

static module_info_t mi;


/*
 * called during initialization of shmem
 *
 */

void
__shmem_fcollect_dispatch_init(void)
{
  char *name;
  int s;

  /*
   * choose the fcollect
   *
   */

  name = __shmem_comms_getenv("SHMEM_FCOLLECT_ALGORITHM");
  if (name == (char *) NULL) {
    name = DEFAULT_FCOLLECT_ALGORITHM;
  }
  s = __shmem_modules_load("fcollect", name, &mi);
  if (s != 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: couldn't load fcollect module \"%s\"",
		  name
		  );
    /* NOT REACHED */
  }

  /*
   * report which implementation we set up
   */
  __shmem_trace(SHMEM_LOG_BROADCAST,
		"using fcollect \"%s\"",
		name
		);
}

/*
 * the rest is what library users see
 *
 */


/* @api@ */
void
pshmem_fcollect32(void *target, const void *source, size_t nlong,
		 int PE_start, int logPE_stride, int PE_size,
		 long *pSync)
{
  SYMMETRY_CHECK(target, 1, "shmem_fcollect32");
  SYMMETRY_CHECK(source, 2, "shmem_fcollect32");

  mi.func_32(target, source, nlong,
	     PE_start, logPE_stride, PE_size,
	     pSync);
}

/* @api@ */
void
pshmem_fcollect64(void *target, const void *source, size_t nlong,
		 int PE_start, int logPE_stride, int PE_size,
		 long *pSync)
{
  SYMMETRY_CHECK(target, 1, "shmem_fcollect64");
  SYMMETRY_CHECK(source, 2, "shmem_fcollect64");

  mi.func_64(target, source, nlong,
	     PE_start, logPE_stride, PE_size,
	     pSync);
}

#pragma weak shmem_fcollect32 = pshmem_fcollect32
#pragma weak shmem_fcollect64 = pshmem_fcollect64
