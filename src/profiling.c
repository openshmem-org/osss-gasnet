/* (c) 2011 University of Houston System.  All rights reserved. */


#include "trace.h"

/*
 * stub for the proposed UFL profiling (PSHMEM) interface
 *
 */

void
shmem_pcontrol(int level)
{
  char *msg;

  if (level == 0) {
    msg = "disabled";
  }
  else if (level == 1) {
    msg = "enabled (default detail)";
  }
  else {
    msg = "tool-specific";
  }

  __shmem_trace(SHMEM_LOG_INFO,
		"shmem_pcontrol(%d) is %s",
		level,
		msg
		);
  return;
}
