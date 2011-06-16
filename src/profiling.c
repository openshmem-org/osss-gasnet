/* (c) 2011 University of Houston.  All rights reserved. */


#include "trace.h"

/*
 * stub for the proposed UFL profiling (PSHMEM) interface
 *
 */

void
shmem_pcontrol(int level)
{
  __shmem_trace(SHMEM_LOG_INFO,
		"pcontrol() interface currently not implemented"
		);
  return;
}
