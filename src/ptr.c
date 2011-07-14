/* (c) 2011 University of Houston System.  All rights reserved. */


/*
 * TODO: shmem_ptr only makes sense on platforms where puts and gets
 * occur exclusively in shared-memory.  On a multi-node cluster, it
 * can't do anything, so return NULL, which is correct behavior.
 * (which is really good, because I couldn't see how it could possibly
 * work :-)
 */

#include <stdio.h>

#include "trace.h"
#include "utils.h"

/* @api@ */
void *
pshmem_ptr(void *target, int pe)
{
  INIT_CHECK();
  PE_RANGE_CHECK(pe);

#ifdef SHMEM_PUTGET_SHARED_MEMORY

  __shmem_trace(SHMEM_LOG_NOTICE,
		"shmem_ptr() not implemented yet"
		);
  return (void *) NULL;

#else  /* ! SHMEM_PUTGET_SHARED_MEMORY */

  return (void *) NULL;

#endif /* SHMEM_PUTGET_SHARED_MEMORY*/
}

#pragma weak shmem_ptr = pshmem_ptr
