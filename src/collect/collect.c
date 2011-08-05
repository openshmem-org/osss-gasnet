/* (c) 2011 University of Houston System.  All rights reserved. */


#include <stdio.h>
#include <strings.h>

#include "comms.h"
#include "trace.h"
#include "utils.h"

#include "mpp/shmem.h"

#include "modules.h"

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
__shmem_collect_dispatch_init(void)
{
  char *name;
  int s;

  /*
   * choose the collect
   *
   */

  name = __shmem_comms_getenv("SHMEM_COLLECT_ALGORITHM");
  if (name == (char *) NULL) {
    name = __shmem_modules_get_implementation("collect");
  }
  s = __shmem_modules_load("collect", name, &mi);
  if (s != 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: couldn't load collect module \"%s\"",
		  name
		  );
    /* NOT REACHED */
  }

  /*
   * report which implementation we set up
   */
  __shmem_trace(SHMEM_LOG_BROADCAST,
		"using collect \"%s\"",
		name
		);
}

/*
 * the rest is what library users see
 *
 */


/* @api@ */
void
pshmem_collect32(void *target, const void *source, size_t nelems,
		 int PE_start, int logPE_stride, int PE_size,
		 long *pSync)
{
  SYMMETRY_CHECK(target, 1, "shmem_collect32");
  SYMMETRY_CHECK(source, 2, "shmem_collect32");

  mi.func_32(target, source, nelems,
	     PE_start, logPE_stride, PE_size,
	     pSync);
}

/* @api@ */
void
pshmem_collect64(void *target, const void *source, size_t nelems,
		 int PE_start, int logPE_stride, int PE_size,
		 long *pSync)
{
  SYMMETRY_CHECK(target, 1, "shmem_collect64");
  SYMMETRY_CHECK(source, 2, "shmem_collect64");

  mi.func_64(target, source, nelems,
	     PE_start, logPE_stride, PE_size,
	     pSync);
}

#pragma weak shmem_collect32 = pshmem_collect32
#pragma weak shmem_collect64 = pshmem_collect64
